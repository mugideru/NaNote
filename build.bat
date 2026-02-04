@echo off
cl test.c /DUNICODE /D_UNICODE /O1 /GS- /Fe:NaNote.exe ^
/link /NODEFAULTLIB /ENTRY:entry /SUBSYSTEM:WINDOWS /ALIGN:16 ^
kernel32.lib user32.lib gdi32.lib shell32.lib comdlg32.lib advapi32.lib
echo Build Complete! Size:
dir NaNote.exe | find "NaNote.exe"