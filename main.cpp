// THIS IS THE MIAN.CPP

#include "config.h"
#include "utils.h"

// Windows APIs
#include <windows.h>

// I/O & console
#include <fcntl.h>   // _O_U16TEXT
#include <io.h>      // _setmode
#include <iostream>

// Containers & strings
#include <string>

// Threading & timing
#include <chrono>


int main() {
    // enable UTF-16 output
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stderr), _O_U16TEXT);

    if (!registerHotkey()) {
        return 1;
    }
    // message loop — waits efficiently
    // Blocks until a message arrives for any window or thread queue
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (msg.message == WM_HOTKEY && msg.wParam == config::HOTKEY_ID) {
            run_once();
        }
    }

    UnregisterHotKey(nullptr, config::HOTKEY_ID);
    return 0;
}
