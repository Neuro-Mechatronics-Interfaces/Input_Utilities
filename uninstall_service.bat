@echo off
setlocal

REM Stop the service
sc stop ControllerInputService

REM Wait for the service to stop
timeout /t 5 /nobreak

REM Check if the service is still running
sc query ControllerInputService | findstr /i "STOPPED"
if not errorlevel 1 (
    echo Service stopped successfully
) else (
    echo Service did not stop, attempting to force stop
    REM Get the PID of the service process
    for /f "tokens=2 delims=," %%i in ('tasklist /svc /FI "SERVICES eq ControllerInputService" /FO CSV /NH') do set pid=%%i
    echo Terminating process with PID %pid%
    taskkill /F /PID %pid%
)

REM Attempt to create socket connections to force the server past any blocking parts
powershell -Command "& { $tcpClient = New-Object System.Net.Sockets.TcpClient; $tcpClient.Connect('127.0.0.1', 6053); $tcpClient.Close(); $tcpClient = New-Object System.Net.Sockets.TcpClient; $tcpClient.Connect('127.0.0.1', 6054); $tcpClient.Close(); }"

REM Delete the service
sc delete ControllerInputService

REM Remove firewall rules
netsh advfirewall firewall delete rule name="Allow ControllerInputService Local"

pause
endlocal
