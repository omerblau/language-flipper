// this is utils.cpp

#include "utils.h"
#include "config.h"

// Windows APIs
#include <windows.h>

// I/O & console
#include <fcntl.h>   // _O_U16TEXT
#include <iostream>

// Containers & strings
#include <string>
#include <unordered_map>
#include <vector>

// Threading & timing
#include <thread>


// ─── Layout Roles & IDs ────────────────────────────────────────────────

std::string makeLayoutString(const LANGID id) {
    char buf[9];
    std::snprintf(buf, sizeof(buf), "%08X", static_cast<unsigned>(id));
    return buf;
}

LANGID getLangIdPrimary() {
    return MAKELANGID(config::LANG_PRIMARY, config::SUBLANG_PRIMARY);
}

LANGID getLangIdSecondary() {
    return MAKELANGID(config::LANG_SECONDARY, config::SUBLANG_SECONDARY);
}

std::string getKLIDPrimary() {
    return makeLayoutString(getLangIdPrimary());
}

std::string getKLIDSecondary() {
    return makeLayoutString(getLangIdSecondary());
}

// ─── Mapping Tables ────────────────────────────────────────────────────

std::unordered_map<wchar_t, wchar_t> makeSecondaryToPrimaryMap() {
    std::unordered_map<wchar_t, wchar_t> map;
    map.reserve(config::KEYMAP_PRIMARY_TO_SECONDARY.size());
    for (auto const &pair: config::KEYMAP_PRIMARY_TO_SECONDARY) {
        map[pair.second] = pair.first;
    }
    return map;
}

// ─── Clipboard Helpers ─────────────────────────────────────────────────

std::wstring readClipboard() {
    // 1) Open the clipboard
    if (!OpenClipboard(nullptr)) {
        if (config::DEBUG_MODE) std::wcerr << L"[readClipboard] Failed to open clipboard\n";
        return L"";
    }

    std::wstring text;

    // 2) Request Unicode text
    HANDLE hData = GetClipboardData(CF_UNICODETEXT);
    if (hData != nullptr) {
        // 3) Lock the handle to get a pointer
        // ReSharper disable once CppTooWideScope
        const auto *pWide = static_cast<const wchar_t *>(GlobalLock(hData));
        if (pWide) {
            // 4) Copy into our string
            text.assign(pWide);
            GlobalUnlock(hData);
        }
    }

    // 5) Always close when done
    CloseClipboard();
    return text;
}

DWORD waitForClipboardChange(const DWORD previousSequence) {
    using Clock = std::chrono::steady_clock;
    using Milliseconds = std::chrono::milliseconds;

    const auto deadline = Clock::now() + Milliseconds(config::CLIPBOARD_POLL_TIMEOUT_MS);

    // Loop until either the clipboard sequence changes, or we hit our deadline.
    while (Clock::now() < deadline) {
        DWORD current = GetClipboardSequenceNumber();
        if (current != previousSequence) {
            return current; // we got new data
        }
        std::this_thread::sleep_for(Milliseconds(config::CLIPBOARD_POLL_INTERVAL_MS));
    }

    // Timeout: no change detected
    return previousSequence;
}

std::wstring copyAndFetchSelection() {
    const DWORD before = GetClipboardSequenceNumber();

    sendCtrlC();

    // wait for it to change
    DWORD after = waitForClipboardChange(before);
    if (after == before) {
        return L"";
    }

    // read & return
    return readClipboard();
}

// ─── Input Simulation ──────────────────────────────────────────────────

// Release any stuck modifier keys (Ctrl, Alt, Shift) in one batched SendInput call
void flushModifiers(UINT modifiers) {
    std::vector<INPUT> ups;
    ups.reserve(3);

    if (modifiers & MOD_CONTROL) {
        INPUT i{};
        i.type = INPUT_KEYBOARD;
        i.ki.wVk = VK_CONTROL;
        i.ki.dwFlags = KEYEVENTF_KEYUP;
        ups.push_back(i);
    }
    if (modifiers & MOD_ALT) {
        INPUT i{};
        i.type = INPUT_KEYBOARD;
        i.ki.wVk = VK_MENU;
        i.ki.dwFlags = KEYEVENTF_KEYUP;
        ups.push_back(i);
    }
    if (modifiers & MOD_SHIFT) {
        INPUT i{};
        i.type = INPUT_KEYBOARD;
        i.ki.wVk = VK_SHIFT;
        i.ki.dwFlags = KEYEVENTF_KEYUP;
        ups.push_back(i);
    }

    if (!ups.empty())
        SendInput(static_cast<UINT>(ups.size()), ups.data(), sizeof(INPUT));
}


