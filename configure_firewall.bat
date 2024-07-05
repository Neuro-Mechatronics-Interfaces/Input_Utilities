@echo off
setlocal

REM Get the directory of the current script
set scriptDir=%~dp0

REM Define the executable name
set exeName=controller_input_server.exe

REM Full path to the executable
set fullPath=%scriptDir%\%exeName%

REM Add firewall rules to allow access from localhost and local networks
netsh advfirewall firewall add rule name="Allow ControllerInputServer Local" program="%fullPath%" protocol=TCP dir=in localport=6053,6054 remoteip=127.0.0.1,192.168.0.0/16,10.0.0.0/8 action=allow

pause
endlocal
