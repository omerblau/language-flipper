// this is config.h
#pragma once

#include "json.hpp"
#include <windows.h>        // for WORD, LANG_* and SUBLANG_* macros
#include <unordered_map>
#include <string>

namespace config {

    extern bool DEBUG_MODE;

    extern WORD LANG_PRIMARY;
    extern WORD SUBLANG_PRIMARY;
    extern std::wstring ROLE_NAME_PRIMARY;

    extern WORD LANG_SECONDARY;
    extern WORD SUBLANG_SECONDARY;
    extern std::wstring ROLE_NAME_SECONDARY;

    extern std::unordered_map<wchar_t, wchar_t> KEYMAP_PRIMARY_TO_SECONDARY;

    extern int CLIPBOARD_POLL_TIMEOUT_MS ;
    extern int CLIPBOARD_POLL_INTERVAL_MS;

    extern UINT BASIC_HOTKEY_MODIFIERS;
    extern UINT BASIC_HOTKEY_VK;
    extern int  BASIC_HOTKEY_ID;

    extern UINT LINE_HOTKEY_MODIFIERS ;
    extern UINT LINE_HOTKEY_VK;
    extern int  LINE_HOTKEY_ID;

    extern UINT ALL_HOTKEY_MODIFIERS;
    extern UINT ALL_HOTKEY_VK;
    extern int  ALL_HOTKEY_ID;

    extern bool AUTO_FLIP_ON_CHANGE ;

    void load(const std::string &filename);
    std::wstring utf8_to_wstring(const std::string& str);
    UINT parse_modifiers(const nlohmann::json& arr);
    UINT parse_vk(const nlohmann::json& j);


}