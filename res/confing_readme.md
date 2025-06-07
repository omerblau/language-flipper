# `config.json` Reference

This file customizes how your keyboard layout switcher/typer utility behaves.  
Edit the file, save it, and **restart the program** to apply changes—**no recompilation required**.

---

## **General Format**
```json
{
    "SETTING_NAME": value,
    ...
}
```

All setting names are case-sensitive and must match exactly.

---

## **Settings Explained**

### **DEBUG_MODE**
- **Type:** `true` or `false`
- **Description:**  
  When `true`, the program will print debug messages and keep the console open.  
  When `false`, the program runs in the background with no console.

---

### **Primary and Secondary Layouts**

#### **LANG_PRIMARY** and **LANG_SECONDARY**
- **Type:** Integer (see common values below)
- **Description:**  
  The language code for your "from" (primary) and "to" (secondary) layouts.  
  Example:
  - English = `9`
  - Hebrew = `13`
  - French = `12`
  - German = `7`

  **Full list:**  
  See [Microsoft's Language Identifiers documentation](https://learn.microsoft.com/en-us/windows/win32/intl/language-identifiers)  
  or in your Windows SDK header file (`winnt.h`).

  | Language      | Value |
    |---------------|-------|
  | English       | 9     |
  | Hebrew        | 13    |
  | French        | 12    |
  | German        | 7     |
  | Spanish       | 10    |
  | Russian       | 25    |
  | Arabic        | 1     |
  | Japanese      | 17    |
  | Chinese       | 4     |

#### **SUBLANG_PRIMARY** and **SUBLANG_SECONDARY**
- **Type:** Integer
- **Description:**  
  The "sub-language" code, usually for region/variant.  
  `1` is most common and means "default".

  **Common values:**  
  | Sublanguage   | Value |
  |-------------- |-------|
  | Default       | 1     |
  | US (for English) | 1  |
  | UK (for English) | 2  |

---

#### **ROLE_NAME_PRIMARY** and **ROLE_NAME_SECONDARY**
- **Type:** String (any text)
- **Description:**  
  Human-readable names for your layouts (used in debug output).

  Example:
  ```json
  "ROLE_NAME_PRIMARY": "English"
  ```

---

### **KEYMAP_PRIMARY_TO_SECONDARY**
- **Type:** JSON object (map of single-character strings)
- **Description:**  
  Maps each key from the primary layout to the secondary layout.  
  Keys and values must be **single characters**, written as strings.

  Example:
  ```json
  "KEYMAP_PRIMARY_TO_SECONDARY": {
    "q": "/", "w": "'", "e": "ק"
    // etc.
  }
  ```

---

### **Clipboard Polling Settings**

#### **CLIPBOARD_POLL_TIMEOUT_MS**
- **Type:** Integer (milliseconds)
- **Description:**  
  Maximum time to wait for clipboard changes after copying, in milliseconds.

#### **CLIPBOARD_POLL_INTERVAL_MS**
- **Type:** Integer (milliseconds)
- **Description:**  
  How often to check if the clipboard has changed, in milliseconds.

---

### **Hotkey Settings (new format!)**

Hotkeys are defined by three fields:
- `*_HOTKEY_MODIFIERS` — A **list** of modifier key names (as strings).  
  Valid values: `"ctrl"`, `"alt"`, `"shift"`, `"win"`
- `*_HOTKEY_VK` — The main key to trigger the hotkey, written as a **single character string** (like `"m"`, `"n"`, `"a"`).
- `*_HOTKEY_ID` — An arbitrary unique integer to identify this hotkey.

**Example (Ctrl+M):**
```json
"BASIC_HOTKEY_MODIFIERS": ["ctrl"],
"BASIC_HOTKEY_VK": "m",
"BASIC_HOTKEY_ID": 1,
```

**Example (Ctrl+Alt+N):**
```json
"ALL_HOTKEY_MODIFIERS": ["ctrl", "alt"],
"ALL_HOTKEY_VK": "n",
"ALL_HOTKEY_ID": 3,
```

- You may use lowercase or uppercase for the key; `"m"` and `"M"` both work.
- Modifiers can appear in any order, as long as each is a recognized string.
- Only the following modifier names are supported: `"ctrl"`, `"alt"`, `"shift"`, `"win"`

---

### **AUTO_FLIP_ON_CHANGE**
- **Type:** `true` or `false`
- **Description:**  
  If `true`, after text is re-typed, the program will automatically switch your system keyboard layout.

---

## **How to find language codes**

- For **language codes** (e.g., English, Hebrew, French):  
  Search “Microsoft language identifier constants” or see [this table](https://learn.microsoft.com/en-us/windows/win32/intl/language-identifiers).

- For **sublanguage codes**:  
  See “Sublanguage Identifier Constants” in the same doc or header.

- For hotkey main keys, just use the character you want in quotes (e.g., `"m"` for the M key).
- For modifiers, use any combination of `"ctrl"`, `"alt"`, `"shift"`, `"win"` as a JSON array.

---

## **Full Example**

```json
{
  "DEBUG_MODE": true,
  "LANG_PRIMARY": 9,
  "SUBLANG_PRIMARY": 1,
  "ROLE_NAME_PRIMARY": "English",
  "LANG_SECONDARY": 13,
  "SUBLANG_SECONDARY": 1,
  "ROLE_NAME_SECONDARY": "Hebrew",
  "KEYMAP_PRIMARY_TO_SECONDARY": {
    "q": "/", "w": "'", "e": "ק"
    // ...etc...
  },
  "CLIPBOARD_POLL_TIMEOUT_MS": 200,
  "CLIPBOARD_POLL_INTERVAL_MS": 5,
  "BASIC_HOTKEY_MODIFIERS": ["ctrl"],
  "BASIC_HOTKEY_VK": "m",
  "BASIC_HOTKEY_ID": 1,
  "LINE_HOTKEY_MODIFIERS": ["ctrl", "alt"],
  "LINE_HOTKEY_VK": "m",
  "LINE_HOTKEY_ID": 2,
  "ALL_HOTKEY_MODIFIERS": ["ctrl", "alt"],
  "ALL_HOTKEY_VK": "n",
  "ALL_HOTKEY_ID": 3,
  "AUTO_FLIP_ON_CHANGE": true
}
```

---

**Edit and save this file, then restart your app to apply changes!**
