@echo off
setlocal

REM Get the directory of the current script
set scriptDir=%~dp0

REM Remove trailing backslash
set scriptDir=%scriptDir:~0,-1%

REM Define the executable name
set exeName=controller_input_server.exe

REM Full path to the executable
set fullPath=%scriptDir%\%exeName%

REM Create the service with a display name
sc create ControllerInputService binPath= "%fullPath%" start= auto DisplayName= "Controller Input Server"
sc description ControllerInputService "Service to launch controller input server"

REM Configure the service to run under the Local System account and allow interaction with the desktop
sc config ControllerInputService obj= LocalSystem type= own type= interact

sc start ControllerInputService

REM Configure Windows Firewall to allow access from localhost and local networks
netsh advfirewall firewall add rule name="Allow ControllerInputService Local" protocol=TCP dir=in localport=6053,6054 remoteip=127.0.0.1,192.168.0.0/16,10.0.0.0/8 action=allow

pause
endlocal
