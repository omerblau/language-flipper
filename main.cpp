#include <windows.h>
#include <fcntl.h>   // _O_U16TEXT
#include <io.h>      // _setmode
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <thread>    // for sleep
#include <chrono>    // for milliseconds

// ───────────────────────────────────────── Layout IDs
constexpr char LAYOUT_HE[] = "0000040D";
constexpr char LAYOUT_EN[] = "00000409";

// ───────────────────────────────────────── Key-maps (single wchar)
const std::unordered_map<wchar_t, wchar_t> EN_TO_HE = {
    {L'q', L'/'}, {L'w', L'\''}, {L'e', L'ק'}, {L'r', L'ר'}, {L't', L'א'},
    {L'y', L'ט'}, {L'u', L'ו'}, {L'i', L'ן'}, {L'o', L'ם'}, {L'p', L'פ'},
    {L'a', L'ש'}, {L's', L'ד'}, {L'd', L'ג'}, {L'f', L'כ'}, {L'g', L'ע'},
    {L'h', L'י'}, {L'j', L'ח'}, {L'k', L'ל'}, {L'l', L'ך'}, {L';', L'ף'},
    {L'z', L'ז'}, {L'x', L'ס'}, {L'c', L'ב'}, {L'v', L'ה'}, {L'b', L'נ'},
    {L'n', L'מ'}, {L'm', L'צ'}, {L',', L'ת'}, {L'.', L'ץ'}
};
const std::unordered_map<wchar_t, wchar_t> HE_TO_EN = [] {
    std::unordered_map<wchar_t, wchar_t> r;
    for (auto [en, he]: EN_TO_HE) r[he] = en;
    return r;
}();

// ───────────────────────────────────────── Helpers
LANGID activeLang() {
    HWND h = GetForegroundWindow();
    return LOWORD(GetKeyboardLayout(GetWindowThreadProcessId(h, nullptr)));
}

std::wstring fix(const std::wstring &src,
                 const std::unordered_map<wchar_t, wchar_t> &map) {
    std::wstring out;
    out.reserve(src.size());
    for (wchar_t ch: src) {
        if (ch >= L'A' && ch <= L'Z') ch = wchar_t(ch + 32);
        auto it = map.find(ch);
        out += (it != map.end()) ? it->second : ch;
    }
    return out;
}

bool flipTo(const char *id) {
    HKL hkl = LoadKeyboardLayoutA(id, KLF_REORDER);
    return hkl && PostMessage(HWND_BROADCAST, WM_INPUTLANGCHANGEREQUEST,
                              0, reinterpret_cast<LPARAM>(hkl));
}

// optional: simulate typing
void typeWide(const std::wstring &w) {
    std::vector<INPUT> in;
    in.reserve(w.size() * 2);
    for (wchar_t ch: w) {
        INPUT d{};
        d.type = INPUT_KEYBOARD;
        d.ki.wScan = ch;
        d.ki.dwFlags = KEYEVENTF_UNICODE;
        in.push_back(d);
        INPUT u = d;
        u.ki.dwFlags |= KEYEVENTF_KEYUP;
        in.push_back(u);
    }
    SendInput(static_cast<UINT>(in.size()), in.data(), sizeof(INPUT));
}

// ───────────────────────────────────────── Clipboard-selection helper

// send Ctrl+C to copy selection
void sendCtrlC() {
    INPUT in[4]{};
    in[0].type = INPUT_KEYBOARD;
    in[0].ki.wVk = VK_CONTROL;
    in[1].type = INPUT_KEYBOARD;
    in[1].ki.wVk = 'C';
    in[2] = in[1];
    in[2].ki.dwFlags = KEYEVENTF_KEYUP;
    in[3] = in[0];
    in[3].ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(4, in, sizeof(INPUT));
}

// read CF_UNICODETEXT from clipboard
std::wstring readClipboard() {
    std::wstring out;
    if (OpenClipboard(nullptr)) {
        HANDLE h = GetClipboardData(CF_UNICODETEXT);
        if (h) {
            const wchar_t *data = static_cast<const wchar_t *>(GlobalLock(h));
            if (data) {
                out.assign(data);
                GlobalUnlock(h);
            }
        }
        CloseClipboard();
    }
    return out;
}

// copy selection and return it
std::wstring getHighlightedText() {
    sendCtrlC();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return readClipboard();
}

