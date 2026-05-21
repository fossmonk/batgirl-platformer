#include "tigr/tigr.h"
#ifdef __WIN32
#include <windows.h>
#endif

void setup_windows_magic(Tigr* s) {
    #ifdef __WIN32
    HWND hwnd = (HWND)s->handle; 

    if (hwnd != NULL) {
        HICON hIcon = LoadIcon(GetModuleHandle(NULL), "MAINICON");
        
        if (hIcon != NULL) {
            SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon); // Title bar (16x16)
            SendMessage(hwnd, WM_SETICON, ICON_BIG,   (LPARAM)hIcon); // Taskbar thumb (32x32)
        }
    }
    #else
    (void)s;
    #endif
}