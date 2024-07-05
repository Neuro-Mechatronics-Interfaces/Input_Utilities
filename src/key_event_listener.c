#include <windows.h>
#include <stdio.h>

// Callback function for handling key events
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        if (wParam == WM_KEYDOWN || wParam == WM_KEYUP) {
            KBDLLHOOKSTRUCT *kbdStruct = (KBDLLHOOKSTRUCT *)lParam;
            
            printf("Virtual Key Code: %d\n", kbdStruct->vkCode);
            printf("Scan Code: %d\n", kbdStruct->scanCode);
            printf("Flags: %08X\n", kbdStruct->flags);
            printf("Time: %lu\n", kbdStruct->time);
            printf("Extra Info: %p\n", kbdStruct->dwExtraInfo);
            printf("-------------------------\n");
        }
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int main() {
    // Set hook for low-level keyboard events
    HHOOK hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);

    if (hook == NULL) {
        printf("Failed to set hook\n");
        return 1;
    }

    printf("Listening for key events...\n");

    // Message loop to keep the program running
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Unhook the keyboard hook before exiting
    UnhookWindowsHookEx(hook);

    return 0;
}
