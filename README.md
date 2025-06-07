# Language Flipper
> **Fix text typed in the *wrong* keyboard layout with a single hotkey.**  
> Select → hit <kbd>Ctrl</kbd> + <kbd>M</kbd> (default) → watch gibberish turn into the text you *meant* to write — and keep on typing in the correct layout.

![demo gif](res/demo.gif) <!-- Replace with a real GIF once recorded -->

---

## Table of Contents
1. [Why?](#why)
2. [Features](#features)
3. [Quick Start](#quick-start)
4. [Configuration](#configuration)
5. [Usage](#usage)
6. [How It Works](#how-it-works)
7. [Roadmap](#roadmap)
8. [Contributing](#contributing)
9. [License](#license)

---

## Why?

Ever discovered you’ve written half an email in gibberish instead of your desired language?  
Switching layouts, re-typing, or copy-pasting into online converters breaks your flow.  
**Language Flipper** lives in the background: one hot-key and the garbage text is replaced *in-place* with its correct-layout twin.

---

## Features

| ⚡️ | Fast — sub-100 ms round-trip |
|----|-----------------------------|
| 🎚️ | **Fully configurable**: pick any two layouts, map any characters, choose your own hot-key |
| 🕶️ | Runs silent — a single **statically-linked `.exe`** (no DLLs, no installer) |
| 🔄 | *Auto-flip* option: after correction, it switches Windows to the right layout so you can keep typing |

---

## Quick Start

1. **Download the latest release** from the **Releases** page.
2. Double-click `Language Flipper.exe`.
3. Select some wrong-layout text and press **Ctrl + M** (default).
4. 🌟 *Magic!* 🌟

---

## Configuration

**All runtime options now live in a simple file: [`config.json`](./config.json)** —  
not in a `.h` file! This makes tweaking layouts, keymaps, and hotkeys as easy as editing a text file (no coding or recompiling).

> 💡 A full configuration guide is included as [`res/confing_readme.md`](./res/confing_readme.md).  
> See it for detailed instructions, examples, and code snippets.

| Setting                         | Purpose                                                 | Example / Default                   |
|----------------------------------|---------------------------------------------------------|-------------------------------------|
| `DEBUG_MODE`                     | Show debug logs/console window                         | `false`                             |
| `LANG_PRIMARY / SUBLANG_PRIMARY` | The layout you *accidentally* type in                  | English (US): `9`, sublang: `1`     |
| `LANG_SECONDARY / SUBLANG_SECONDARY` | The layout you *want*                             | Hebrew: `13`, sublang: `1`          |
| `ROLE_NAME_PRIMARY` / `ROLE_NAME_SECONDARY` | Human-readable layout names                  | `"English"`, `"Hebrew"`             |
| `KEYMAP_PRIMARY_TO_SECONDARY`    | Character mapping, in JSON, *no recompilation needed*  | `"q": "/"`, `"e": "ק"` etc.         |
| `*_HOTKEY_MODIFIERS` / `*_HOTKEY_VK` / `*_HOTKEY_ID` | Hotkey definition for each action   | See below                           |
| `AUTO_FLIP_ON_CHANGE`            | Flip Windows layout after correction                   | `true`                              |

**Hotkey settings now support:**
- **Basic hotkey:** Corrects current selection (default: Ctrl + M)
- **Line hotkey:** Selects & corrects the *current line* (default: Ctrl + Alt + M)
- **All hotkey:** Selects & corrects *all* text (default: Ctrl + Alt + N)

Each hotkey can be customized in `config.json` with human-friendly lists like `["ctrl", "alt"]` for modifiers, and `"m"` for the key.

---

### Example Hotkey Block in `config.json`

``
"BASIC_HOTKEY_MODIFIERS": ["ctrl"],
"BASIC_HOTKEY_VK": "m",
"BASIC_HOTKEY_ID": 1,

"LINE_HOTKEY_MODIFIERS": ["ctrl", "alt"],
"LINE_HOTKEY_VK": "m",
"LINE_HOTKEY_ID": 2,

"ALL_HOTKEY_MODIFIERS": ["ctrl", "alt"],
"ALL_HOTKEY_VK": "n",
"ALL_HOTKEY_ID": 3,
``

**See the release folder for a sample config and README.**

---

## Usage

1. Start **Language Flipper.exe** (it hides itself unless `DEBUG_MODE` is `true`).
2. **Select** text that looks wrong, or use the hotkeys to select a line or all text:
   - **Ctrl + M** (default): Correct currently selected text
   - **Ctrl + Alt + M**: *Select and correct* the current line
   - **Ctrl + Alt + N**: *Select and correct* all text in the active window
3. The junk text is instantly replaced with the correct letters.
4. If `AUTO_FLIP_ON_CHANGE` is enabled, your system layout also switches so you can keep typing.

> **Tip:** All hotkeys are customizable in `config.json`.  
> Works everywhere: VS Code, Word, Chrome address bar, Discord, even the Windows *Run* dialog!

---

## How It Works

1. Registers a global hot-key via `RegisterHotKey`.  
2. On trigger:  
   * Simulates **Ctrl + C** to copy the selection.  
   * Detects the active thread’s keyboard layout with `GetKeyboardLayout`.  
   * Transforms clipboard text by walking the `KEYMAP`.  
   * Types the corrected text back using `SendInput`.  
   * Optionally flips the layout with `LoadKeyboardLayout` + `ActivateKeyboardLayout`.

All logic fits in ~300 lines across `main.cpp`, `utils.cpp/h`, and `config.h`.

---

## Contributing

Got an idea? Open an **issue**!

---

## License

Language Flipper is distributed under the MIT License.  
See [LICENSE.MIT](./LICENSE.MIT) for full text.

This project includes [nlohmann/json](https://github.com/nlohmann/json),  
which is also MIT-licensed and included under the same license file.

---

© 2025 Omer Blau. Attribution is appreciated!
