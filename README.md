# Input Utilities #
Simple compilable executables written in `c` for emulating keyboard/mouse inputs (Windows 11).  

## Contents ##  
Each `.c` file should have everything in it that's needed to compile.  
* [Controller Emulator Server](#xbox-controller-emulator-server)
* [Key Event Listener (Debugging)](#key-event-listener)


### XBOX Controller Emulator Server ###  

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
The setup is simple. Currently, it's hard-coded to connect on port `6053` so you will follow the general steps of:  
1. Create a TCP/IP client socket connection to port `6053` on `localhost` (`127.0.0.1`).  
2. Send `\r\n`-terminated `char` messages to the socket with structure: `<button><0 or 1>`.  

The first character in the message denotes which button to press or release. The second character (`'0'` or `'1'`) denotes whether to press or release the button (respectively; yes, `'0'` goes with "press").  See the table below for button keys:  
| Button | Character |
| ----: | :-------  |
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

I may add more buttons depending if anybody uses/requests them.  

##### Example Usage #####  
Below is some example MATLAB code I've confirmed works to make Megaman move back and forth and charge/discharge his hand cannon in MegaManX running in SNES9x while I run this emulator server in the background:  
```matlab
client = tcpclient("127.0.0.1", 6053);
writeline(client, 'y0'); % Press (and hold) "Y" 
pause(0.020);
writeline(client, '60'); % Press (and hold) DPAD Right
pause(1.5);
writeline(client, '61'); % Stop running Right
pause(0.020);
writeline(client, '40'); % Start running Left
pause(1.5);
writeline(client, '41'); % Stop running Left
pause(0.020);
% Turn right:
writeline(client, '61');
pause(0.020);
writeline(client, '60');
pause(0.020); 
writeline(client, 'Y1'); % FIRE!!!! (Release "Y")
```

#### Test Application ####
To compile an application that you can run right away to confirm if the server is working as intended, run the following from an `MSYS64` terminal in the root folder of this repository:  
```bash
gcc src/xbox_emulator__simple_test.c -o xbox_emulator__simple_test
```
To run  
```bash
./xbox_emulator__simple_test.exe
``` 
(Or double-click the compiled executable).   

---

### Key Event Listener ### 

[Return](#contents)  

If you're not sure what the windows keycodes should be enumerated as, then run this to get them to print out into a terminal.  

#### Listener Application ####  
To compile
```bash
gcc src/key_event_listener.c -o key_event_listener
```
To run  
```bash
./key_event_listener.exe
``` 
(Or double-click the compiled executable).   
