#ifndef WALCMAN_TERMINAL_H
#define WALCMAN_TERMINAL_H

// Initialize terminal for raw input (single key presses)
void terminal_raw_mode(void);

// Restore terminal to normal mode
void terminal_normal_mode(void);

// Read a single character without waiting for Enter
int terminal_getchar(void);

// Read a full line in raw mode (for file paths, etc)
int terminal_readline(char *buffer, int max_len);

#endif // WALCMAN_TERMINAL_H
