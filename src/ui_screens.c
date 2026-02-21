/**
 * ui_screens.c - Complete screen layouts implementation
 *
 * High-level screen builders that compose UI components.
 * Each screen builder clears the buffer and constructs a complete
 * screen from components, ready to render.
 *
 * Screens are stateless - they query the player for current state
 * and build the appropriate display.
 */

#include <stdio.h>
#include "ui_screens.h"
#include "ui_components.h"
#include "ui_format.h"
#include "player.h"

// ===== Command definitions =====

typedef struct
{
    const char *key;
    const char *description;
} KeyHint;

// Common commands shown in all contexts
static const KeyHint common_commands[] = {
    {"[h]", "Help"},
    {"[c]", "Toggle controls"},
    {"[q]", "Quit"},
};

#define COMMON_COMMANDS_COUNT (sizeof(common_commands) / sizeof(common_commands[0]))

// ===== Screen template helpers =====

/**
 * Start a standard screen (header + separator)
 * buf: Buffer to build into
 */
static void screen_begin(UIBuffer *buf)
{
    ui_buffer_clear(buf);
    ui_component_header(buf);
    ui_component_separator(buf, 50);
    ui_buffer_append(buf, "\n");
}

/**
 * End a standard screen (footer)
 * buf: Buffer to build into
 */
static void screen_end(UIBuffer *buf)
{
    ui_buffer_append(buf, "\n");
    ui_component_footer(buf);
}

/**
 * Display hints section with context-aware play command
 * buf: Buffer to append to
 * title: Section title
 * show_play_file: If true, show "[p] Play file"; if false, show "[space] Play/Pause"
 * show_controls: Whether to show controls or prompt
 */
static void display_commands(UIBuffer *buf, const char *title, int show_play_file, int show_controls)
{
    if (!show_controls)
    {
        ui_component_message(buf, "Press [c] to show controls");
        return;
    }

    ui_component_key_hints_section(buf, title);

    // Context-aware play command
    if (show_play_file)
    {
        ui_component_key_hint(buf, "[p]", "Play file");
    }
    else
    {
        ui_component_key_hint(buf, "[space]", "Play/Pause");
    }

    // Common commands
    for (size_t i = 0; i < COMMON_COMMANDS_COUNT; i++)
    {
        ui_component_key_hint(buf, common_commands[i].key, common_commands[i].description);
    }
}

void ui_screen_welcome(UIBuffer *buf, int show_controls)
{
    if (!buf)
        return;

    screen_begin(buf);
    ui_component_message(buf, "Ready to play");
    ui_buffer_append(buf, "\n");
    display_commands(buf, "Commands", 1, show_controls);
    screen_end(buf);
}

void ui_screen_help(UIBuffer *buf)
{
    if (!buf)
        return;

    screen_begin(buf);
    ui_buffer_append(buf, " - Terminal Music Player\n\n");

    ui_component_key_hints_section(buf, "Commands");
    ui_component_key_hint(buf, "[p]", "Play file");
    ui_component_key_hint(buf, "[space]", "Play/Pause");
    ui_component_key_hint(buf, "[s]", "Stop");
    ui_component_key_hint(buf, "[r]", "Toggle repeat");
    for (size_t i = 0; i < COMMON_COMMANDS_COUNT; i++)
    {
        ui_component_key_hint(buf, common_commands[i].key, common_commands[i].description);
    }

    screen_end(buf);
}

void ui_screen_playing(UIBuffer *buf, Player *player, int show_controls)
{
    if (!buf || !player)
        return;

    screen_begin(buf);

    PlayerState state = player_get_state(player);

    if (state == STATE_PLAYING || state == STATE_PAUSED)
    {
        // Show playback status
        if (state == STATE_PAUSED)
        {
            ui_component_status_line(buf, "⏸", "PAUSED");
        }
        else
        {
            ui_component_status_line(buf, "▶", "PLAYING");
        }

        ui_buffer_append(buf, "\n");

        // Show currently playing file with loop status indicator
        const char *current_file = player_get_current_file(player);
        const char *loop_indicator = player_get_loop(player) ? " ↺" : " ⇾";

        char formatted_filename[64];
        ui_format_filename(formatted_filename, sizeof(formatted_filename), current_file, 40);

        ui_buffer_append(buf, "File: ");
        ui_buffer_append(buf, formatted_filename);
        ui_buffer_append(buf, loop_indicator);
        ui_buffer_append(buf, "\n");
        ui_buffer_append(buf, "\n");

        // Show controls (context: already playing, so show stop + pause)
        if (show_controls)
        {
            ui_component_key_hints_section(buf, "Controls");
            ui_component_key_hint(buf, "[space]", "Play/Pause");
            ui_component_key_hint(buf, "[s]", "Stop");
            ui_component_key_hint(buf, "[r]", "Toggle repeat");
            for (size_t i = 0; i < COMMON_COMMANDS_COUNT; i++)
            {
                ui_component_key_hint(buf, common_commands[i].key, common_commands[i].description);
            }
        }
        else
        {
            ui_component_message(buf, "Press [c] to show controls");
        }
    }
    else
    {
        // Not playing - show welcome screen
        ui_component_message(buf, "Ready to play");
        ui_buffer_append(buf, "\n");
        display_commands(buf, "Commands", 1, show_controls);
    }

    screen_end(buf);
}

void ui_screen_loading(UIBuffer *buf, const char *filepath)
{
    if (!buf)
        return;

    screen_begin(buf);
    ui_component_loading(buf, filepath);
}
