#include "config.h"

// Windows APIs
#include <windows.h>

// I/O & console
#include <fcntl.h>   // _O_U16TEXT
#include <io.h>      // _setmode
#include <iostream>

// Containers & strings
#include <string>
#include <unordered_map>
#include <vector>

// Threading & timing
#include <thread>
#include <chrono>


// ──── lang 2 -> lang 1 map generations ───────────────────────────────
static std::unordered_map<wchar_t, wchar_t> makeHeToEnMap() {
    std::unordered_map<wchar_t, wchar_t> map;
    map.reserve(config::EN_TO_HE.size());
    for (auto const &pair: config::EN_TO_HE) {
        map[pair.second] = pair.first;
    }
    return map;
}
const std::unordered_map<wchar_t, wchar_t> HE_TO_EN = makeHeToEnMap();
// ─────────────────────────────────────────────────────────────────────


LANGID activeLang() {
    HWND h = GetForegroundWindow();
    return LOWORD(GetKeyboardLayout(GetWindowThreadProcessId(h, nullptr)));
}

std::wstring fix(const std::wstring &src, const std::unordered_map<wchar_t, wchar_t> &map) {
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
    std::vector<INPUT> ups;
    ups.reserve(3);

    if constexpr (config::HOTKEY_MODIFIERS & MOD_CONTROL) {
        INPUT i{};
        i.type = INPUT_KEYBOARD;
        i.ki.wVk = VK_CONTROL;
        i.ki.dwFlags = KEYEVENTF_KEYUP;
        ups.push_back(i);
    }
    if constexpr (config::HOTKEY_MODIFIERS & MOD_ALT) {
        INPUT i{};
        i.type = INPUT_KEYBOARD;
        i.ki.wVk = VK_MENU;
        i.ki.dwFlags = KEYEVENTF_KEYUP;
        ups.push_back(i);
    }
    if constexpr (config::HOTKEY_MODIFIERS & MOD_SHIFT) {
        INPUT i{};
        i.type = INPUT_KEYBOARD;
        i.ki.wVk = VK_SHIFT;
        i.ki.dwFlags = KEYEVENTF_KEYUP;
        ups.push_back(i);
    }

    if (!ups.empty())
        SendInput(static_cast<UINT>(ups.size()), ups.data(), sizeof(INPUT));
}

// Wait (up to timeoutMs) for the clipboard sequence to change
static DWORD waitForNewClipboard(DWORD before, int timeoutMs = 200) {
    auto deadline = std::chrono::steady_clock::now()
                    + std::chrono::milliseconds(timeoutMs);
    while (GetClipboardSequenceNumber() == before
           && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    return GetClipboardSequenceNumber();
}

// Copy the current selection and return the newly‐copied text (or empty if none)
static std::wstring copyAndFetchSelection() {
    // 1) clear any stuck modifiers
    flushModifiers();

    // 2) note current clipboard version
    DWORD before = GetClipboardSequenceNumber();

    // 3) send Ctrl+C
    sendCtrlC();

    // 4) wait for it to change
    DWORD after = waitForNewClipboard(before);
    if (after == before) {
        // nothing new
        return L"";
    }

    // 5) read & return
    return readClipboard();
}

// Given the raw selected text, fix‐and‐type it, then flip the layout
static void processAndType(const std::wstring &selected) {
    // echo what we got
    std::wcout << L"selected: " << selected << L"\n";

    // detect layout
    LANGID lang = activeLang();
    bool flipped = false;

    if (lang == 0x0409) {
        // EN→HE
        auto out = fix(selected, config::EN_TO_HE);
        std::wcout << L"EN→HE: " << out << L"\n";
        typeWide(out);
        flipped = flipTo(config::LAYOUT_HE);
        if (flipped) std::wcout << L"Layout flipped ► Hebrew\n";
    } else if (lang == 0x040D) {
        // HE→EN
        auto out = fix(selected, HE_TO_EN);
        std::wcout << L"HE→EN: " << out << L"\n";
        typeWide(out);
        flipped = flipTo(config::LAYOUT_EN);
        if (flipped) std::wcout << L"Layout flipped ► English\n";
    } else {
        std::wcerr << L"Unsupported layout. Exit.\n";
    }

    if (!flipped && (lang == 0x0409 || lang == 0x040D)) {
        std::wcerr << L"Flip failed\n";
    }
}

// ─── Simplified run_once ────────────────────────────────────────────
void run_once() {
    auto selected = copyAndFetchSelection();
    if (selected.empty()) {
        std::wcerr << L"No new text selected. Skipping.\n";
        return;
    }
    processAndType(selected);
}

static bool registerHotkey() {
    // build a human‐readable name, e.g. "Ctrl+Alt+B"
    std::wstring name;
    if constexpr (config::HOTKEY_MODIFIERS & MOD_CONTROL) name += L"Ctrl+";
    if constexpr (config::HOTKEY_MODIFIERS & MOD_ALT) name += L"Alt+";
    if constexpr (config::HOTKEY_MODIFIERS & MOD_SHIFT) name += L"Shift+";
    name += static_cast<wchar_t>(config::HOTKEY_VK);
    if (!RegisterHotKey(nullptr, config::HOTKEY_ID, config::HOTKEY_MODIFIERS, config::HOTKEY_VK)) {
        std::wcerr << L"Failed to register hotkey " << name << L"\n";
        return false;
    }
    std::wcout << L"Hotkey registered " << name << L"\n";
    return true;
}

int main() {
    // enable UTF-16 output
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stderr), _O_U16TEXT);

    if (!registerHotkey()) {
        return 1;
    }

    // message loop — waits efficiently for WM_HOTKEY
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (msg.message == WM_HOTKEY && msg.wParam == config::HOTKEY_ID) {
            run_once();
        }
    }

    UnregisterHotKey(nullptr, config::HOTKEY_ID);
    return 0;
}
