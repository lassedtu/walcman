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
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <libgen.h>
#include "ui_format.h"

#ifndef VERSION
#define VERSION "unknown" // Fallback if not provided by build system
#endif

#define CONFIG_FILE "/.config/walcman/config"

// Cache for UI color
static char ui_color_cache[32] = "";
static int ui_color_loaded = 0;

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

/**
 * Convert color name to ANSI escape code
 */
static const char *color_name_to_ansi(const char *color_name)
{
    // Color mapping table
    typedef struct
    {
        const char *name;
        const char *ansi;
    } ColorMap;

    static const ColorMap colors[] = {
        {"red", "\033[1;31m"},
        {"green", "\033[1;32m"},
        {"yellow", "\033[1;33m"},
        {"blue", "\033[1;34m"},
        {"magenta", "\033[1;35m"},
        {"purple", "\033[1;35m"}, // Alias for magenta
        {"pink", "\033[1;35m"},   // Alias for magenta
        {"cyan", "\033[1;36m"},
        {"white", "\033[1;37m"},
        {"gray", "\033[0;90m"},
        {"grey", "\033[0;90m"},   // Alternative spelling
        {"orange", "\033[1;33m"}, // Use yellow for orange
        {NULL, NULL}};

    // Case-insensitive search
    for (int i = 0; colors[i].name != NULL; i++)
    {
        if (strcasecmp(color_name, colors[i].name) == 0)
        {
            return colors[i].ansi;
        }
    }

    return ""; // Unknown color, return empty string
}

const char *ui_get_color(void)
{
    // Load color from config on first call
    if (!ui_color_loaded)
    {
        ui_color_loaded = 1;
        const char *home = getenv("HOME");
        if (!home)
            return "";

        char config_path[512];
        snprintf(config_path, sizeof(config_path), "%s%s", home, CONFIG_FILE);

        FILE *f = fopen(config_path, "r");
        if (!f)
            return "";

        char line[256];
        char color_name[64] = "";

        while (fgets(line, sizeof(line), f))
        {
            // Skip comments and empty lines
            if (line[0] == '#' || line[0] == '\n')
                continue;

            // Check for ui_color setting
            if (strncmp(line, "ui_color=", 9) == 0)
            {
                const char *value = line + 9;
                // Remove trailing newline and whitespace
                size_t len = strcspn(value, "\n\r \t");
                if (len > 0 && len < sizeof(color_name))
                {
                    strncpy(color_name, value, len);
                    color_name[len] = '\0';

                    // Convert color name to ANSI code
                    const char *ansi = color_name_to_ansi(color_name);
                    if (ansi && ansi[0] != '\0')
                    {
                        strncpy(ui_color_cache, ansi, sizeof(ui_color_cache) - 1);
                        ui_color_cache[sizeof(ui_color_cache) - 1] = '\0';
                    }
                }
                break;
            }
        }

        fclose(f);
    }

    return ui_color_cache;
}
