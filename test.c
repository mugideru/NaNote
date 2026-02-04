#include <windows.h>
#include <shellapi.h>
#include <commdlg.h>

// --- 1. 型定義 (OFNのサイズ節約用) ---
typedef struct {
    DWORD lStructSize; HWND hwndOwner; HINSTANCE hInstance; LPCWSTR lpstrFilter;
    LPWSTR lpstrCustomFilter; DWORD nMaxCustFilter; DWORD nFilterIndex;
    LPWSTR lpstrFile; DWORD nMaxFile; LPWSTR lpstrFileTitle; DWORD nMaxFileTitle;
    LPCWSTR lpstrInitialDir; LPCWSTR lpstrTitle; DWORD Flags; WORD nFileOffset;
    WORD nFileExtension; LPCWSTR lpstrDefExt; LPARAM lCustData;
    LPOFNHOOKPROC lpfnHook; LPCWSTR lpTemplateName;
} MY_OFNW;

// --- 2. グローバル変数 ---
WNDPROC oldEditProc;
HBRUSH hBlackBrush;
HWND hEdit;
WCHAR currentFn[MAX_PATH];
WCHAR titleBuf[MAX_PATH + 64];
int isDark = 0, isLocked = 0, isDirty = 0, isTopmost = 0, isWrap = 0;
UINT currentCP = 1200;
WCHAR appDataPath[MAX_PATH];
// --- 3. ユーティリティ ---
#pragma function(memcpy)
void* memcpy(void* d, const void* s, size_t n) {
    char* dd = (char*)d; const char* ss = (char*)s;
    while (n--) *dd++ = *ss++; return d;
}
#pragma function(memset)
void* memset(void* dest, int c, size_t n) {
    unsigned char* p = (unsigned char*)dest;
    while (n--) *p++ = (unsigned char)c; return dest;
}

void catW(WCHAR** d, const WCHAR* s) {
    while (*s) { **d = *s; (*d)++; s++; }
}

WCHAR* MyStrStr(WCHAR* hay, WCHAR* needle) {
    if (!*needle) return hay;
    for (; *hay; hay++) {
        WCHAR* h = hay; WCHAR* n = needle;
        while (*h && *n) {
            WCHAR ch1 = (*h >= 'A' && *h <= 'Z') ? *h + 32 : *h;
            WCHAR ch2 = (*n >= 'A' && *n <= 'Z') ? *n + 32 : *n;
            if (ch1 != ch2) break;
            h++; n++;
        }
        if (!*n) return hay;
    }
    return 0;
}

// --- 4. ロジック ---

void SyncCfg(int mode) {
    HKEY hKey;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\NaNote", 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, NULL) == 0) {
        DWORD sz = sizeof(int);
        if (mode == 0) { // 読み込み
            RegQueryValueExW(hKey, L"Dark", NULL, NULL, (LPBYTE)&isDark, &sz);
        } else { // 保存
            RegSetValueExW(hKey, L"Dark", 0, REG_DWORD, (const BYTE*)&isDark, sz);
        }
        RegCloseKey(hKey);
    }
}

