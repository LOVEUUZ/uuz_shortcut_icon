#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <QObject>
#include <QDebug>
#include <queue>
#include <unordered_map>
#include <chrono>

#include <windows.h>

#include "keyEvent.h"

//核心，windows钩子
class WindowsHookKeyEx : public QObject {
public:
    static WindowsHookKeyEx* ptr_windows_hook;

    static WindowsHookKeyEx* getWindowHook();

    void installHook();

    void unInstallHook();

    //策略模式，接受一个传进来的方法
    bool setFunc(const std::function<void(KeyEvent)> & newFunc);

private:
    explicit WindowsHookKeyEx()  = default;
    ~WindowsHookKeyEx() override = default;

    HHOOK             hook;
    MSG               msg;
    std::thread       msgLoopThread;
    std::atomic<bool> stopRequested = false;
    DWORD             msgLoopThreadId; //存储线程id

    std::queue<KeyEvent>          queue_; //按键队列
    std::mutex                    mtx_write;
    std::function<void(KeyEvent)> func = nullptr; //存储当前接收的策略

    static std::unordered_map<uint64_t, int> keyPressCount;  // 键位与计数器映射，计数器用于判断按下与松开
    static bool altPressed;                                  // 跟踪 Alt 键的状态

    void messageLoop(); //循环windows键盘事件

    static std::string getKeyName(DWORD vkCode); //获取按键对应名称

    static LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam); //钩子函数
};
