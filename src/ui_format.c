/**
 * ui_format.c - Text formatting utilities implementation
 *
 * Provides formatting functions for UI display:
 * - Time conversion to human-readable format
 * - Filename extraction from paths
 * - Progress bar visual generation
 * - Version string management
 */

#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include "ui_format.h"

#ifndef VERSION
#define VERSION "unknown" // Fallback if not provided by build system
#endif

void ui_format_time(char *buf, size_t size, float seconds)
{
    if (!buf || size == 0)
        return;

    int total_seconds = (int)seconds;
    int hours = total_seconds / 3600;
    int minutes = (total_seconds % 3600) / 60;
    int secs = total_seconds % 60;

    if (hours > 0)
    {
        snprintf(buf, size, "%d:%02d:%02d", hours, minutes, secs);
    }
    else
    {
        snprintf(buf, size, "%d:%02d", minutes, secs);
    }
}

void ui_format_filename(char *buf, size_t size, const char *path, int max_width)
{
    if (!buf || size == 0 || !path)
    {
        if (buf && size > 0)
            buf[0] = '\0';
        return;
    }

    // Create a copy for basename since it may modify the string
    char path_copy[512];
    strncpy(path_copy, path, sizeof(path_copy) - 1);
    path_copy[sizeof(path_copy) - 1] = '\0';

    char *filename = basename(path_copy);
    size_t len = strlen(filename);

    if (max_width > 0 && (int)len > max_width)
    {
        // Truncate with ellipsis
        int copy_len = max_width - 3; // Leave room for "..."
        if (copy_len < 0)
            copy_len = 0;

        strncpy(buf, filename, copy_len);
        if (size > (size_t)copy_len)
        {
            strncpy(buf + copy_len, "...", size - copy_len - 1);
            buf[size - 1] = '\0';
        }
    }
    else
    {
        strncpy(buf, filename, size - 1);
        buf[size - 1] = '\0';
    }
}

void ui_format_progress_bar(char *buf, size_t size, float progress, int width)
{
    if (!buf || size == 0 || width <= 0)
    {
        if (buf && size > 0)
            buf[0] = '\0';
        return;
    }

    if (progress < 0.0f)
        progress = 0.0f;
    if (progress > 1.0f)
        progress = 1.0f;

    int filled = (int)(progress * width);
    int pos = 0;

    if (pos < (int)size - 1)
        buf[pos++] = '[';

    for (int i = 0; i < width && pos < (int)size - 1; i++)
    {
        buf[pos++] = (i < filled) ? '=' : '-';
    }

    if (pos < (int)size - 1)
        buf[pos++] = ']';

    buf[pos] = '\0';
}

const char *ui_get_version(void)
{
    return VERSION;
}
