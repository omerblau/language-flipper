#pragma once

// ─────────────────────────────────────────────────────────────────────────────
//   CONFIGURATION — edit *only* this file to customize your language pair,
//   key mappings, hotkey combo, and behavior. No other code changes needed!
// ─────────────────────────────────────────────────────────────────────────────

#include <windows.h>        // for WORD, LANG_* and SUBLANG_* macros
#include <unordered_map>
#include <string>

namespace config {
    // ─── Debug / console toggle ────────────────────────────────────
    // false → run entirely hidden, no console, no prints
    // true  → keep console open, show all prints
    constexpr bool DEBUG_MODE = false;


    // ─── Primary / “from” Layout ─────────────────────────────────────────────
    //
    // Pick your source language. These macros come from <winnt.h>:
    //   • primary: the language your text was typed in
    //   • sublang: usually SUBLANG_DEFAULT
    //
    // To find yours, look in:
    //   C:\Program Files (x86)\Windows Kits\10\Include\<version>\um\winnt.h
    // or consult Microsoft’s “Language Identifier Constants” list.
    //
    constexpr WORD LANG_PRIMARY    = LANG_ENGLISH;      // e.g. LANG_ENGLISH
    constexpr WORD SUBLANG_PRIMARY = SUBLANG_DEFAULT;   // e.g. SUBLANG_DEFAULT

    // Human‐readable name for logging: “English”, “Hebrew”, “French”, etc.
    inline const std::wstring ROLE_NAME_PRIMARY = L"English";


    // ─── Secondary / “to” Layout ──────────────────────────────────────────────
    //
    // Pick the target language you want to convert INTO.
    // Use the same LANG_* / SUBLANG_* macros from <winnt.h>.
    //
    constexpr WORD LANG_SECONDARY    = LANG_HEBREW;     // e.g. LANG_HEBREW
    constexpr WORD SUBLANG_SECONDARY = SUBLANG_DEFAULT; // e.g. SUBLANG_DEFAULT

    inline const std::wstring ROLE_NAME_SECONDARY = L"Hebrew";


    // ─── Key Mapping ─────────────────────────────────────────────────────────
    //
    // Define how each character from PRIMARY maps to SECONDARY.
    // It must cover every key you expect—but missing keys pass through unchanged.
    //
    const std::unordered_map<wchar_t, wchar_t> KEYMAP_PRIMARY_TO_SECONDARY = {
        {L'q', L'/'}, {L'w', L'\''}, {L'e', L'ק'}, {L'r', L'ר'}, {L't', L'א'},
        {L'y', L'ט'}, {L'u', L'ו'}, {L'i', L'ן'}, {L'o', L'ם'}, {L'p', L'פ'},
        {L'a', L'ש'}, {L's', L'ד'}, {L'd', L'ג'}, {L'f', L'כ'}, {L'g', L'ע'},
        {L'h', L'י'}, {L'j', L'ח'}, {L'k', L'ל'}, {L'l', L'ך'}, {L';', L'ף'}, {L'\'', L','},
        {L'z', L'ז'}, {L'x', L'ס'}, {L'c', L'ב'}, {L'v', L'ה'}, {L'b', L'נ'},
        {L'n', L'מ'}, {L'm', L'צ'}, {L',', L'ת'}, {L'.', L'ץ'}, {L'/', L'.'}
    };


    // ─── Timing & Clipboard Polling ───────────────────────────────────────────
    //
    // How long to wait (ms) for the clipboard to update after Ctrl+C,
    // and how often to check.
    //
    constexpr int CLIPBOARD_POLL_TIMEOUT_MS  = 200;
    constexpr int CLIPBOARD_POLL_INTERVAL_MS =   5;


    // ─── Hotkey Settings ─────────────────────────────────────────────────────
    //
    // HOTKEY_ID:   arbitrary identifier (only matters if you register >1 hotkey)
    // MODIFIERS   : bitwise-OR of MOD_CONTROL, MOD_ALT, MOD_SHIFT, MOD_WIN
    // VK          : any virtual-key (e.g. 'A'–'Z', VK_F1–VK_F24, VK_OEM_* …)
    //
    // To change the shortcut, edit MODIFIERS and/or VK here.
    //
    constexpr UINT BASIC_HOTKEY_MODIFIERS = MOD_CONTROL;  // e.g. MOD_CONTROL | MOD_ALT
    constexpr UINT BASIC_HOTKEY_VK        = 'M';          // e.g. 'M' for Ctrl+M
    constexpr int  BASIC_HOTKEY_ID        = 1;

    constexpr UINT LINE_HOTKEY_MODIFIERS  = MOD_CONTROL | MOD_ALT;
    constexpr UINT LINE_HOTKEY_VK         = 'M';
    constexpr int  LINE_HOTKEY_ID         = 2;

    constexpr UINT ALL_HOTKEY_MODIFIERS   = MOD_CONTROL | MOD_ALT;
    constexpr UINT ALL_HOTKEY_VK          = 'N';
    constexpr int  ALL_HOTKEY_ID          = 3;

    // ─── Auto‐flip toggle ─────────────────────────────────────────────────────
    //
    // If true, after re‐typing the text we switch your system keyboard layout.
    // If false, we leave your current layout alone.
    //
    constexpr bool AUTO_FLIP_ON_CHANGE = true;

} // namespace config