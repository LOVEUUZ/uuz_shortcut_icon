#include "WindowsHookMouseEx.h"
#include <iostream>

HHOOK               WindowsHookMouseEx::hMouseHook         = nullptr;
WindowsHookMouseEx* WindowsHookMouseEx::windowsHookMouseEx = nullptr;

WindowsHookMouseEx::WindowsHookMouseEx() : running(false) {}

WindowsHookMouseEx::~WindowsHookMouseEx() {
  if (running) {
    unInstallHook();
  }
}

//单例
WindowsHookMouseEx* WindowsHookMouseEx::getWindowHook() {
  if (windowsHookMouseEx == nullptr) {
    windowsHookMouseEx = new WindowsHookMouseEx();
  }
  return windowsHookMouseEx;
}

bool WindowsHookMouseEx::setFunc(const std::function<void()> & newFunc) {
  func = newFunc;
  return true;
}

//注册钩子
void WindowsHookMouseEx::installHook() {
#ifdef NDEBUG  //注意调试的时候尽量不编译，要不然鼠标一卡一卡的心烦
    std::lock_guard<std::mutex> lock(hookMutex);

    if (running) return;

    qInfo() << "mouse hook install";

    running = true;
    hookThread = std::thread([this]() {
        threadId = GetCurrentThreadId();
        hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, GetModuleHandle(nullptr), 0);
        if (!hMouseHook) {
            qWarning() << "Failed to set mouse hook!";
            running = false;
            return;
        }
        MSG msg;
        while (running && GetMessage(&msg, nullptr, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        //线程退出前清理 hook
        if (hMouseHook) {
            UnhookWindowsHookEx(hMouseHook);
            hMouseHook = nullptr;
        }
        });
    qInfo() << "mouse hook install successful";
#endif
}

//卸载钩子
void WindowsHookMouseEx::unInstallHook() {
#ifdef NDEBUG
    std::lock_guard<std::mutex> lock(hookMutex);

    if (!running) return;

    qInfo() << "mouse hook unInstall";

    running = false;
    // 让 GetMessage 退出
    PostThreadMessage(threadId, WM_QUIT, 0, 0);
    if (hookThread.joinable()) {
        hookThread.join();
    }
    qInfo() << "mouse hook unInstall successful";
#endif
}

//回调
LRESULT CALLBACK WindowsHookMouseEx::LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode >= 0 && (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN || wParam == WM_MBUTTONDOWN)) {
    if (windowsHookMouseEx && windowsHookMouseEx->func) {
      windowsHookMouseEx->func();
    }
  }
  return CallNextHookEx(hMouseHook, nCode, wParam, lParam);
}
