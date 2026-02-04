# NaNote 🖊️
**The 8KB Win32 Text Editor**

NaNote is a minimalist, high-performance text editor for Windows, crafted with pure Win32 API and zero C Runtime (CRT) dependencies. It’s an exercise in binary size optimization, fitting a feature-rich experience into exactly 8,192 bytes.



## ✨ Features
- **Tiny Footprint**: Exactly **8KB** (8,192 bytes). Small enough to fit on a 1970s floppy disk with room to spare.
- **Zero Bloat**: Built with `/NODEFAULTLIB`. No external dependencies other than core Windows DLLs.
- **Modern UI**: Supports **Dark Mode** (toggle with `Ctrl+D`) and modern system fonts.
- **Smart Search**: A context-aware, non-intrusive floating search bar (`Ctrl+F`).
- **Encoding Support**: UTF-8, UTF-16, Shift-JIS, and Latin-1 support.
- **Stay-on-Top**: Keep your notes visible while you work (`Ctrl+T`).
- **Portable**: Auto-saves and persists settings to `%APPDATA%\NaNote`.

## 🎹 Keyboard Shortcuts
| Key | Action |
|:---|:---|
| `Ctrl + S` | Save current file |
| `Ctrl + O` | Open file |
| `Ctrl + N` | Save as / New file |
| `Ctrl + D` | Toggle Dark Mode |
| `Ctrl + F` | Floating Search |
| `Ctrl + T` | Toggle Always-on-Top |
| `Ctrl + L` | Toggle Read-only (Lock) |
| `Ctrl + 1~4` | Switch Encodings (U8 / U16 / SJIS / L1) |
| `F5` | Insert Timestamp |

## 🛠️ Build
To reproduce the 8KB binary, you need the MSVC compiler (cl.exe).

```cmd
cl NaNote.c /DUNICODE /D_UNICODE /O1 /GS- /Fe:NaNote.exe /link /NODEFAULTLIB /ENTRY:entry /SUBSYSTEM:WINDOWS /MERGE:.rdata=.text kernel32.lib user32.lib gdi32.lib shell32.lib comdlg32.lib advapi32.lib