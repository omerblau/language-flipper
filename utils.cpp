#include "utils.h"


std::unordered_map<wchar_t, wchar_t> makeHeToEnMap() {
    std::unordered_map<wchar_t, wchar_t> map;
    map.reserve(config::EN_TO_HE.size());
    for (auto const &pair: config::EN_TO_HE) {
        map[pair.second] = pair.first;
    }
    return map;
}

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

// Attempts to switch keyboard layout to the one identified by `layoutId` (e.g. "00000409").
// Returns true if both loading the layout and posting the change request succeed.
bool switchKeyboardLayout(std::string_view layoutId) {
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

// read CF_UNICODETEXT from clipboard
std::wstring readClipboard() {
    // 1) Open the clipboard
    if (!OpenClipboard(nullptr)) {
        std::wcerr << L"[readClipboard] Failed to open clipboard\n";
        return L"";
    }

    std::wstring text;

    // 2) Request Unicode text
    HANDLE hData = GetClipboardData(CF_UNICODETEXT);
    if (hData != nullptr) {
        // 3) Lock the handle to get a pointer
        const wchar_t *pWide = static_cast<const wchar_t *>(GlobalLock(hData));
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

// Wait up to CLIPBOARD_POLL_TIMEOUT_MS for the clipboard sequence to change.
// Returns the new sequence number, or the old one if we timed out.
DWORD waitForClipboardChange(const DWORD previousSequence) {
    using Clock = std::chrono::steady_clock;
    using Milliseconds = std::chrono::milliseconds;

    const auto deadline = Clock::now() + Milliseconds(config::CLIPBOARD_POLL_TIMEOUT_MS);
    DWORD current;

    // Loop until either the clipboard sequence changes, or we hit our deadline.
    while (Clock::now() < deadline) {
        current = GetClipboardSequenceNumber();
        if (current != previousSequence) {
            return current; // we got new data
        }
        std::this_thread::sleep_for(Milliseconds(config::CLIPBOARD_POLL_INTERVAL_MS));
    }

    // Timeout: no change detected
    return previousSequence;
}

// Copy the current selection and return the newly‐copied text (or empty if none)
std::wstring copyAndFetchSelection() {
    // clear any stuck modifiers
    flushModifiers();

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


// 2) Detect layout in its own function
Layout detectLayout() {
    LANGID id = activeLang();
    if (id == 0x0409) return Layout::English;
    if (id == 0x040D) return Layout::Hebrew;
    return Layout::Unsupported;
}

// 3) Transform text based on the layout we’re coming *from*
std::wstring transformText(const std::wstring &input, const Layout from) {
    switch (from) {
        case Layout::English:
            return fix(input, config::EN_TO_HE);
        case Layout::Hebrew:
            return fix(input, HE_TO_EN);
        default:
            return input;
    }
}

// 4) Output (log) original & transformed
void logTransformation(const std::wstring &orig, const std::wstring &transformed, Layout from) {
    std::wcout << L"selected: " << orig << L'\n';
    if (from == Layout::English)
        std::wcout << L"EN→HE: " << transformed << L'\n';
    else
        std::wcout << L"HE→EN: " << transformed << L'\n';
}

// 5) Type & switch in one function
//    Returns true if the layout flip succeeded
bool typeAndSwitch(const std::wstring &text, Layout from) {
    typeWide(text);
    bool ok = false;

    if (from == Layout::English) {
        ok = switchKeyboardLayout(config::LAYOUT_HE);
        std::wcout << (ok
                           ? L"Layout flipped ► Hebrew\n"
                           : L"Flip to Hebrew failed\n");
    } else if (from == Layout::Hebrew) {
        ok = switchKeyboardLayout(config::LAYOUT_EN);
        std::wcout << (ok
                           ? L"Layout flipped ► English\n"
                           : L"Flip to English failed\n");
    }

    return ok;
}

// 6) The new “process one clipboard event” is now super-clean:
void handleClipboardText(const std::wstring &selected) {
    auto layout = detectLayout();
    if (layout == Layout::Unsupported) {
        std::wcerr << L"Unsupported layout. Aborting.\n";
        return;
    }

    auto transformed = transformText(selected, layout);
    logTransformation(selected, transformed, layout);
    typeAndSwitch(transformed, layout);
}

// 7) And run_once just glues it together:
void run_once() {
    auto selected = copyAndFetchSelection();
    if (selected.empty()) {
        std::wcerr << L"No new text selected. Skipping.\n";
        return;
    }
    handleClipboardText(selected);
}

bool registerHotkey() {
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