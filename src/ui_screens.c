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

void ui_screen_welcome(UIBuffer *buf)
{
    if (!buf)
        return;

    ui_buffer_clear(buf);
    ui_component_header(buf);
    ui_component_separator(buf, 50);
    ui_buffer_append(buf, "\n");

    ui_component_message(buf, "Ready to play");
    ui_buffer_append(buf, "\n");

    ui_component_key_hints_section(buf, "Commands");
    ui_component_key_hint(buf, "[p]", "Play file");
    ui_component_key_hint(buf, "[h]", "Help");
    ui_component_key_hint(buf, "[q]", "Quit");

    ui_buffer_append(buf, "\n");
    ui_component_footer(buf);
}

void ui_screen_help(UIBuffer *buf)
{
    if (!buf)
        return;

    ui_buffer_clear(buf);
    ui_component_header(buf);
    ui_buffer_append(buf, " - Terminal Music Player\n\n");

    ui_component_key_hints_section(buf, "Commands");
    ui_component_key_hint(buf, "p", "Play file");
    ui_component_key_hint(buf, "[space]", "Play/Pause");
    ui_component_key_hint(buf, "s", "Stop");
    ui_component_key_hint(buf, "h", "Show this help");
    ui_component_key_hint(buf, "q", "Quit");

    ui_component_separator(buf, 50);
    ui_buffer_append(buf, "\n");
}

void ui_screen_playing(UIBuffer *buf, Player *player)
{
    if (!buf || !player)
        return;

    ui_buffer_clear(buf);
    ui_component_header(buf);
    ui_component_separator(buf, 50);
    ui_buffer_append(buf, "\n");

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

        // TODO: Show file info (disabled for now)
        // const char *current_file = player_get_current_file(player);
        // ui_component_file_info(buf, current_file);
        // ui_buffer_append(buf, "\n");

        // TODO: Show progress bar and time (disabled for now)
        // float position = player_get_position(player);
        // float duration = player_get_duration(player);
        // if (duration > 0.0f)
        // {
        //     ui_component_progress_bar(buf, position / duration, 40);
        //     ui_component_time_display(buf, position, duration);
        // }
        // ui_buffer_append(buf, "\n");

        // Show controls
        ui_component_key_hints_section(buf, "Controls");
        ui_component_key_hint(buf, "[space]", "Resume/Pause");
        ui_component_key_hint(buf, "[s]", "Stop");
        ui_component_key_hint(buf, "[q]", "Quit");
    }
    else
    {
        // Not playing - show welcome screen
        ui_component_message(buf, "Ready to play");
        ui_buffer_append(buf, "\n");

        ui_component_key_hints_section(buf, "Commands");
        ui_component_key_hint(buf, "[p]", "Play file");
        ui_component_key_hint(buf, "[h]", "Help");
        ui_component_key_hint(buf, "[q]", "Quit");
    }

    ui_buffer_append(buf, "\n");
    ui_component_footer(buf);
}

void ui_screen_loading(UIBuffer *buf, const char *filepath)
{
    if (!buf)
        return;

    ui_buffer_clear(buf);
    ui_component_header(buf);
    ui_component_separator(buf, 50);
    ui_buffer_append(buf, "\n");

    ui_component_loading(buf, filepath);
}
