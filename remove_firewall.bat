@echo off
setlocal

REM Get the directory of the current script
set scriptDir=%~dp0

REM Define the executable name
set exeName=controller_input_server.exe

REM Full path to the executable
set fullPath=%scriptDir%\%exeName%

REM Remove firewall rules
netsh advfirewall firewall delete rule name="Allow ControllerInputServer Local"

pause
endlocal
