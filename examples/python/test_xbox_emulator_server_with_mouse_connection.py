from connections.tcp import TCP
from connections.parser import SequenceParser
import time, os
import tkinter as tk

def get_default_config_path():
    """Get default paths for configuration files based on the current script's location."""
    # Get the directory of the current script
    script_dir = os.path.dirname(os.path.abspath(__file__))

    # Construct default paths
    return os.path.join(script_dir, "configurations", "mouse_emulator_server_test_commands.tsv")

def test_xbox_emulator_server_with_mouse_connection(ip="127.0.0.1", gamepad_port: int = 6053, mouse_port: int = 6054, config_file="mouse_loop_config.tsv"):
    # Connect to the analog input server
    conn = TCP(ip, gamepad_port=gamepad_port, mouse_port=mouse_port)

    # Load the loop sequence using the SequenceParser
    parser = SequenceParser(config_file)
    if parser.load_config():
        loop_sequence, total_loop_time, loop_indices = parser.get_sequence()
        loop_max_idx = round(total_loop_time / parser.loop_delay)
    else:
        # Default loop settings if configuration file is missing
        loop_sequence = None
        loop_indices = []
        loop_max_idx = 650

    # Create a tkinter window
    root = tk.Tk()
    root.title("Close To Exit Test Loop")
    root.geometry("400x200")
    root.configure(bg="black")

    label = tk.Label(root, text="Close This Window to Stop", fg="white", bg="black", font=("Arial", 14))
    label.pack(expand=True)

    # Bring the window to the front
    root.lift()
    root.attributes('-topmost', True)
    root.update()
    root.attributes('-topmost', False)

    running = True

    def on_close():
        """Handle window close event."""
        nonlocal running
        running = False
        root.destroy()

    root.protocol("WM_DELETE_WINDOW", on_close)

    try:
        # Test loop runs as long as the window is open
        loop_idx = 0
        while running:
            if loop_sequence:
                # Use loaded configuration
                if loop_idx in loop_indices:
                    action_idx = loop_indices.index(loop_idx)
                    action_type, arg1, arg2, _ = loop_sequence[action_idx]

                    if action_type == 0:
                        conn.send_digital_input(arg1, arg2)
                    elif action_type == 1:
                        dx = int(arg1)
                        dy = int(arg2)
                        conn.send_mouse_movement_input(dx, dy)
                    elif action_type == 2:
                        button = arg1
                        button_state = arg2
                        conn.send_mouse_button_input(button, button_state)
            else:
                # Default loop behavior
                if loop_idx == 0:
                    conn.send_mouse_movement_input(0, -250)  # Move up
                elif loop_idx == 50:
                    conn.send_mouse_movement_input(0, 250)  # Move down
                elif loop_idx == 100:
                    conn.send_mouse_movement_input(-250, 0)  # Move left
                elif loop_idx == 150:
                    conn.send_mouse_movement_input(250, 0)  # Move right
                elif loop_idx == 200:
                    conn.send_mouse_button_input('left', 'down')  # Left mouse button down
                elif loop_idx == 250:
                    conn.send_mouse_button_input('left', 'up')  # Left mouse button up
                elif loop_idx == 300:
                    conn.send_mouse_button_input('right', 'down')  # Right mouse button down
                elif loop_idx == 350:
                    conn.send_mouse_button_input('right', 'up')  # Right mouse button up

            # Update the tkinter window to handle events
            root.update()
            time.sleep(parser.loop_delay)
            loop_idx = (loop_idx + 1) % loop_max_idx

    except tk.TclError:
        # Handle the case where the window is closed abruptly
        print("Window closed.")
    finally:
        # Close the connection
        conn.close()

if __name__ == "__main__":
    config_file = get_default_config_path()
    test_xbox_emulator_server_with_mouse_connection(config_file=config_file)
