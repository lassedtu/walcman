/**
 * ui_components.c - Reusable UI widgets implementation
 *
 * Library of composable UI components for building screens.
 * Each component appends formatted output to the provided UIBuffer.
 *
 * Components are designed to be combined to create complete screens
 * without needing to know about each other.
 */

#include <stdio.h>
#include <string.h>
#include "ui_components.h"
#include "ui_format.h"

#define DEFAULT_WIDTH 50 // Default width for separators and bars

void ui_component_header(UIBuffer *buf)
{
    if (!buf)
        return;

    ui_buffer_appendf(buf, "walcman v%s\n", ui_get_version());
}

void ui_component_separator(UIBuffer *buf, int width)
{
    if (!buf)
        return;

    if (width <= 0)
        width = DEFAULT_WIDTH;

    for (int i = 0; i < width; i++)
    {
        ui_buffer_append_char(buf, '-');
    }
    ui_buffer_append_char(buf, '\n');
}

void ui_component_footer(UIBuffer *buf)
{
    if (!buf)
        return;

    ui_component_separator(buf, DEFAULT_WIDTH);
    ui_buffer_append_char(buf, '\n');
}

void ui_component_status_line(UIBuffer *buf, const char *icon, const char *text)
{
    if (!buf)
        return;

    if (icon && text)
    {
        ui_buffer_appendf(buf, "%s  %s\n", icon, text);
    }
    else if (text)
    {
        ui_buffer_appendf(buf, "%s\n", text);
    }
}

void ui_component_file_info(UIBuffer *buf, const char *filename)
{
    if (!buf)
        return;

    if (filename)
    {
        char formatted_name[64];
        ui_format_filename(formatted_name, sizeof(formatted_name), filename, 45);
        ui_buffer_appendf(buf, "File: %s\n", formatted_name);
    }
    else
    {
        ui_buffer_append(buf, "File: Unknown\n");
    }
}

void ui_component_key_hint(UIBuffer *buf, const char *key, const char *description)
{
    if (!buf || !key || !description)
        return;

    ui_buffer_appendf(buf, "  %-10s - %s\n", key, description);
}

void ui_component_key_hints_section(UIBuffer *buf, const char *title)
{
    if (!buf)
        return;

    if (title)
    {
        ui_buffer_appendf(buf, "\n%s:\n", title);
    }
    else
    {
        ui_buffer_append(buf, "\n");
    }
}

void ui_component_progress_bar(UIBuffer *buf, float progress, int width)
{
    if (!buf)
        return;

    if (width <= 0)
        width = 40;

    char bar[128];
    ui_format_progress_bar(bar, sizeof(bar), progress, width);
    ui_buffer_appendf(buf, "%s ", bar);
}

void ui_component_time_display(UIBuffer *buf, float current, float total)
{
    if (!buf)
        return;

    char current_str[16];
    char total_str[16];

    ui_format_time(current_str, sizeof(current_str), current);
    ui_format_time(total_str, sizeof(total_str), total);

    ui_buffer_appendf(buf, "%s / %s\n", current_str, total_str);
}

void ui_component_message(UIBuffer *buf, const char *message)
{
    if (!buf)
        return;

    if (message)
    {
        ui_buffer_appendf(buf, "%s\n", message);
    }
}

void ui_component_loading(UIBuffer *buf, const char *filepath)
{
    if (!buf)
        return;

    ui_buffer_append(buf, "Loading: ");
    if (filepath)
    {
        char formatted_name[64];
        ui_format_filename(formatted_name, sizeof(formatted_name), filepath, 35);
        ui_buffer_appendf(buf, "%s", formatted_name);
    }
    ui_buffer_append(buf, "\n\n");
}
