// THIS IS UTILS.H

#pragma once

#include "config.h"

#include <string>
#include <unordered_map>
#include <windows.h>   // for LANGID


#define DEBUG_PRINT(msg) do { if (config::DEBUG_MODE) std::wcout << msg << std::endl; } while (0)

// ─── Layout Roles & IDs ────────────────────────────────────────────────

/// Which role the current layout is playing in the swap.
enum class LayoutRole { Primary, Secondary, Unsupported };

/// Build the LANGIDs from your config (no more MAKELANGID in main code).
constexpr LANGID LANGID_PRIMARY = MAKELANGID(config::LANG_PRIMARY, config::SUBLANG_PRIMARY);
constexpr LANGID LANGID_SECONDARY = MAKELANGID(config::LANG_SECONDARY, config::SUBLANG_SECONDARY);

/// Convert a LANGID (e.g. 0x0409) to its 8-digit KLID string ("00000409").
std::string makeLayoutString(LANGID id);

/// The two layout-ID strings ready to pass to LoadKeyboardLayoutA()
inline const std::string KLID_PRIMARY = makeLayoutString(LANGID_PRIMARY);
inline const std::string KLID_SECONDARY = makeLayoutString(LANGID_SECONDARY);


// ─── Mapping Tables ────────────────────────────────────────────────────

/// Generate Secondary→Primary map from your config’s Primary→Secondary map
std::unordered_map<wchar_t, wchar_t> makeSecondaryToPrimaryMap();

/// The reverse mapping (auto-generated at startup)
inline const std::unordered_map<wchar_t, wchar_t> KEYMAP_SECONDARY_TO_PRIMARY
        = makeSecondaryToPrimaryMap();


// ─── Clipboard Helpers ─────────────────────────────────────────────────

/// Read CF_UNICODETEXT from the clipboard (or empty on failure).
std::wstring readClipboard();

/// Wait up to config::CLIPBOARD_POLL_TIMEOUT_MS for the clipboard sequence to change.
DWORD waitForClipboardChange(DWORD previousSequence);

/// Send Ctrl+C and return the newly-copied text (or empty if none).
std::wstring copyAndFetchSelection();


// ─── Input Simulation ──────────────────────────────────────────────────

/// Release any stuck Ctrl/Alt/Shift modifiers based on HOTKEY_MODIFIERS.
void flushModifiers(UINT modifiers);


/// Simulate a Ctrl+C keystroke to copy the current selection.
void sendCtrlC();


void selectCurrentLine();


void selectAllText();

/// Type out a wide string as Unicode input events.
void typeText(const std::wstring &text);


// ─── Detection & Fixing ─────────────────────────────────────────────────

/// Get the current foreground‐window thread’s keyboard LANGID.
LANGID activeLang();

/// Apply the mapping table to a string, lowercasing A–Z first.
std::wstring fix(const std::wstring &src,
                 const std::unordered_map<wchar_t, wchar_t> &map);

/// Decide which role we’re in (Primary / Secondary / Unsupported).
LayoutRole detectLayout();

/// Transform text based on the role: Primary→Secondary or Secondary→Primary.
std::wstring transformText(const std::wstring &input, LayoutRole from);

/// Log “orig → transformed” using ROLE_NAME_PRIMARY/SECONDARY from config.
void logTransformation(const std::wstring &orig, const std::wstring &transformed, LayoutRole from);


// ─── Switching (optional) ──────────────────────────────────────────────

/// Broadcast WM_INPUTLANGCHANGEREQUEST for a given KLID string.
bool switchKeyboardLayout(std::string_view layoutId);

/// Flip from Primary↔Secondary and log the result.
bool flipLayout(LayoutRole from);


// ─── Hotkey & Orchestration ────────────────────────────────────────────

/// Copy→transform→type→(optional flip) for a single clipboard event.
void handleClipboardText(const std::wstring &selected);


std::wstring makeHotkeyName(UINT modifiers, UINT vk);

/// Register the hotkey as defined in config (ID, modifiers, virtual key).
bool registerHotkey(int id, UINT modifiers, UINT vk);

/// Run a single cycle: copy, transform, type (and optionally flip).
void copyAndFlip();
