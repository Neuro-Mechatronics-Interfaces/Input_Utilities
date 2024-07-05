#include <windows.h>
#include <stdio.h>
#include <signal.h>

// Global flag for signal handling
volatile sig_atomic_t flag = 0;

// Signal handler for SIGINT (Ctrl+C)
void sigint_handler(int sig) {
    flag = 1;
}

int main() {
    // Register signal handler for SIGINT (Ctrl+C)
    signal(SIGINT, sigint_handler);

    INPUT input[2];
    memset(input, 0, sizeof(input)); // Clear input structure

    // Set up the input structure for key press
    input[0].type = INPUT_KEYBOARD;
    input[0].ki.wVk = 'Y'; // Virtual key code for 'Y' key
    input[0].ki.dwFlags = 0; // 0 for key press

    // Set up the input structure for key release
    input[1].type = INPUT_KEYBOARD;
    input[1].ki.wVk = 'Y'; // Virtual key code for 'Y' key
    input[1].ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release

    while (!flag) {
        // Simulate key press and release
        SendInput(1, &input[0], sizeof(INPUT));

        // Wait for 300 milliseconds
        Sleep(300);

        SendInput(1, &input[1], sizeof(INPUT));

        // Repeat every 1000 milliseconds (1 second)
        Sleep(700);
    }

    return 0;
}
