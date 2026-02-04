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





# NaNote 🖊️
**8KB Win32 テキストエディタ**

NaNote は、純粋な Win32 API のみを使用し、C ランタイム (CRT) への依存を完全に排除して構築された、超軽量・高性能な Windows 用テキストエディタです。バイナリサイズの最適化を極限まで突き詰め、わずか 8,192 バイト（8KB）の中に実用的な機能を凝縮しました。

## ✨ 特徴
- **圧倒的な極小サイズ**: 合計 **8KB** (8,192 bytes)。フロッピーディスクどころか、数セクタに収まるサイズです。
- **純粋な Win32 実装**: `/NODEFAULTLIB` ビルドにより、システム標準の DLL 以外への依存はゼロ。
- **モダンな外観**: ダークモード搭載 (`Ctrl+D`)。システム既定のフォント（メイリオ等）を自動取得し、視認性にも妥協しません。
- **スマートな検索機能**: 邪魔にならないフローティング検索バー (`Ctrl+F`) を搭載。
- **多彩な文字コード**: UTF-8, UTF-16, Shift-JIS, Latin-1 をサポート。
- **最前面表示**: 作業中にメモを確認しやすい最前面固定モード (`Ctrl+T`)。
- **ポータブル & 自動保存**: `%APPDATA%\NaNote` に自動的に設定と内容を保存。

## 🎹 キーボードショートカット
| キー | 操作 |
|:---|:---|
| `Ctrl + S` | 上書き保存 |
| `Ctrl + O` | ファイルを開く |
| `Ctrl + N` | 名前を付けて保存 / 新規作成 |
| `Ctrl + D` | ダークモード切り替え |
| `Ctrl + F` | 検索窓の表示 |
| `Ctrl + T` | 常に手前に表示 |
| `Ctrl + L` | 読み取り専用（ロック）の切り替え |
| `Ctrl + 1~4` | 文字コード切り替え (U8 / U16 / SJIS / L1) |
| `F5` | タイムスタンプ（現在時刻）の挿入 |

## 🛠️ ビルド方法
この 8KB バイナリを再現するには、MSVC コンパイラ (cl.exe) が必要です。

```cmd
cl NaNote.c /DUNICODE /D_UNICODE /O1 /GS- /Fe:NaNote.exe ^
/link /NODEFAULTLIB /ENTRY:entry /SUBSYSTEM:WINDOWS /MERGE:.rdata=.text ^
kernel32.lib user32.lib gdi32.lib shell32.lib comdlg32.lib advapi32.lib