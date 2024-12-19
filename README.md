# Input Utilities #

Simple compilable executables written in `c` for emulating keyboard/mouse inputs (Windows 11).

## Contents ##
Each `.c` file should have everything in it that's needed to compile.
* [Controller Emulator Server](#xbox-controller-emulator-server)
* [Controller Emulator Server **(With Mouse Port)**](#xbox-controller-emulator-server-with-mouse)
* [Key Event Listener (Debugging)](#key-event-listener)

### XBOX Controller Emulator Server With Mouse ###

[Return](#contents)

This application creates a background local loopback server so you can send principled "messages" to the server over TCP/IP on your local device; the messages are translated by this server so that they look like actual keypresses from an XBOX controller, which should be compatible with most 3rd-party applications that you might want to play. So in a nutshell this is very useful if, for example, you are decoding device streams from EMG or other neural data sources in MATLAB or Python, and you want to handle all math and data processing in those higher-level languages. This allows separation/abstraction of the part where you have to generate the "physical key clicks" etc. so that other applications can see your decoded output.

#### Server Application ####
To compile the server application, run the following command from an `MSYS64` terminal in the root folder of this repository:
```bash
gcc src/xbox_controller_emulator.c -o xbox_emulator_server.exe -lws2_32
```
To run the compiled application, from the same terminal you can simply then run:
```bash
./xbox_emulator_server.exe
```
(Or just double-click the compiled executable).
You may receive a security prompt to open a port--this is exposed on local loopback; make sure to allow it.

##### Message API #####
The setup is simple. Currently, it's hard-coded to connect on port `6053` for digital inputs and port `6054` for analog (mouse) inputs.

#### Digital Input Messages ####
1. Create a TCP/IP client socket connection to port `6053` on `localhost` (`127.0.0.1`).
2. Send `\r\n`-terminated `char` messages to the socket with structure: `<button><0 or 1>`.

The first `char` of the message must be one of the following, indicating the button to press:

| Button | Character |
| ---    | ---       |
| `A` | `'a'` or `'A'`|
| `B` | `'b'` or `'B'`|
| `X` | `'x'` or `'X'`|
| `Y` | `'y'` or `'Y'`|
| `L` | `'l'` or `'L'`|
| `R` | `'r'` or `'R'`|
| `DPAD UP`     | `'8'` |
| `DPAD DOWN`   | `'2'` |
| `DPAD LEFT`   | `'4'` |
| `DPAD RIGHT`  | `'6'` |

_(The `DPAD` button `<-->` character mapping is related to the layout of the numbers on a standard keyboard numpad.)_

The second `char` should be either `'0'` (key press) or `'1'` (key release).

##### Example Usage #####
Below is some example MATLAB code I've confirmed works to make Megaman move back and forth and charge/discharge his hand cannon in MegaManX running in SNES9x while I run this emulator server in the background:
```matlab
gamepad = tcpclient("127.0.0.1", 6053);
writeline(gamepad, 'y0'); % Press (and hold) "Y"
pause(0.020);
writeline(gamepad, '60'); % Press (and hold) DPAD Right
pause(1.5);
writeline(gamepad, '61'); % Stop running Right
pause(0.020);
writeline(gamepad, '40'); % Start running Left
pause(1.5);
writeline(gamepad, '41'); % Stop running Left
pause(0.020);
% Turn right:
writeline(gamepad, '61');
pause(0.020);
writeline(gamepad, '60');
pause(0.020); 
writeline(gamepad, 'Y1'); % FIRE!!!! (Release "Y")
```

### XBOX Controller Emulator Server With Mouse ###

[Return](#contents)

This is an extended version of the control emulator from the [previous section](#xbox-controller-emulator-server), which adds in mouse emulation. The mouse message API is described under the **[Analog Input Messages](#analog-input-messages)** section; it supports mouse input to move across an arbitrary screen size using pixel `dx,dy` messages, as well as "left" and "right" button click, hold, and release events and "scrollwheel" "up" and "down" events.  

#### Server Application ####
To compile the server application, run the following command from an `MSYS64` terminal in the root folder of this repository:
```bash
gcc src/xbox_controller_emulator_with_mouse.c -o xbox_emulator_server_with_mouse.exe -lws2_32
```
To run the compiled application, from the same terminal you can simply then run:
```bash
./xbox_emulator_server_with_mouse.exe
```
(Or just double-click the compiled executable).
You may receive a security prompt to open a port--this is exposed on local loopback; make sure to allow it.

##### Message API #####
The setup is simple. Currently, it's hard-coded to connect on port `6053` for digital inputs and port `6054` for analog (mouse) inputs. The API for "Digital" messages is the same as in the [previous section](#digital-input-messages).  

### Analog Input Messages

1. Create a TCP/IP client socket connection to port `6054` on `localhost` (`127.0.0.1`).
2. Send messages to the socket with one of the following structures:

- **Analog Movement**: `<direction>,<dx>,<dy>`
  - `direction`: `'u'`, `'d'`, `'l'`, `'r'`, `'x'` for up, down, left, right, or unspecified movement.
  - `dx`, `dy`: Integer values for movement along the x and y axes.
- **Mouse Clicks**: `click,<button>`
  - `button`: `'left'` or `'right'`.
- **Mouse Wheel Scroll**: `scroll,<direction>`
  - `direction`: `'up'` or `'down'`.
- **Mouse Down/Up**: `<state>,<button>`
  - `state`: `'on'` or `'off'` for mouse-down or mouse-up. 
  - `button`: `'left'` or `'right'`. 

#### Example Usage
Below is some example MATLAB code to simulate analog stick movement, mouse clicks, and mouse wheel scrolls. Note that you **must** open connections to both sockets before it will recognize the controller:
```matlab
gamepad = tcpclient("127.0.0.1", 6053);
mouse = tcpclient("127.0.0.1", 6054);
% Analog movement
writeline(mouse, 'u,0,-250'); % Move up 250 pixels
pause(1.5);
writeline(mouse, 'd,0,250'); % Move down 250 pixels
pause(1.5);
% Mouse wheel scroll
writeline(mouse, 'scroll,up'); % Scroll up
pause(1.5);
writeline(mouse, 'scroll,down'); % Scroll down
pause(1.5);
writeline(mouse, 'click,left'); % Left mouse click
pause(1.5);
writeline(mouse, 'on,left'); % Now, hold the left-mouse down
pause(0.1);
writeline(mouse, 'l,-250,0'); % Move left 250 pixels
pause(0.1);
writeline(mouse, 'off,left'); % Release the left-mouse- may have highlighted text etc.
pause(0.5);
writeline(mouse, 'click,right'); % Right mouse click
pause(1.5);
writeline(mouse, 'r,250,0'); % Move right 250 pixels to return to start
```  
When you are done sending messages from MATLAB, you can use `clear` to close the `gamepad` and `mouse` connections, which will close the server sockets and stop the server application. If you want to re-connect, you need to manually restart the application.  

### Persistent Controller and Mouse Server ###  
This does the same thing as the other two applications, but it persists the socket on ports 6053 and 6054 after the tcp/ip client socket connections are closed. You can compile it from an `MSYS64` terminal at the root folder of this repository, like this:  
```bash
gcc -o controller_input_server.exe src/controller_input_persistent_server.c -lws2_32
```
Then, you can run `controller_input_server.exe` by double-clicking it or from the same `MSYS64` terminal like:  
```bash
./controller_input_server.exe -console
```
Once you have compiled this executable, you can consider installing/using it automatically by running the installation script (`install_service.bat`); however, you should really read the next section first and take the safety considerations under advisement. Basically, you probably want to really make sure to firewall/restrict this application so that the ports 6053 and 6054 are only accessed via safe/internal/local networks.  

## Windows Service for Controller Input Server
**Note: this is not working, but you can compile `controller_input_server.exe` and run that from a terminal as required instead.**

~~### Overview~~  
~~The `ControllerInputService` is a Windows service that automatically starts the `controller_input_server.exe` application on system startup. This service ensures that the controller input server is always running, providing continuous input handling for your setup.~~  

~~### Installation~~  

~~To install the `ControllerInputService`, follow these steps:~~  

~~1. **Download and Extract Repository:**~~  
   ~~Ensure that you have downloaded and extracted the repository containing `install_service.bat` and `controller_input_server.exe` to a desired location on your system.~~  

~~2. **Run the Installation Script:**~~  
   ~~Run the `install_service.bat` script with administrative privileges to install the service:~~  
   ~~- Right-click `install_service.bat` and select "Run as administrator."~~  
   ~~- The script will automatically detect the path to `controller_input_server.exe`, create the service, and configure Windows Firewall to allow access only from localhost and local networks.~~  
   
   ~~The script does the following:~~  
   ~~- Creates the `ControllerInputService` to run `controller_input_server.exe`.~~  
   ~~- Configures Windows Firewall to allow incoming connections on ports 6053 and 6054 only from the localhost (`127.0.0.1`) and local networks (`192.168.0.0/16` and `10.0.0.0/8`).~~  

~~### Uninstallation~~  

~~To uninstall the `ControllerInputService`, follow these steps:~~  

~~1. **Run the Uninstallation Script:**~~
   ~~Run the `uninstall_service.bat` script with administrative privileges to uninstall the service:~~  
   ~~- Right-click `uninstall_service.bat` and select "Run as administrator."~~  

   ~~The script does the following:~~  
   ~~- Stops the `ControllerInputService`.~~  
   ~~- Deletes the `ControllerInputService`.~~  
   ~~- Removes the firewall rules associated with the service.~~  

~~### Safety Considerations~~  

~~- **Administrative Privileges:**~~  
  ~~- The installation script requires administrative privileges to create and manage the Windows service. Ensure you trust the source of the script before running it with elevated permissions.~~  

~~- **Resource Usage:**~~  
  ~~- Running the service continuously may consume system resources. Monitor the system for any performance issues and adjust the service configuration if necessary.~~  

~~- **External Connections:**~~  
  ~~- **Security Risk**: The service listens on network ports, potentially allowing external connections to control the mouse and keyboard. This poses a significant security risk, especially if the service is exposed to external networks.~~  
  ~~- **Firewall Settings**: By default, the installation script configures Windows Firewall to restrict access to the ports used by the service (`6053` and `6054`) to trusted IP addresses only (localhost and local networks).~~  
  ~~- **Network Segmentation**: Ensure the service is only accessible within a secure, segmented network to prevent unauthorized access.~~  
  ~~- **Encryption**: Consider implementing encryption and authentication mechanisms for the data transmitted over the network to prevent interception and unauthorized control.~~  

~~By following these instructions and considerations, you can ensure that the `controller_input_server.exe` application is always running and ready to handle controller inputs while maintaining a secure environment.~~  