void flushModifiers() {
    // Release any stuck Ctrl or Alt keys so our Ctrl+C isn't combined with the hotkey
    INPUT ups[2]{};

    ups[0].type                = INPUT_KEYBOARD;
    ups[0].ki.wVk              = VK_CONTROL;
    ups[0].ki.dwFlags          = KEYEVENTF_KEYUP;

    ups[1].type                = INPUT_KEYBOARD;
    ups[1].ki.wVk              = VK_MENU;                // Alt
    ups[1].ki.dwFlags          = KEYEVENTF_KEYUP;

    SendInput(2, ups, sizeof(INPUT));
}


// void run_once() {
//     std::wstring selected;
//     DWORD before = GetClipboardSequenceNumber();
//
//     sendCtrlC();
//
//     auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(120);
//     while (GetClipboardSequenceNumber() == before &&
//            std::chrono::steady_clock::now() < deadline) {
//         std::this_thread::sleep_for(std::chrono::milliseconds(5));
//            }
//
//     if (GetClipboardSequenceNumber() == before) {
//         std::wcerr << L"No new text selected. Skipping.\n";
//         return;
//     }
//
//     selected = readClipboard();
//     std::wcout << L"selected: " << selected << L"\n";
//
//     LANGID lang = activeLang();
//     if (lang == 0x0409) {
//         std::wstring engFixed = fix(selected, EN_TO_HE);
//         std::wcout << L"EN→HE: " << engFixed << L"\n";
//         typeWide(engFixed);
//         if (flipTo(LAYOUT_HE)) std::wcout << L"Layout flipped ► Hebrew\n";
//         else std::wcerr << L"Flip failed\n";
//     }
//     else if (lang == 0x040D) {
//         std::wstring hebFixed = fix(selected, HE_TO_EN);
//         std::wcout << L"HE→EN: " << hebFixed << L"\n";
//         typeWide(hebFixed);
//         if (flipTo(LAYOUT_EN)) std::wcout << L"Layout flipped ► English\n";
//         else std::wcerr << L"Flip failed\n";
//     }
//     else {
//         std::wcerr << L"Unsupported layout. Exit.\n";
//     }
// }

//
void run_once() {
    std::wstring selected;
    DWORD before = GetClipboardSequenceNumber();
    flushModifiers();
    sendCtrlC();

    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(200);
    while (GetClipboardSequenceNumber() == before &&
           std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
           }

    if (GetClipboardSequenceNumber() == before) {
        std::wcerr << L"No new text selected. Skipping.\n";
        return;
    }

    selected = readClipboard();
    std::wcout << L"selected: " << selected << L"\n";

    LANGID lang = activeLang();
    if (lang == 0x0409) {
        std::wstring engFixed = fix(selected, EN_TO_HE);
        std::wcout << L"EN→HE: " << engFixed << L"\n";
        typeWide(engFixed);
        if (flipTo(LAYOUT_HE)) std::wcout << L"Layout flipped ► Hebrew\n";
        else std::wcerr << L"Flip failed\n";
    }
    else if (lang == 0x040D) {
        std::wstring hebFixed = fix(selected, HE_TO_EN);
        std::wcout << L"HE→EN: " << hebFixed << L"\n";
        typeWide(hebFixed);
        if (flipTo(LAYOUT_EN)) std::wcout << L"Layout flipped ► English\n";
        else std::wcerr << L"Flip failed\n";
    }
    else {
        std::wcerr << L"Unsupported layout. Exit.\n";
    }
}

// ───────────────────────────────────────── MAIN
int main() {
    // enable UTF-16 output
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stderr), _O_U16TEXT);

    // register Ctrl+Alt+B as our hotkey (ID = 1)
    if (!RegisterHotKey(NULL, 1, MOD_CONTROL | MOD_ALT, 'B')) {
        std::wcerr << L"Failed to register hotkey Ctrl+Alt+B\n";
        return 1;
    }

    std::wcout << L"Press Ctrl+Alt+B to fix the highlighted text in the foreground window...\n";

    // message loop — waits efficiently for WM_HOTKEY
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (msg.message == WM_HOTKEY && msg.wParam == 1) {
            // std::this_thread::sleep_for(std::chrono::milliseconds(300));
            run_once(); // your existing single-operation function
        }
    }

    // (we never actually get here, but if we did:)
    UnregisterHotKey(NULL, 1);
    return 0;
}
