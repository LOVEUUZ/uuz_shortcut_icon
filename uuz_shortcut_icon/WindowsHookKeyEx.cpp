﻿#include "WindowsHookKeyEx.h"

#include <iostream>
#include <qlogging.h>

#include "keyEvent.h"

WindowsHookKeyEx* WindowsHookKeyEx::ptr_windows_hook = nullptr;

WindowsHookKeyEx* WindowsHookKeyEx::getWindowHook() {
  if (ptr_windows_hook == nullptr) {
    ptr_windows_hook = new WindowsHookKeyEx;
  }
  return ptr_windows_hook;
}

void WindowsHookKeyEx::installHook() {
  // std::cout << "Install KEY hook\n";
  qInfo() << "Install KEY hook";
  getWindowHook();
  hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, nullptr, 0);
  if (hook == nullptr) {
    // std::cerr << "Failed to install KEY hook" << "\n";
    qWarning() << "Install KEY hook";
    std::exit(0);
  }

  // std::cout << "Install KEY hook successful\n";
  qInfo() << "Install KEY hook successful";
  // 消息循环在单独的线程中运行
  msgLoopThread = std::thread(&WindowsHookKeyEx::messageLoop, this);
}

void WindowsHookKeyEx::unInstallHook() {
  // std::cout << "unInstall KEY hook\n";
  qInfo() << "unInstall KEY hook";
  if (hook != nullptr) {
    stopRequested = true;

    // 向消息循环线程发送 WM_QUIT 消息
    PostThreadMessage(msgLoopThreadId, WM_QUIT, 0, 0);

    if (msgLoopThread.joinable()) {
      msgLoopThread.detach();
    }

    UnhookWindowsHookEx(hook);
    hook = nullptr;

    qInfo() << "unInstall KEY hook successful";
  }
}

bool WindowsHookKeyEx::setFunc(const std::function<void(KeyEvent)> & newFunc) {
  // if (func != nullptr) {
  //   return false;
  // }
  func = newFunc;
  return true;
}

//循环windows键盘事件
void WindowsHookKeyEx::messageLoop() {
  msgLoopThreadId = GetCurrentThreadId();
  while (!stopRequested.load()) {
    while (GetMessage(&msg, nullptr, 0, 0)) {
      if (msg.message == WM_QUIT) {
        break;
      }
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
}

std::string WindowsHookKeyEx::getKeyName(DWORD vkCode) {
  UINT scanCode = MapVirtualKey(vkCode, MAPVK_VK_TO_VSC);
  // Construct a fake lParam
  LONG lParam = (scanCode << 16) | 1;
  char keyName[128];
  if (GetKeyNameTextA(lParam, keyName, sizeof(keyName)) > 0) {
    return std::string(keyName);
  }
  return "Unknown";
}



// 2025.04.10：优化按键钩子，处理当alt按下与非按下状态下其他键位的触发
std::unordered_map<uint64_t, int> WindowsHookKeyEx::keyPressCount;
bool WindowsHookKeyEx::altPressed = false;
//钩子函数
LRESULT WindowsHookKeyEx::KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode >= 0) {
    auto        pkbhs   = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
    std::string keyName = getKeyName(pkbhs->vkCode);

    std::pair<uint64_t, std::string> p = std::make_pair(pkbhs->vkCode, keyName);

    KeyEvent key_event;
    key_event.key      = pkbhs->vkCode;
    key_event.key_name = getKeyName(pkbhs->vkCode);


    if ((pkbhs->vkCode == 164 || pkbhs->vkCode == 165) && wParam == WM_KEYUP) {
        std::thread([&]() {
          std::this_thread::sleep_for(std::chrono::milliseconds(100));   // 延迟100ms，当手速很快的时候避免alt释放过快
          altPressed = false;
        }).detach();
      key_event.isPressed = false;
    }
    else if (pkbhs->vkCode == 164 || pkbhs->vkCode == 165) {
      altPressed = true;
      key_event.isPressed = true;
    }
    else if (altPressed) {
            //当alt按下状态处理其他按键的出发状态
            if (keyPressCount[pkbhs->vkCode] % 2 == 0) {  // 第一次按下
                key_event.isPressed = true;
                keyPressCount[pkbhs->vkCode]++;
            }
            else {  // 第二次按下，视为松开
                key_event.isPressed = false;
                keyPressCount[pkbhs->vkCode]++;
            }
    }
    else {
	  //正常情况下的按键处理(alt没有按下的时候)
      if (wParam == WM_KEYDOWN) {
        key_event.isPressed = true;
      }
      else if (wParam == WM_KEYUP) {
        key_event.isPressed = false;
      }
    }
    ptr_windows_hook->func(key_event); //策略模式，执行注册的事件
  }
  return CallNextHookEx(nullptr, nCode, wParam, lParam);
  // return CallNextHookEx(hook, nCode, wParam, lParam);
}