void sendCtrlC() {
    INPUT inputs[4] = {};

    // Ctrl down, C down, C up, Ctrl up
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_CONTROL;
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = 'C';
    inputs[2] = inputs[1];
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;
    inputs[3] = inputs[0];
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(4, inputs, sizeof(INPUT));
}

void selectCurrentLine() {
    INPUT inputs[4] = {};

    // Press SHIFT
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_SHIFT;

    // Press HOME
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = VK_HOME;

    // Release HOME
    inputs[2] = inputs[1];
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;

    // Release SHIFT
    inputs[3] = inputs[0];
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(4, inputs, sizeof(INPUT));
}

void selectAllText() {
    INPUT inputs[4] = {};

    // Press CTRL down
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_CONTROL;

    // Press A
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = 'A';

    // Release A
    inputs[2] = inputs[1];
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;

    // Release CTRL
    inputs[3] = inputs[0];
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(4, inputs, sizeof(INPUT));
}

void typeText(const std::wstring &text) {
    // We need one INPUT down‐event and one up‐event per character:
    std::vector<INPUT> inputs;
    // Reserve space for two events per character to avoid resizing
    inputs.reserve(text.size() * 2);

    for (const wchar_t ch: text) {
        // Key‐down event (UNICODE scan code)
        INPUT keyDown{};
        keyDown.type = INPUT_KEYBOARD;
        keyDown.ki.wScan = ch; // Unicode code point
        keyDown.ki.dwFlags = KEYEVENTF_UNICODE; // tell Windows it's a unicode wScan

        // Key‐up event is identical, plus the KEYEVENTF_KEYUP flag:
        INPUT keyUp = keyDown;
        keyUp.ki.dwFlags |= KEYEVENTF_KEYUP;

        inputs.push_back(keyDown);
        inputs.push_back(keyUp);
    }

    // Fire them all in one batch for efficiency
    SendInput(static_cast<UINT>(inputs.size()),
              inputs.data(),
              sizeof(INPUT));
}

// ─── Detection & Fixing ─────────────────────────────────────────────────

LANGID activeLang() {
    const HWND h = GetForegroundWindow();
    return LOWORD(GetKeyboardLayout(GetWindowThreadProcessId(h, nullptr)));
}

std::wstring fix(const std::wstring &src, const std::unordered_map<wchar_t, wchar_t> &map) {
    std::wstring out;
    out.reserve(src.size());
    for (wchar_t ch: src) {
        if (ch >= L'A' && ch <= L'Z') ch = static_cast<wchar_t>(ch + 32);
        auto it = map.find(ch);
        out += (it != map.end()) ? it->second : ch;
    }
    return out;
}

LayoutRole detectLayout() {
    const LANGID id = activeLang();
    if (id == getLangIdPrimary()) return LayoutRole::Primary;
    if (id == getLangIdSecondary()) return LayoutRole::Secondary;
    return LayoutRole::Unsupported;
}

std::wstring transformText(const std::wstring &input, const LayoutRole from) {
    switch (from) {
        case LayoutRole::Primary:
            return fix(input, config::KEYMAP_PRIMARY_TO_SECONDARY); // primary→secondary map
        case LayoutRole::Secondary:
            return fix(input, makeSecondaryToPrimaryMap()); // secondary→primary map
        default:
            return input;
    }
}

