// THIS IS THE MIAN.CPP

#include "config.h"
#include "utils.h"

#include <windows.h>
#include <fcntl.h>   // _O_U16TEXT
#include <io.h>      // _setmode
#include <iostream>

int main() {
    if (config::DEBUG_MODE) {
        // enable UTF-16 output
        _setmode(_fileno(stdout), _O_U16TEXT);
        _setmode(_fileno(stderr), _O_U16TEXT);
    } else {
        FreeConsole();
    }

    if (!registerHotkey()) {
        MessageBoxA(nullptr, "Could not register hotkey", "Error", MB_ICONERROR);
        return 1;
    }

    // Blocks until a message arrives for any window or thread queue
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (msg.message == WM_HOTKEY && msg.wParam == HOTKEY_ID) {
            run_once();
        }
    }

    UnregisterHotKey(nullptr, HOTKEY_ID);
    return 0;
}
