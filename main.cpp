// THIS IS THE MIAN.CPP

#include "config.h"
#include "utils.h"

#include <windows.h>
#include <fcntl.h>   // _O_U16TEXT
#include <io.h>      // _setmode
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    // Load config from file (will override defaults in config.cpp)
    config::load("config.json");

    if (config::DEBUG_MODE) {
        // enable UTF-16 output
        _setmode(_fileno(stdout), _O_U16TEXT);
        _setmode(_fileno(stderr), _O_U16TEXT);
    } else {
        FreeConsole();
    }

    // Register all hotkeys
    bool ok = true;
    ok &= registerHotkey(config::BASIC_HOTKEY_ID, config::BASIC_HOTKEY_MODIFIERS, config::BASIC_HOTKEY_VK);
    ok &= registerHotkey(config::LINE_HOTKEY_ID, config::LINE_HOTKEY_MODIFIERS, config::LINE_HOTKEY_VK);
    ok &= registerHotkey(config::ALL_HOTKEY_ID, config::ALL_HOTKEY_MODIFIERS, config::ALL_HOTKEY_VK);

    if (!ok) {
        MessageBoxA(nullptr, "Could not register all hotkeys", "Error", MB_ICONERROR);
        return 1;
    }

    // Blocks until a message arrives for any window or thread queue
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (msg.message == WM_HOTKEY) {
            if (msg.wParam == config::BASIC_HOTKEY_ID) {
                std::wcout << L"Basic Case: \n";
                flushModifiers(config::BASIC_HOTKEY_MODIFIERS);
                copyAndFlip();
            } else if (msg.wParam == config::LINE_HOTKEY_ID) {
                std::wcout << L"Line Case: \n";
                flushModifiers(config::LINE_HOTKEY_MODIFIERS);
                selectCurrentLine();
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                copyAndFlip();
            } else if (msg.wParam == config::ALL_HOTKEY_ID) {
                std::wcout << L"All Case: \n";
                flushModifiers(config::ALL_HOTKEY_MODIFIERS);
                selectAllText();
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                copyAndFlip();
            }
        }
    }


    // Unregister on exit
    UnregisterHotKey(nullptr, config::BASIC_HOTKEY_ID);
    UnregisterHotKey(nullptr, config::LINE_HOTKEY_ID);
    UnregisterHotKey(nullptr, config::ALL_HOTKEY_ID);

    return 0;
}
