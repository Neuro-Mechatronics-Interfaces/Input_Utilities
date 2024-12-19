%TEST_XBOX_EMULATOR_SERVER_CONNECTION Connect to socket from xbox_emulator_server.exe and run test command loop until figure is closed.  

gamepad = tcpclient("127.0.0.1", 6053);
fig = figure('Color','k','Name','Close To Exit Test Loop');

while isvalid(fig)
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
    pause(2.00);
end