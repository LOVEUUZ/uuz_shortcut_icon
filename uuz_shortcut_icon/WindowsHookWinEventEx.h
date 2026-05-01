#pragma once

#include <QObject>

#include <windows.h>
#include <string>
#include <functional>

class WindowsHookWinEventEx : public QObject {
public:
    static WindowsHookWinEventEx* ptr_windowsEvent_hook;    //单例

	using Callback = std::function<void(DWORD pid, const std::string& processName)>; // 回调函数，参数: 进程ID,进程名称

    WindowsHookWinEventEx();
    ~WindowsHookWinEventEx();

    static WindowsHookWinEventEx* getWindowsHookwinevent();

    bool installHook(Callback cb);
    void uninstallHook();

private:
    static void CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook,DWORD event,HWND hwnd,LONG idObject,LONG idChild,DWORD dwEventThread,DWORD dwmsEventTime);

    void handleEvent(HWND hwnd);

private:
    HWINEVENTHOOK windowEventListenHook;        //注册到系统中的hook
    Callback      windowFocusChangeEvent;       //接受到事件后具体处理逻辑，写在 setwindowsWinEvent 那边

    // 缓存
    DWORD         lastWindowId;
    std::string   lastWindowName;
};