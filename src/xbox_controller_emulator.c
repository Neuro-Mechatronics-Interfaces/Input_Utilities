#include <winsock2.h>
#include <stdio.h>
#include <signal.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")

#define XBOX_CONTROLLER_EMULATOR_PORT 6053

// Global flag for signal handling
volatile sig_atomic_t flag = 0;

// Signal handler for SIGINT (Ctrl+C)
void sigint_handler(int sig) {
    flag = 1;
}

INPUT input[2];

void setKey(char key, char updown) {
    switch (key)
    {
        case 'a':
        case 'A': {
            input[updown - '0'].ki.wVk = 'A';
            break;
        }
        case 'b':
        case 'B': {
            input[updown - '0'].ki.wVk = 'B';
            break;
        }
        case 'x':
        case 'X': {
            input[updown - '0'].ki.wVk = 'X';
            break;
        }
        case 'y':
        case 'Y': {
            input[updown - '0'].ki.wVk = 'Y';
            break;
        }
        case 'l':
        case 'L': {
            input[updown - '0'].ki.wVk = 'L';
            break;
        }
        case 'r':
        case 'R': {
            input[updown - '0'].ki.wVk = 'R';
            break;
        }
        case '4': { // left
            input[updown - '0'].ki.wVk = 0x25;
            break;
        }
        case '6': { // right
            input[updown - '0'].ki.wVk = 0x27;
            break;
        }
        case '2': { // down
            input[updown - '0'].ki.wVk = 0x28;
            break;
        }
        case '8': { // up
            input[updown - '0'].ki.wVk = 0x26;
            break;
        }
    }    
    SendInput(1, &input[updown - '0'], sizeof(INPUT));
}

int main() {
    // Register signal handler for SIGINT (Ctrl+C)
    signal(SIGINT, sigint_handler);
    memset(input, 0, sizeof(input)); // Clear input structure

    // Set up the input structure for key press
    input[0].type = INPUT_KEYBOARD;
    input[0].ki.dwFlags = 0; // 0 for key press

    // Set up the input structure for key release
    input[1].type = INPUT_KEYBOARD;
    input[1].ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release

    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }

    // Create socket
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        printf("Error creating socket: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Bind socket
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(XBOX_CONTROLLER_EMULATOR_PORT); // Use the desired port number

    if (bind(listenSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Bind failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // Listen for incoming connections
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("Listen failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    printf("XBOX Controller Emulator Running on Port 6053!\n");

    // Accept incoming connection
    SOCKET clientSocket;
    struct sockaddr_in clientAddr;
    int clientAddrLen = sizeof(clientAddr);

    clientSocket = accept(listenSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
    if (clientSocket == INVALID_SOCKET) {
        printf("Accept failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    printf("XBOX Controller connected.\n");

    char buffer[5]; // Buffer to store received message

    while (!flag) {
        // Receive message from client
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            // Extract the key from the received message
            char key = buffer[0];
            char updown = buffer[1];
            
            // Set the key for input
            setKey(key, updown);
        }
        else if (bytesReceived == 0) {
            printf("Client disconnected\n");
            break;
        }
        else {
            printf("Receive failed with error: %d\n", WSAGetLastError());
            break;
        }
    }

    // Cleanup
    closesocket(clientSocket);
    closesocket(listenSocket);
    WSACleanup();

    return 0;
}