void logTransformation(const std::wstring &orig, const std::wstring &transformed, const LayoutRole from) {
    DEBUG_PRINT(L"selected: " << orig);
    // Choose labels based on the role
    if (from == LayoutRole::Primary) {
        // primary → secondary
        DEBUG_PRINT(config::ROLE_NAME_PRIMARY << L"→"
            << config::ROLE_NAME_SECONDARY << L": " << transformed);
    } else if (from == LayoutRole::Secondary) {
        // secondary → primary
        DEBUG_PRINT(config::ROLE_NAME_SECONDARY << L"→" <<
            config::ROLE_NAME_PRIMARY << L": " << transformed);
    } else {
        DEBUG_PRINT(L"Unsupported role. Output: " << transformed);
    }
}


// ─── Switching (optional) ──────────────────────────────────────────────

bool switchKeyboardLayout(const std::string_view layoutId) {
    // 1) Load the layout into this process (reordering the HKL list)
    HKL layoutHandle = LoadKeyboardLayoutA(layoutId.data(), KLF_REORDER);
    if (!layoutHandle) {
        // failed to load the layout at all
        return false;
    }

    // 2) Broadcast a WM_INPUTLANGCHANGEREQUEST so all windows switch
    LRESULT result = PostMessage(
        HWND_BROADCAST,
        WM_INPUTLANGCHANGEREQUEST,
        0,
        reinterpret_cast<LPARAM>(layoutHandle)
    );

    // PostMessage returns nonzero on success
    return result != 0;
}

bool flipLayout(const LayoutRole from) {
    bool ok = false;
    const std::wstring &fromName =
            (from == LayoutRole::Primary)
                ? config::ROLE_NAME_PRIMARY
                : (from == LayoutRole::Secondary)
                      ? config::ROLE_NAME_SECONDARY
                      : L"Unknown";

    const std::wstring &toName =
            (from == LayoutRole::Primary)
                ? config::ROLE_NAME_SECONDARY
                : (from == LayoutRole::Secondary)
                      ? config::ROLE_NAME_PRIMARY
                      : L"Unknown";

    // do the flip
    if (from == LayoutRole::Primary) {
        ok = switchKeyboardLayout(getKLIDSecondary());
    } else if (from == LayoutRole::Secondary) {
        ok = switchKeyboardLayout(getKLIDPrimary());
    }

    // log the result using macro
    if (ok) {
        DEBUG_PRINT(L"Layout flipped: " << fromName << L"→" << toName);
    } else {
        DEBUG_PRINT(L"Flip from " << fromName << L" to " << toName << L" failed");
    }

    return ok;
}


// ─── Hotkey & Orchestration ────────────────────────────────────────────

void handleClipboardText(const std::wstring &selected) {
    const auto layout = detectLayout();
    if (layout == LayoutRole::Unsupported) {
        DEBUG_PRINT(L"Unsupported layout. Aborting.\n");
        return;
    }

    const auto transformed = transformText(selected, layout);

    if (config::DEBUG_MODE) logTransformation(selected, transformed, layout);
    typeText(transformed);

    if (config::AUTO_FLIP_ON_CHANGE) {
        flipLayout(layout);
    }
}

std::wstring makeHotkeyName(const UINT modifiers, const UINT vk) {
    std::wstring name;
    if (modifiers & MOD_CONTROL) name += L"Ctrl+";
    if (modifiers & MOD_ALT) name += L"Alt+";
    if (modifiers & MOD_SHIFT) name += L"Shift+";
    if (modifiers & MOD_WIN) name += L"Win+";

    name += static_cast<wchar_t>(vk);
    return name;
}

bool registerHotkey(const int id, const UINT modifiers, const UINT vk) {
    const std::wstring name = makeHotkeyName(modifiers, vk);

    if (!RegisterHotKey(nullptr, id, modifiers, vk)) {
        DEBUG_PRINT(L"Failed to register hotkey " << name);
        return false;
    }

    DEBUG_PRINT(L"Hotkey registered " << name);
    return true;
}

void copyAndFlip() {
    const auto selected = copyAndFetchSelection();
    if (selected.empty()) {
        if (config::DEBUG_MODE) std::wcerr << L"No new text selected. Skipping.\n";
        return;
    }
    handleClipboardText(selected);
}
