#include "config.h"
#include "utils.h"
#include "third_party/json/json.hpp"

#include <fstream>
#include <iostream>
#include <codecvt>
#include <locale>


using json = nlohmann::json;

namespace config {
    // All config variables declared as extern in config.h
    bool DEBUG_MODE = true;

    WORD LANG_PRIMARY = LANG_ENGLISH;
    WORD SUBLANG_PRIMARY = SUBLANG_DEFAULT;
    std::wstring ROLE_NAME_PRIMARY = L"English";

    WORD LANG_SECONDARY = LANG_HEBREW;
    WORD SUBLANG_SECONDARY = SUBLANG_DEFAULT;
    std::wstring ROLE_NAME_SECONDARY = L"Hebrew";

    std::unordered_map<wchar_t, wchar_t> KEYMAP_PRIMARY_TO_SECONDARY = {
        {L'q', L'/'}, {L'w', L'\''}, {L'e', L'ק'}, {L'r', L'ר'}, {L't', L'א'},
        {L'y', L'ט'}, {L'u', L'ו'}, {L'i', L'ן'}, {L'o', L'ם'}, {L'p', L'פ'},
        {L'a', L'ש'}, {L's', L'ד'}, {L'd', L'ג'}, {L'f', L'כ'}, {L'g', L'ע'},
        {L'h', L'י'}, {L'j', L'ח'}, {L'k', L'ל'}, {L'l', L'ך'}, {L';', L'ף'}, {L'\'', L','},
        {L'z', L'ז'}, {L'x', L'ס'}, {L'c', L'ב'}, {L'v', L'ה'}, {L'b', L'נ'},
        {L'n', L'מ'}, {L'm', L'צ'}, {L',', L'ת'}, {L'.', L'ץ'}, {L'/', L'.'}
    };
    int CLIPBOARD_POLL_TIMEOUT_MS = 200;
    int CLIPBOARD_POLL_INTERVAL_MS = 5;

    UINT BASIC_HOTKEY_MODIFIERS = MOD_CONTROL;
    UINT BASIC_HOTKEY_VK = 'M';
    int BASIC_HOTKEY_ID = 1;

    UINT LINE_HOTKEY_MODIFIERS = MOD_CONTROL | MOD_ALT;
    UINT LINE_HOTKEY_VK = 'M';
    int LINE_HOTKEY_ID = 2;

    UINT ALL_HOTKEY_MODIFIERS = MOD_CONTROL | MOD_ALT;
    UINT ALL_HOTKEY_VK = 'N';
    int ALL_HOTKEY_ID = 3;

    bool AUTO_FLIP_ON_CHANGE = true;

