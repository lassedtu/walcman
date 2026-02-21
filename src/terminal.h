/**
 * terminal.h - Low-level terminal I/O abstraction
 *
 * Provides terminal control for raw mode input (single key presses without
 * waiting for Enter). Uses POSIX termios for non-blocking character input.
 *
 * Call terminal_raw_mode() before reading input, and terminal_normal_mode()
 * before exiting to restore normal terminal behavior.
 */

#ifndef WALCMAN_TERMINAL_H
#define WALCMAN_TERMINAL_H

/**
 * Enable raw mode (single key press input, no echo)
 * Call before starting input loop
 */
void terminal_raw_mode(void);

/**
 * Restore normal terminal mode
 * Call before program exit
 */
void terminal_normal_mode(void);

/**
 * Read a single character without waiting for Enter
 * Non-blocking: returns immediately if no input available
 * Returns: Character code, or -1 if no input
 */
int terminal_read_char(void);

/**
 * Read a full line in raw mode (for file paths, etc)
 * Handles backspace and displays input as typed
 * buffer: Output buffer for line
 * max_len: Maximum buffer size
 * Returns: Length of line read, or 0 if cancelled
 */
int terminal_read_line(char *buffer, int max_len);

#endif // WALCMAN_TERMINAL_H
