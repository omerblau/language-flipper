// THIS IS UTILS.H

#pragma once

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

enum class Layout { English, Hebrew, Unsupported };

// ──── lang 2 -> lang 1 map generations ───────────────────────────────
std::unordered_map<wchar_t, wchar_t> makeHeToEnMap();

inline std::unordered_map<wchar_t, wchar_t> HE_TO_EN = makeHeToEnMap();

void flushModifiers();

void sendCtrlC();

void typeWide(const std::wstring &w);

LANGID activeLang();

std::wstring fix(const std::wstring &src, const std::unordered_map<wchar_t, wchar_t> &map);

bool switchKeyboardLayout(std::string_view layoutId);

std::wstring readClipboard();

DWORD waitForClipboardChange(const DWORD previousSequence);

std::wstring copyAndFetchSelection();

bool registerHotkey();

Layout detectLayout();

std::wstring transformText(const std::wstring &input, const Layout from);

void logTransformation(const std::wstring &orig, const std::wstring &transformed, Layout from);

bool typeAndSwitch(const std::wstring &text, Layout from);

void handleClipboardText(const std::wstring &selected);

void run_once();