    void load(const std::string &filename) {
        std::ifstream file(filename);
        if (!file) {
            DEBUG_PRINT(L"[config] Could not open config file: " << std::wstring(filename.begin(), filename.end()));
            return;
        }

        json j;
        try {
            file >> j;
        } catch (const std::exception &e) {
            DEBUG_PRINT(L"[config] JSON parsing error: "<< std::wstring(
                std::string(e.what()).begin(), std::string(e.what()).end()));
            return;
        }

        if (j.contains("DEBUG_MODE")) DEBUG_MODE = j["DEBUG_MODE"];
        if (j.contains("LANG_PRIMARY")) LANG_PRIMARY = j["LANG_PRIMARY"];
        if (j.contains("SUBLANG_PRIMARY")) SUBLANG_PRIMARY = j["SUBLANG_PRIMARY"];
        if (j.contains("ROLE_NAME_PRIMARY"))
            ROLE_NAME_PRIMARY = utf8_to_wstring(j["ROLE_NAME_PRIMARY"].get<std::string>());

        if (j.contains("LANG_SECONDARY")) LANG_SECONDARY = j["LANG_SECONDARY"];
        if (j.contains("SUBLANG_SECONDARY")) SUBLANG_SECONDARY = j["SUBLANG_SECONDARY"];
        if (j.contains("ROLE_NAME_SECONDARY"))
            ROLE_NAME_SECONDARY = utf8_to_wstring(j["ROLE_NAME_SECONDARY"].get<std::string>());

        if (j.contains("KEYMAP_PRIMARY_TO_SECONDARY")) {
            KEYMAP_PRIMARY_TO_SECONDARY.clear();
            for (auto &[key, val]: j["KEYMAP_PRIMARY_TO_SECONDARY"].items()) {
                std::wstring k_w = utf8_to_wstring(key);
                std::wstring v_w = utf8_to_wstring(val.get<std::string>());
                if (!k_w.empty() && !v_w.empty()) {
                    KEYMAP_PRIMARY_TO_SECONDARY[k_w[0]] = v_w[0];
                }
            }
        }

        if (j.contains("CLIPBOARD_POLL_TIMEOUT_MS")) CLIPBOARD_POLL_TIMEOUT_MS = j["CLIPBOARD_POLL_TIMEOUT_MS"];
        if (j.contains("CLIPBOARD_POLL_INTERVAL_MS")) CLIPBOARD_POLL_INTERVAL_MS = j["CLIPBOARD_POLL_INTERVAL_MS"];

        if (j.contains("BASIC_HOTKEY_MODIFIERS")) BASIC_HOTKEY_MODIFIERS = parse_modifiers(j["BASIC_HOTKEY_MODIFIERS"]);
        if (j.contains("BASIC_HOTKEY_VK")) BASIC_HOTKEY_VK = parse_vk(j["BASIC_HOTKEY_VK"]);
        if (j.contains("BASIC_HOTKEY_ID")) BASIC_HOTKEY_ID = j["BASIC_HOTKEY_ID"];

        if (j.contains("LINE_HOTKEY_MODIFIERS")) LINE_HOTKEY_MODIFIERS = parse_modifiers(j["LINE_HOTKEY_MODIFIERS"]);
        if (j.contains("LINE_HOTKEY_VK")) LINE_HOTKEY_VK = parse_vk(j["LINE_HOTKEY_VK"]);
        if (j.contains("LINE_HOTKEY_ID")) LINE_HOTKEY_ID = j["LINE_HOTKEY_ID"];

        if (j.contains("ALL_HOTKEY_MODIFIERS")) ALL_HOTKEY_MODIFIERS = parse_modifiers(j["ALL_HOTKEY_MODIFIERS"]);
        if (j.contains("ALL_HOTKEY_VK")) ALL_HOTKEY_VK = parse_vk(j["ALL_HOTKEY_VK"]);
        if (j.contains("ALL_HOTKEY_ID")) ALL_HOTKEY_ID = j["ALL_HOTKEY_ID"];

        if (j.contains("AUTO_FLIP_ON_CHANGE")) AUTO_FLIP_ON_CHANGE = j["AUTO_FLIP_ON_CHANGE"];

        if (DEBUG_MODE) {
            DEBUG_PRINT(L"[config] Loaded configuration from "
                << std::wstring(filename.begin(), filename.end()));
        }
    }

    std::wstring utf8_to_wstring(const std::string &str) {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t> > conv;
        return conv.from_bytes(str);
    }

    UINT parse_modifiers(const json &arr) {
        UINT mods = 0;
        if (!arr.is_array()) return mods;
        for (const auto &mod: arr) {
            std::string s = mod.get<std::string>();
            for (auto &c: s) c = tolower(c);
            if (s == "ctrl") mods |= MOD_CONTROL;
            else if (s == "alt") mods |= MOD_ALT;
            else if (s == "shift") mods |= MOD_SHIFT;
            else if (s == "win") mods |= MOD_WIN;
        }
        return mods;
    }

    UINT parse_vk(const json &j) {
        if (j.is_string()) {
            std::string s = j.get<std::string>();
            if (s.length() == 1) {
                // Return uppercase ASCII for A-Z/0-9
                char ch = s[0];
                if (ch >= 'a' && ch <= 'z') ch -= 32;
                return static_cast<UINT>(ch);
            }
            // You could expand this for F1, etc.
        } else if (j.is_number()) {
            return j.get<UINT>();
        }
        return 0; // fallback
    }
}
