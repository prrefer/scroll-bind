#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include "hidusage.h"
#include <iostream>
#include <thread>
#include <chrono>

INPUT spaceInput{};
constexpr int inputSize{ sizeof(INPUT) };

void pressSpace() {
    spaceInput.ki.dwFlags = 0; // press
    SendInput(1, &spaceInput, inputSize);
    std::this_thread::sleep_for(std::chrono::nanoseconds(1));
    spaceInput.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &spaceInput, inputSize);
}

LRESULT CALLBACK rawInputProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_INPUT) {
        RAWINPUT rawInput;
        UINT size = sizeof(rawInput);
        GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, &rawInput, &size, sizeof(RAWINPUTHEADER));

        if (rawInput.header.dwType == RIM_TYPEMOUSE and rawInput.data.mouse.usButtonFlags == RI_MOUSE_WHEEL) {
            pressSpace();
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int main() {
    std::cout << "scroll-bind | https://github.com/prrefer/scroll-bind" << std::endl;

    spaceInput.type = INPUT_KEYBOARD;
    spaceInput.ki.wVk = VK_SPACE;
    spaceInput.ki.wScan = MapVirtualKey(VK_SPACE, MAPVK_VK_TO_VSC);

    WNDCLASSEX windowClass;
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.lpfnWndProc = rawInputProc;
    windowClass.lpszClassName = "prrefer";
    RegisterClassEx(&windowClass);
    HWND hwnd = CreateWindowEx(0, windowClass.lpszClassName, nullptr, 0, 0, 0, 0, 0, HWND_MESSAGE, 0, windowClass.hInstance, 0);

    RAWINPUTDEVICE rawInputDevice;
    rawInputDevice.usUsagePage = HID_USAGE_PAGE_GENERIC;
    rawInputDevice.usUsage = HID_USAGE_GENERIC_MOUSE;
    rawInputDevice.dwFlags = RIDEV_INPUTSINK;
    rawInputDevice.hwndTarget = hwnd;

    if (!RegisterRawInputDevices(&rawInputDevice, 1, sizeof(RAWINPUTDEVICE))) {
        std::cerr << GetLastError();
        return EXIT_FAILURE;
    }

    MSG message;
    while (GetMessage(&message, nullptr, 0, 0)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
    UnregisterClassA(windowClass.lpszClassName, windowClass.hInstance);

    return EXIT_SUCCESS;
}