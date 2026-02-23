/**
 * ui_format.c - Text formatting utilities implementation
 *
 * Provides formatting functions for UI display:
 * - Time conversion to human-readable format
 * - Filename extraction from paths
 * - Progress bar visual generation
 * - Version string management
 * - UI color configuration management
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
const char *color_name_to_ansi(const char *color_name)
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
        {"pink", "\033[1;35m"},
        {"magenta", "\033[1;35m"}, // Alias for pink
        {"purple", "\033[1;35m"},  // Alias for pink
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

/**
 * Write UI color setting to config file
 */
int ui_write_color_config(const char *color_name)
{
    const char *home = getenv("HOME");
    if (!home)
        return -1;

    char config_path[512];
    snprintf(config_path, sizeof(config_path), "%s%s", home, CONFIG_FILE);

    // Read existing config
    char *config_content = NULL;
    size_t config_size = 0;
    FILE *f = fopen(config_path, "r");
    if (f)
    {
        // Get file size
        fseek(f, 0, SEEK_END);
        config_size = ftell(f);
        fseek(f, 0, SEEK_SET);

        config_content = (char *)malloc(config_size + 256); // Extra space for new line
        if (!config_content)
        {
            fclose(f);
            return -1;
        }

        fread(config_content, 1, config_size, f);
        fclose(f);
    }
    else
    {
        config_content = (char *)malloc(256);
        if (!config_content)
            return -1;
        config_content[0] = '\0';
        config_size = 0;
    }

    // Write config back, updating or adding ui_color line
    FILE *out = fopen(config_path, "w");
    if (!out)
    {
        free(config_content);
        return -1;
    }

    int found_ui_color = 0;
    char line[256];
    const char *ptr = config_content;

    // Parse and update existing content
    while (ptr && ptr - config_content < (int)config_size)
    {
        const char *newline = strchr(ptr, '\n');
        size_t line_len = newline ? (newline - ptr) : strlen(ptr);

        if (line_len < sizeof(line) - 1)
        {
            strncpy(line, ptr, line_len);
            line[line_len] = '\0';

            // Check if this is the ui_color line
            if (strncmp(line, "ui_color=", 9) == 0)
            {
                found_ui_color = 1;
                if (color_name && color_name[0] != '\0')
                {
                    fprintf(out, "ui_color=%s\n", color_name);
                }
                else
                {
                    fprintf(out, "ui_color=\n");
                }
            }
            else
            {
                fprintf(out, "%s\n", line);
            }
        }

        if (!newline)
            break;
        ptr = newline + 1;
    }

    // If ui_color wasn't found, add it
    if (!found_ui_color)
    {
        if (color_name && color_name[0] != '\0')
        {
            fprintf(out, "ui_color=%s\n", color_name);
        }
        else
        {
            fprintf(out, "ui_color=\n");
        }
    }

    fclose(out);
    free(config_content);

    // Reset cache so it reloads on next call
    ui_color_loaded = 0;
    ui_color_cache[0] = '\0';

    return 0;
}

/**
 * Format text with a specific color
 */
void ui_format_with_color(char *buf, size_t size, const char *text, const char *color_name)
{
    if (!buf || size == 0 || !text)
        return;

    const char *ansi_color = color_name_to_ansi(color_name);
    const char *global_color = ui_get_color(); // Get global color to reset to

    if (ansi_color && ansi_color[0] != '\0')
    {
        if (global_color && global_color[0] != '\0')
        {
            // Reset back to global color after displaying the temp color
            snprintf(buf, size, "%s%s%s", ansi_color, text, global_color);
        }
        else
        {
            // No global color, reset to default
            snprintf(buf, size, "%s%s\033[0m", ansi_color, text);
        }
    }
    else
    {
        // No valid color, just copy text as-is
        strncpy(buf, text, size - 1);
        buf[size - 1] = '\0';
    }
}
