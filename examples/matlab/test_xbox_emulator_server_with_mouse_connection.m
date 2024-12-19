%TEST_XBOX_EMULATOR_SERVER_WITH_MOUSE_CONNECTION Connect to socket from xbox_emulator_server_with_mouse.exe and run test command loop until figure is closed.  
gamepad = tcpclient("127.0.0.1", 6053);
mouse = tcpclient("127.0.0.1", 6054);

fig = figure('Color','k','Name','Close To Exit Test Loop');

while isvalid(fig)
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
end