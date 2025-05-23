// include/config.h
#pragma once
#include <windows.h>
#include <unordered_map>
#include <cwchar>


namespace config {
    // ─── Layout IDs ───────────────────────────────────────────────────────
    constexpr char LAYOUT_HE[] = "0000040D";
    constexpr char LAYOUT_EN[] = "00000409";

    // ─── Hotkey settings ─────────────────────────────────────────────────
    constexpr int HOTKEY_ID         = 1;
    // Combine any of MOD_CONTROL, MOD_ALT, MOD_SHIFT, MOD_WIN (bitwise-OR)
    // e.g. MOD_CONTROL | MOD_ALT  for Ctrl+Alt
    constexpr UINT HOTKEY_MODIFIERS = MOD_CONTROL | MOD_SHIFT;

    // Virtual-Key code:
    //   'A'–'Z' for letters, VK_F1–VK_F24, VK_OEM_* for punctuation, etc.
    constexpr UINT HOTKEY_VK = 'B';

    // ─── keys mapping ─────────────────────────────────────────────────
    // the map from lang 1 -> lang 2 will auto generate
    const std::unordered_map<wchar_t, wchar_t> EN_TO_HE = {
        {L'q', L'/'}, {L'w', L'\''}, {L'e', L'ק'}, {L'r', L'ר'},
        {L't', L'א'}, {L'y', L'ט'}, {L'u', L'ו'}, {L'i', L'ן'},
        {L'o', L'ם'}, {L'p', L'פ'}, {L'a', L'ש'}, {L's', L'ד'},
        {L'd', L'ג'}, {L'f', L'כ'}, {L'g', L'ע'}, {L'h', L'י'},
        {L'j', L'ח'}, {L'k', L'ל'}, {L'l', L'ך'}, {L';', L'ף'},
        {L'z', L'ז'}, {L'x', L'ס'}, {L'c', L'ב'}, {L'v', L'ה'},
        {L'b', L'נ'}, {L'n', L'מ'}, {L'm', L'צ'}, {L',', L'ת'},
        {L'.', L'ץ'}
    };
}
