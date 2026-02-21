/**
 * terminal.c - Low-level terminal I/O implementation
 *
 * Manages terminal modes using POSIX termios for raw input.
 * Raw mode allows single key press detection without Enter,
 * essential for interactive controls.
 *
 * Always restore normal mode on exit to prevent terminal corruption.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include "terminal.h"

// Store original terminal settings for restoration
static struct termios original_termios;

void terminal_raw_mode(void)
{
    if (tcgetattr(STDIN_FILENO, &original_termios) == -1)
    {
        perror("tcgetattr");
        return;
    }

    struct termios raw = original_termios;

    // Disable canonical mode and echo
    raw.c_lflag &= ~(ICANON | ECHO);
    // Set minimum characters to read and timeout
    raw.c_cc[VMIN] = 0;  // Non-blocking read
    raw.c_cc[VTIME] = 0; // No timeout

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    {
        perror("tcsetattr");
        return;
    }
}

void terminal_normal_mode(void)
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios) == -1)
    {
        perror("tcsetattr");
        return;
    }
}

int terminal_read_char(void)
{
    unsigned char c;
    ssize_t nread = read(STDIN_FILENO, &c, 1);
    if (nread > 0)
    {
        return c;
    }
    return -1; // No character available
}

int terminal_read_line(char *buffer, int max_len)
{
    int pos = 0;

    while (pos < max_len - 1)
    {
        int ch = terminal_read_char();

        if (ch == -1)
        {
            usleep(10000); // Sleep a bit to avoid spinning
            continue;
        }

        // Enter key - finish reading
        if (ch == '\n' || ch == '\r')
        {
            buffer[pos] = '\0';
            printf("\n");
            return pos;
        }

        // Backspace
        if (ch == 127 || ch == '\b')
        {
            if (pos > 0)
            {
                pos--;
                printf("\b \b");
                fflush(stdout);
            }
            continue;
        }

        // Skip control characters except tab
        if (ch < 32 && ch != '\t')
        {
            continue;
        }

        // Add character to buffer and echo it
        buffer[pos++] = ch;
        printf("%c", ch);
        fflush(stdout);
    }

    buffer[pos] = '\0';
    return pos;
}