void IO(int mode, HWND hWnd, UINT cp) {
    if (!currentFn[0]) return;
    const WCHAR* cpN = (cp==65001)?L"[U8] ":(cp==1200)?L"[U16] ":(cp==932)?L"[SJ] ":L"[L1] ";
    WCHAR* tp = titleBuf;
    if (isDirty) catW(&tp, L"* ");
    if (isTopmost) catW(&tp, L"[T] ");
    catW(&tp, cpN);
    if (isLocked) catW(&tp, L"(L) ");
    catW(&tp, currentFn);
    *tp = 0;
    SetWindowTextW(hWnd, titleBuf);

    if (mode == 2) return;

    HANDLE h = CreateFileW(currentFn, mode == 1 ? GENERIC_WRITE : GENERIC_READ, 0, NULL, 
                           mode == 1 ? CREATE_ALWAYS : OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;

    if (mode == 1) { 
        int len = GetWindowTextLengthW(hEdit);
        WCHAR *wbuf = (WCHAR*)GlobalAlloc(GPTR, (len + 1) * 2);
        if (wbuf) {
            GetWindowTextW(hEdit, wbuf, len + 1);
            DWORD bw;
            if (cp == 1200) {
                WriteFile(h, L"\xFEFF", 2, &bw, NULL);
                WriteFile(h, wbuf, len * 2, &bw, NULL);
            } else {
                int mlen = WideCharToMultiByte(cp, 0, wbuf, len, NULL, 0, NULL, NULL);
                char* mbuf = (char*)GlobalAlloc(GPTR, mlen + 1);
                if (mbuf) {
                    WideCharToMultiByte(cp, 0, wbuf, len, mbuf, mlen, NULL, NULL);
                    WriteFile(h, mbuf, mlen, &bw, NULL);
                    GlobalFree(mbuf);
                }
            }
            GlobalFree(wbuf);
        }
    } else { 
        DWORD sz = GetFileSize(h, NULL), br;
        if (sz != 0 && sz != INVALID_FILE_SIZE) {
            BYTE *buf = (BYTE*)GlobalAlloc(GPTR, sz + 2); 
            if (buf) {
                ReadFile(h, buf, sz, &br, NULL);
                buf[sz] = buf[sz+1] = 0;
                if (cp == 1200) {
                    int skip = (buf[0] == 0xFF && buf[1] == 0xFE) ? 2 : 0;
                    SetWindowTextW(hEdit, (WCHAR*)(buf + skip));
                }
                GlobalFree(buf);
            }
        } else { SetWindowTextW(hEdit, L""); }
    }
    CloseHandle(h);
}

// --- 5. プロシージャ ---
LRESULT CALLBACK SearchProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (m == WM_CHAR && w == VK_RETURN) return 0;
    if (m == WM_KEYDOWN && w == VK_RETURN) {
        WCHAR sBuf[64];
        if (GetWindowTextW(h, sBuf, 64) > 0) {
            LRESULT sel = SendMessageW(hEdit, EM_GETSEL, 0, 0);
            int len = GetWindowTextLengthW(hEdit);
            WCHAR* text = (WCHAR*)GlobalAlloc(GPTR, (len + 1) * 2);
            if (text) {
                GetWindowTextW(hEdit, text, len + 1);
                WCHAR* found = MyStrStr(text + (LOWORD(sel)), sBuf);
                if (!found) found = MyStrStr(text, sBuf);
                if (found) {
                    int pos = (int)(found - text);
                    SendMessageW(hEdit, EM_SETSEL, pos, pos + lstrlenW(sBuf));
                    SendMessageW(hEdit, EM_SCROLLCARET, 0, 0);
                }
                GlobalFree(text);
            }
        }
        return 0;
    }
    if (m == WM_KEYDOWN && w == VK_ESCAPE) { ShowWindow(h, SW_HIDE); SetFocus(hEdit); return 0; }
    return CallWindowProcW(oldEditProc, h, m, w, l);
}

LRESULT CALLBACK W(HWND n, UINT m, WPARAM w, LPARAM l) {
    if (m == WM_CREATE) {
        hEdit = CreateWindowExW(0, L"EDIT", 0, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | 0x100, 0, 0, 0, 0, n, 0, 0, 0);
        DragAcceptFiles(n, TRUE);
        IO(0, n, 1200);
    } else if (m == WM_COMMAND && HIWORD(w) == EN_CHANGE) {
        if (!isDirty) { isDirty = 1; IO(2, n, currentCP); }
    } else if (m == WM_DROPFILES) {
        if (DragQueryFileW((HDROP)w, 0, currentFn, MAX_PATH)) { isDirty = 0; IO(0, n, currentCP); }
        DragFinish((HDROP)w);
    } else if (m == WM_DESTROY) {
        if (!isLocked) IO(1, n, currentCP);
        PostQuitMessage(0);
    } else if (m == WM_SIZE) {
        MoveWindow(hEdit, 0, 0, LOWORD(l), HIWORD(l), TRUE);
    } else if (m == WM_CTLCOLOREDIT) {
        if (isDark) {
            SetTextColor((HDC)w, 0x00FFFFFF); SetBkColor((HDC)w, 0x00202020);
            return (LRESULT)hBlackBrush;
        }
    }
    return DefWindowProcW(n, m, w, l);
}

