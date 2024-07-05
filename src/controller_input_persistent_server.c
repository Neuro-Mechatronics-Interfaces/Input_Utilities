#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <signal.h>
#pragma comment(lib, "ws2_32.lib")

#define XBOX_CONTROLLER_EMULATOR_PORT 6053
#define XBOX_ANALOG_EMULATOR_PORT 6054

// Service name
#define SERVICE_NAME "ControllerInputService"

// Global flag for signal handling
volatile sig_atomic_t flag = 0;
INPUT input[3];
POINT p;
int AS_CONSOLE;

// Handle to the service status
SERVICE_STATUS_HANDLE g_ServiceStatusHandle = NULL;
SERVICE_STATUS g_ServiceStatus = {0};

void setServiceStatus(DWORD currentState, DWORD win32ExitCode, DWORD waitHint) {
    static DWORD checkPoint = 1;

    g_ServiceStatus.dwCurrentState = currentState;
    g_ServiceStatus.dwWin32ExitCode = win32ExitCode;
    g_ServiceStatus.dwWaitHint = waitHint;
    g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;

    if (currentState == SERVICE_START_PENDING) {
        g_ServiceStatus.dwControlsAccepted = 0;
    } else if (currentState == SERVICE_RUNNING || currentState == SERVICE_STOPPED) {
        g_ServiceStatus.dwCheckPoint = 0;
    } else {
        g_ServiceStatus.dwCheckPoint = checkPoint++;
    }

    SetServiceStatus(g_ServiceStatusHandle, &g_ServiceStatus);
}

void WINAPI serviceControlHandler(DWORD controlCode) {
    switch (controlCode) {
        case SERVICE_CONTROL_STOP:
        case SERVICE_CONTROL_SHUTDOWN:
            flag = 1;
            setServiceStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);
            return;
        default:
            break;
    }
    setServiceStatus(g_ServiceStatus.dwCurrentState, NO_ERROR, 0);
}

