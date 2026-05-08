#include "WindowsHookWinEventEx.h"
#include <psapi.h>

#pragma comment(lib, "psapi.lib")

WindowsHookWinEventEx* WindowsHookWinEventEx::ptr_windowsEvent_hook = nullptr;

WindowsHookWinEventEx::WindowsHookWinEventEx() : 
    windowEventListenHook(NULL), lastWindowId(0) {}

WindowsHookWinEventEx::~WindowsHookWinEventEx() {
    uninstallHook();
}

WindowsHookWinEventEx* WindowsHookWinEventEx::getWindowsHookwinevent() {
    if (ptr_windowsEvent_hook == nullptr) {
        ptr_windowsEvent_hook = new WindowsHookWinEventEx();
    }
    return ptr_windowsEvent_hook;
}

bool WindowsHookWinEventEx::installHook(Callback cb) {
    if (windowEventListenHook)
        return true;

    windowFocusChangeEvent = cb;

    windowEventListenHook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, NULL, 
                                WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);

    return windowEventListenHook != NULL;
}

void WindowsHookWinEventEx::uninstallHook() {
    if (windowEventListenHook) {
        UnhookWinEvent(windowEventListenHook);
        windowEventListenHook = NULL;
    }

    lastWindowId = 0;
    lastWindowName.clear();
}

//注册到系统中的回调，接收事件通知
void CALLBACK WindowsHookWinEventEx::WinEventProc(HWINEVENTHOOK,DWORD event,HWND hwnd,LONG,LONG,DWORD,DWORD) {
	if (event != EVENT_SYSTEM_FOREGROUND || !hwnd)      //只关心前台窗口事件，并且窗口句柄有效
        return;

    if (ptr_windowsEvent_hook) {
        ptr_windowsEvent_hook->handleEvent(hwnd);
    }
}

//简单缓存判断
void WindowsHookWinEventEx::handleEvent(HWND hwnd) {
    DWORD pid = 0;
	GetWindowThreadProcessId(hwnd, &pid);   //获取窗口所属进程ID

    if (pid != lastWindowId) {
        lastWindowId = pid;

        char name[MAX_PATH] = { 0 };

        //根据进程PID获得进程handle      查询进程基本信息标志        允许读取目标进程虚拟内存标志
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (hProcess) {
            GetModuleBaseNameA(hProcess, NULL, name, MAX_PATH);
            CloseHandle(hProcess);
        }

        lastWindowName = name;
    }

    if (windowFocusChangeEvent) {
        windowFocusChangeEvent(pid, lastWindowName);
    }
}