void entry() {
    WCHAR appDataPath[MAX_PATH];
    WCHAR rootDir[MAX_PATH];
    // 1. 環境変数 "APPDATA" (C:\Users\Name\AppData\Roaming) を取得
    DWORD len = GetEnvironmentVariableW(L"APPDATA", appDataPath, MAX_PATH);
    
    // 2. パスを連結して "C:\...\AppData\Roaming\NaNote" を作成
    WCHAR* p = rootDir;
    const WCHAR* s = appDataPath;
    while (*s) *p++ = *s++;
    const WCHAR* sub = L"\\NaNote";
    while (*sub) *p++ = *sub++;
    *p = 0;

    // 3. フォルダ作成
    CreateDirectoryW(rootDir, NULL);

    // 4. "...\NaNote\base.txt" を作成して currentFn にセット
    p = currentFn;
    s = rootDir;
    while (*s) *p++ = *s++;
    sub = L"\\base.txt";
    while (*sub) *p++ = *sub++;
    *p = 0;

    // 描画などの初期化
    hBlackBrush = CreateSolidBrush(0x00202020);
    SyncCfg(0);
    NONCLIENTMETRICSW ncm = { sizeof(ncm) };
    SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
    HFONT hFont = CreateFontIndirectW(&ncm.lfMessageFont); // メッセージボックス用の標準フォントを取得
    HWND hSearchEdit = 0;

    WNDCLASSW wc = {0};
    wc.lpfnWndProc = W; wc.lpszClassName = L"B";
    wc.hCursor = LoadCursorW(NULL, (LPCWSTR)32512);
    wc.hbrBackground = (HBRUSH)6;
    RegisterClassW(&wc);
    
    HWND n = CreateWindowExW(0, L"B", L"NaNote", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, 600, 400, 0, 0, 0, 0);
    if (n) {
        ShowWindow(n, 5);
        SendMessageW(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
        MSG msg;
        while (GetMessageW(&msg, 0, 0, 0)) {
            if (msg.message == WM_KEYDOWN) {
                if (GetKeyState(VK_CONTROL) & 0x8000) {
                    if (msg.wParam >= '1' && msg.wParam <= '4') {
                        currentCP = (msg.wParam=='1')?65001:(msg.wParam=='2')?1200:(msg.wParam=='3')?1252:932;
                        isDirty = 0; IO(0, n, currentCP); continue;
                    }
                    if (msg.wParam == 'A') { SendMessageW(hEdit, 0x00B1, 0, -1); continue; }
                    if (msg.wParam == 'D') {
                        isDark = !isDark;
                        SyncCfg(1); // レジストリへ保存
                        InvalidateRect(n, NULL, TRUE);
                        continue;
                    }
                    if (msg.wParam == 'F') {
                        RECT rc;
                        GetWindowRect(n, &rc); // 本体の最新位置を取得
                        int x = rc.right - 210;
                        int y = rc.top + 30;
                        
                        if (!hSearchEdit) {
                            hSearchEdit = CreateWindowExW(0x88, L"EDIT", L"", 0x90800080, x, y, 200, 25, n, 0, 0, 0);
                            SendMessageW(hSearchEdit, WM_SETFONT, (WPARAM)hFont, 1);
                            oldEditProc = (WNDPROC)SetWindowLongPtrW(hSearchEdit, -4, (LONG_PTR)SearchProc);
                        } else {
                            SetWindowPos(hSearchEdit, 0, x, y, 0, 0, 0x0015); // SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW
                        }
                        SetFocus(hSearchEdit);
                        continue;
                    }
                    if (msg.wParam == 'G') {
                        DWORD s, e; SendMessageW(hEdit, EM_GETSEL, (WPARAM)&s, (LPARAM)&e);
                        if (s != e) {
                            int len = GetWindowTextLengthW(hEdit);
                            WCHAR* t = (WCHAR*)GlobalAlloc(GPTR, (len + 1) * 2);
                            if (t) {
                                GetWindowTextW(hEdit, t, len + 1);
                                t[e] = 0; ShellExecuteW(n, L"open", t + s, NULL, NULL, 1);
                                GlobalFree(t);
                            }
                        }
                        continue;
                    }
                    if (msg.wParam == 'L') { isLocked = !isLocked; SendMessageW(hEdit, EM_SETREADONLY, isLocked, 0); IO(2, n, currentCP); continue; }
                    
                    if (msg.wParam == 'N' || msg.wParam == 'O') {
                        WCHAR nFn[MAX_PATH] = {0};
                        MY_OFNW ofn = {0};
                        ofn.lStructSize = 76;
                        ofn.hwndOwner = n;
                        ofn.lpstrFile = nFn;
                        ofn.nMaxFile = MAX_PATH;
                        ofn.lpstrFilter = L"*.txt\0*.txt\0*.*\0*.*\0";
                        ofn.Flags = 0x802; // OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT
                        
                        ofn.lpstrInitialDir = rootDir; 

                        if (msg.wParam == 'N') {
                            if (GetSaveFileNameW((LPOPENFILENAMEW)&ofn)) {
                                memcpy(currentFn, nFn, (lstrlenW(nFn) + 1) * 2);
                                isDirty = 0; IO(1, n, currentCP); IO(2, n, currentCP);
                            }
                        } else {
                            if (GetOpenFileNameW((LPOPENFILENAMEW)&ofn)) {
                                memcpy(currentFn, nFn, (lstrlenW(nFn) + 1) * 2);
                                isDirty = 0; IO(0, n, currentCP);
                            }
                        }
                        continue;
                    }
                    if (msg.wParam == 'O') {
                        WCHAR nFn[MAX_PATH] = {0};
                        MY_OFNW ofn = {0};
                        ofn.lStructSize = 76;
                        ofn.hwndOwner = n;
                        ofn.lpstrFile = nFn;
                        ofn.nMaxFile = MAX_PATH;
                        ofn.lpstrFilter = L"*.txt\0*.txt\0*.*\0*.*\0";
                        ofn.Flags = 0x802;
                        ofn.lpstrInitialDir = appDataPath;
                        if (GetOpenFileNameW((LPOPENFILENAMEW)&ofn)) {
                            // 選択されたパスを現在のファイル名としてコピー
                            memcpy(currentFn, nFn, (lstrlenW(nFn) + 1) * 2);
                            isDirty = 0;
                            IO(0, n, currentCP);
                        }
                        continue;
                    }
                    if (msg.wParam == 'S') { IO(1, n, currentCP); isDirty = 0; IO(2, n, currentCP); continue; }
                    if (msg.wParam == 'T') {
                        isTopmost = !isTopmost;
                        SetWindowPos(n, (HWND)(isTopmost ? -1 : -2), 0, 0, 0, 0, 3);
                        IO(2, n, currentCP); continue;
                    }
              
                    if (msg.wParam == 'W') {
                        isWrap = !isWrap;
                        int len = GetWindowTextLengthW(hEdit);
                        WCHAR* tmp = (WCHAR*)GlobalAlloc(GPTR, (len + 1) * 2);
                        if (tmp) {
                            GetWindowTextW(hEdit, tmp, len + 1);
                            RECT rc; GetClientRect(n, &rc);
                            DestroyWindow(hEdit);
                            DWORD st = WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | 0x100;
                            if (!isWrap) st |= WS_HSCROLL | ES_AUTOHSCROLL;
                            hEdit = CreateWindowExW(0, L"EDIT", tmp, st, 0, 0, rc.right, rc.bottom, n, 0, 0, 0);
                            SendMessageW(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
                            InvalidateRect(n, NULL, TRUE); SetFocus(hEdit); GlobalFree(tmp);
                        }
                        IO(2, n, currentCP); continue;
                    }
                }
                    
                if (msg.wParam == VK_ESCAPE) { if (!isLocked) IO(1, n, currentCP); ExitProcess(0); }
                if (msg.wParam == VK_F5) {
                    SYSTEMTIME st; GetLocalTime(&st);
                    WCHAR ts[24];
                    ts[0]=(st.wYear/1000)+'0'; ts[1]=((st.wYear/100)%10)+'0'; ts[2]=((st.wYear/10)%10)+'0'; ts[3]=(st.wYear%10)+'0'; ts[4]='/';
                    ts[5]=(st.wMonth/10)+'0'; ts[6]=(st.wMonth%10)+'0'; ts[7]='/';
                    ts[8]=(st.wDay/10)+'0'; ts[9]=(st.wDay%10)+'0'; ts[10]=' ';
                    ts[11]=(st.wHour/10)+'0'; ts[12]=(st.wHour%10)+'0'; ts[13]=':';
                    ts[14]=(st.wMinute/10)+'0'; ts[15]=(st.wMinute%10)+'0'; ts[16]=0;
                    SendMessageW(hEdit, EM_REPLACESEL, TRUE, (LPARAM)ts); continue;
                }
            }
            TranslateMessage(&msg); DispatchMessageW(&msg);
        }
    }
    ExitProcess(0);
}