void disableMouseAcceleration() {
    SystemParametersInfo(SPI_SETMOUSE, 0, NULL, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
}

void enableMouseAcceleration() {
    SystemParametersInfo(SPI_SETMOUSE, 1, NULL, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
}

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

void setAnalog(int dx, int dy) {
    if (GetCursorPos(&p)) {
        if (AS_CONSOLE==1) {
            printf("Current Position: (%d, %d)\n", p.x, p.y);
        }
    }

    input[2].type = INPUT_MOUSE;
    input[2].mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
    input[2].mi.dx = (p.x + dx) * 65535 / GetSystemMetrics(SM_CXSCREEN);
    input[2].mi.dy = (p.y + dy) * 65535 / GetSystemMetrics(SM_CYSCREEN);
    input[2].mi.dwExtraInfo = 0;
    input[2].mi.mouseData = 0;
    input[2].mi.time = 0;
    SendInput(1, &input[2], sizeof(INPUT));

    if (GetCursorPos(&p)) {
        if (AS_CONSOLE == 1) {
            printf("New Position: (%d, %d)\n", p.x, p.y);
        }
    }
}

void mouseClick(const char *button) {
    input[2].type = INPUT_MOUSE;
    input[2].mi.dwFlags = (strcmp(button, "left") == 0) ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_RIGHTDOWN;
    SendInput(1, &input[2], sizeof(INPUT));

    input[2].mi.dwFlags = (strcmp(button, "left") == 0) ? MOUSEEVENTF_LEFTUP : MOUSEEVENTF_RIGHTUP;
    SendInput(1, &input[2], sizeof(INPUT));
}

void mouseOn(const char *button) {
    input[2].type = INPUT_MOUSE;
    input[2].mi.dwFlags = (strcmp(button, "left") == 0) ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_RIGHTDOWN;
    SendInput(1, &input[2], sizeof(INPUT));
}

void mouseOff(const char *button) {
    input[2].type = INPUT_MOUSE;
    input[2].mi.dwFlags = (strcmp(button, "left") == 0) ? MOUSEEVENTF_LEFTUP : MOUSEEVENTF_RIGHTUP;
    SendInput(1, &input[2], sizeof(INPUT));
}

void mouseScroll(const char *direction) {
    input[2].type = INPUT_MOUSE;
    input[2].mi.dwFlags = MOUSEEVENTF_WHEEL;
    input[2].mi.mouseData = (strcmp(direction, "up") == 0) ? WHEEL_DELTA : -WHEEL_DELTA;
    SendInput(1, &input[2], sizeof(INPUT));
}

void runServer() {
    disableMouseAcceleration();

    memset(input, 0, sizeof(input)); // Clear input structure

    // Set up the input structure for key press
    input[0].type = INPUT_KEYBOARD;
    input[0].ki.dwFlags = 0; // 0 for key press

    // Set up the input structure for key release
    input[1].type = INPUT_KEYBOARD;
    input[1].ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release

    while (!flag) {
        // Initialize Winsock
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            return;
        }

        SOCKET listenSocketDigital = INVALID_SOCKET;
        SOCKET listenSocketAnalog = INVALID_SOCKET;

        // Create socket for digital inputs
        listenSocketDigital = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listenSocketDigital == INVALID_SOCKET) {
            printf("Error creating socket: %d\n", WSAGetLastError());
            WSACleanup();
            continue;
        }

        // Bind socket for digital inputs
        struct sockaddr_in serverAddrDigital;
        serverAddrDigital.sin_family = AF_INET;
        serverAddrDigital.sin_addr.s_addr = htonl(INADDR_ANY);
        serverAddrDigital.sin_port = htons(XBOX_CONTROLLER_EMULATOR_PORT);

        if (bind(listenSocketDigital, (struct sockaddr *)&serverAddrDigital, sizeof(serverAddrDigital)) == SOCKET_ERROR) {
            printf("Bind failed with error: %d\n", WSAGetLastError());
            closesocket(listenSocketDigital);
            WSACleanup();
            continue;
        }

        // Listen for incoming connections on digital input port
        if (listen(listenSocketDigital, SOMAXCONN) == SOCKET_ERROR) {
            printf("Listen failed with error: %d\n", WSAGetLastError());
            closesocket(listenSocketDigital);
            WSACleanup();
            continue;
        }

        // Create socket for analog inputs
        listenSocketAnalog = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listenSocketAnalog == INVALID_SOCKET) {
            printf("Error creating socket: %d\n", WSAGetLastError());
            closesocket(listenSocketDigital);
            WSACleanup();
            continue;
        }

        // Bind socket for analog inputs
        struct sockaddr_in serverAddrAnalog;
        serverAddrAnalog.sin_family = AF_INET;
        serverAddrAnalog.sin_addr.s_addr = htonl(INADDR_ANY);
        serverAddrAnalog.sin_port = htons(XBOX_ANALOG_EMULATOR_PORT);

        if (bind(listenSocketAnalog, (struct sockaddr *)&serverAddrAnalog, sizeof(serverAddrAnalog)) == SOCKET_ERROR) {
            printf("Bind failed with error: %d\n", WSAGetLastError());
            closesocket(listenSocketDigital);
            closesocket(listenSocketAnalog);
            WSACleanup();
            continue;
        }

        // Listen for incoming connections on analog input port
        if (listen(listenSocketAnalog, SOMAXCONN) == SOCKET_ERROR) {
            printf("Listen failed with error: %d\n", WSAGetLastError());
            closesocket(listenSocketDigital);
            closesocket(listenSocketAnalog);
            WSACleanup();
            continue;
        }

        if (AS_CONSOLE == 1) {
            printf("XBOX Controller Emulator Running on Ports %d (Digital) and %d (Analog)!\n", XBOX_CONTROLLER_EMULATOR_PORT, XBOX_ANALOG_EMULATOR_PORT);
        }
        
        // Accept incoming connections
        SOCKET clientSocketDigital;
        struct sockaddr_in clientAddrDigital;
        int clientAddrLenDigital = sizeof(clientAddrDigital);

        clientSocketDigital = accept(listenSocketDigital, (struct sockaddr *)&clientAddrDigital, &clientAddrLenDigital);
        if (clientSocketDigital == INVALID_SOCKET) {
            printf("Accept failed with error: %d\n", WSAGetLastError());
            closesocket(listenSocketDigital);
            closesocket(listenSocketAnalog);
            continue;
        }

        // Set client socket to non-blocking mode
        u_long mode = 1;
        ioctlsocket(clientSocketDigital, FIONBIO, &mode);

        SOCKET clientSocketAnalog;
        struct sockaddr_in clientAddrAnalog;
        int clientAddrLenAnalog = sizeof(clientAddrAnalog);

        clientSocketAnalog = accept(listenSocketAnalog, (struct sockaddr *)&clientAddrAnalog, &clientAddrLenAnalog);
        if (clientSocketAnalog == INVALID_SOCKET) {
            printf("Accept failed with error: %d\n", WSAGetLastError());
            closesocket(listenSocketDigital);
            closesocket(listenSocketAnalog);
            continue;
        }

        // Set client socket to non-blocking mode
        ioctlsocket(clientSocketAnalog, FIONBIO, &mode);

        if (AS_CONSOLE==1) {
            printf("XBOX Controller connected.\n");
        }

        char buffer[5]; // Buffer to store received message
        char analogBuffer[100]; // Buffer for analog input

        while (!flag) {
            // Check for digital input
            int bytesReceivedDigital = recv(clientSocketDigital, buffer, sizeof(buffer), 0);
            if (bytesReceivedDigital > 0) {
                // Extract the key from the received message
                char key = buffer[0];
                char updown = buffer[1];
                
                // Set the key for input
                setKey(key, updown);
            }
            else if (bytesReceivedDigital == 0) {
                if (AS_CONSOLE==1) {
                    printf("Client disconnected from digital input\n");
                }
                break;
            }
            else if (bytesReceivedDigital == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK) {
                if (AS_CONSOLE==1) {
                    printf("Receive failed with error: %d\n", WSAGetLastError());
                }
                break;
            }

            // Check for analog input
            int bytesReceivedAnalog = recv(clientSocketAnalog, analogBuffer, sizeof(analogBuffer), 0);
            if (bytesReceivedAnalog > 0) {
                // Determine the type of analog message
                char type[10];
                sscanf(analogBuffer, "%9[^,]", type);

                if (strcmp(type,"x") == 0 || strcmp(type, "u") == 0 || strcmp(type, "d") == 0 || strcmp(type, "l") == 0 || strcmp(type, "r") == 0) {
                    // Expected format: "<direction>,<dx>,<dy>\n"
                    char direction[2];
                    int dx, dy;
                    sscanf(analogBuffer, "%1s,%d,%d", direction, &dx, &dy);
                    
                    if (AS_CONSOLE==1) {
                        // Debug print statements
                        printf("Analog Input Received: Direction: %s, dx: %d, dy: %d\n", direction, dx, dy);
                    }
                    
                    // Set the analog input
                    setAnalog(dx, dy);
                } else if (strcmp(type, "click") == 0) {
                    // Expected format: "click,<button>\n"
                    char button[6];
                    sscanf(analogBuffer, "click,%5s", button);

                    if (AS_CONSOLE==1) {
                        // Debug print statements
                        printf("Mouse Click Received: Button: %s\n", button);
                    }

                    // Set the mouse click
                    mouseClick(button);
                } else if (strcmp(type, "scroll") == 0) {
                    // Expected format: "scroll,<direction>\n"
                    char direction[6];
                    sscanf(analogBuffer, "scroll,%5s", direction);

                    if (AS_CONSOLE==1) {
                        // Debug print statements
                        printf("Mouse Scroll Received: Direction: %s\n", direction);
                    }
                    
                    // Set the mouse scroll
                    mouseScroll(direction);
                } else if (strcmp(type, "on") == 0) {
                    // Expected format: "hold,<button>\n"
                    char button[6];
                    sscanf(analogBuffer, "on,%5s", button);

                    if (AS_CONSOLE==1) {
                        // Debug print statements
                        printf("Mouse ON Received: Button: %s\n", button);
                    }
                    

                    // Set the mouse button as clicked/dragging
                    mouseOn(button);
                }  else if (strcmp(type, "off") == 0) {
                    // Expected format: "hold,<button>\n"
                    char button[6];
                    sscanf(analogBuffer, "off,%5s", button);

                    if (AS_CONSOLE==1) {
                        // Debug print statements
                        printf("Mouse OFF Received: Button: %s\n", button);
                    }
                    
                    // Set the mouse click
                    mouseOff(button);
                }
            }
            else if (bytesReceivedAnalog == 0) {
                if (AS_CONSOLE==1) {
                    printf("Client disconnected from analog input\n");
                }
                break;
            }
            else if (bytesReceivedAnalog == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK) {
                if (AS_CONSOLE==1) {
                    printf("Receive failed with error: %d\n", WSAGetLastError());
                }
                break;
            }

            Sleep(10); // Add a small delay to prevent tight loop
        }

        // Cleanup
        closesocket(clientSocketDigital);
        closesocket(clientSocketAnalog);
        closesocket(listenSocketDigital);
        closesocket(listenSocketAnalog);
        WSACleanup();
    }

    enableMouseAcceleration();
}

void WINAPI serviceMain(DWORD argc, LPSTR *argv) {
    g_ServiceStatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, serviceControlHandler);
    if (g_ServiceStatusHandle == NULL) {
        return;
    }

    // Run the server
    runServer();

    setServiceStatus(SERVICE_STOPPED, NO_ERROR, 0);
}

int main(int argc, char *argv[]) {
    if (argc > 1 && strcmp(argv[1], "-console") == 0) {
        AS_CONSOLE = 1;
        // Run as a console application
        runServer();
    } else {
        AS_CONSOLE = 0;
        // Run as a service
        SERVICE_TABLE_ENTRY ServiceTable[] = {
            {SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)serviceMain},
            {NULL, NULL}
        };

        if (StartServiceCtrlDispatcher(ServiceTable) == FALSE) {
            return GetLastError();
        }
    }

    return 0;
}
