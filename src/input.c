/**
 * input.c - Keyboard input handling and command dispatch
 *
 * Implements the command pattern for input handling:
 * - input_map_key: Maps character codes to semantic actions (pure function)
 * - input_handle_action: Executes actions by modifying player state and UI
 * - prompt_for_file: Helper for file path input (internal)
 *
 * This design separates key mapping from action execution, making it easy
 * to add new commands or change key bindings.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "input.h"
#include "player.h"
#include "util.h"
#include "ui_core.h"
#include "ui_components.h"
#include "ui_screens.h"
#include "ui_format.h"
#include "error.h"
#include "terminal.h"

InputAction input_map_key(int ch)
{
    switch (ch)
    {
    case ' ':
        return INPUT_ACTION_TOGGLE_PAUSE;
    case 's':
    case 'S':
        return INPUT_ACTION_STOP;
    case 'p':
    case 'P':
        return INPUT_ACTION_PROMPT_FILE;
    case 'h':
    case 'H':
        return INPUT_ACTION_SHOW_HELP;
    case 'c':
    case 'C':
        return INPUT_ACTION_TOGGLE_CONTROLS;
    case 'r':
    case 'R':
        return INPUT_ACTION_TOGGLE_LOOP;
    case 'o':
    case 'O':
        return INPUT_ACTION_SHOW_SETTINGS;
    case 'q':
    case 'Q':
        return INPUT_ACTION_QUIT;
    default:
        return INPUT_ACTION_NONE;
    }
}

static void prompt_for_file(Player *player, UIBuffer *ui_buf)
{
    if (!ui_buf)
        return;

    ui_buffer_clear(ui_buf);
    ui_component_header(ui_buf);
    ui_component_separator(ui_buf, 50);
    ui_buffer_append(ui_buf, "\n");
    ui_buffer_append(ui_buf, "Enter file path: ");
    ui_buffer_render(ui_buf);

    char filepath[512];
    int len = terminal_read_line(filepath, sizeof(filepath));

    if (len > 0)
    {
        strip_quotes(filepath);
        unescape_path(filepath);
        ui_screen_loading(ui_buf, filepath);
        ui_buffer_render(ui_buf);

        if (player_play(player, filepath) != 0)
        {
            error_print(ERR_FILE_LOAD, filepath);
        }
    }
}

int input_handle_action(Player *player, InputAction action, UIBuffer *ui_buf, int *show_controls)
{
    switch (action)
    {
    case INPUT_ACTION_NONE:
        return 1;

    case INPUT_ACTION_QUIT:
        return 0;

    case INPUT_ACTION_TOGGLE_PAUSE:
        if (player->is_playing)
        {
            if (player->is_paused)
            {
                player_resume(player);
            }
            else
            {
                player_pause(player);
            }
            ui_screen_playing(ui_buf, player, *show_controls);
            ui_buffer_render(ui_buf);
        }
        return 1;

    case INPUT_ACTION_STOP:
        if (player->is_playing)
        {
            player_stop(player);
            ui_screen_playing(ui_buf, player, *show_controls);
            ui_buffer_render(ui_buf);
        }
        return 1;

    case INPUT_ACTION_SHOW_HELP:
        ui_screen_help(ui_buf);
        ui_buffer_render(ui_buf);
        return 1;

    case INPUT_ACTION_PROMPT_FILE:
        prompt_for_file(player, ui_buf);
        ui_screen_playing(ui_buf, player, *show_controls);
        ui_buffer_render(ui_buf);
        return 1;

    case INPUT_ACTION_TOGGLE_CONTROLS:
        if (show_controls)
        {
            *show_controls = !(*show_controls);
            ui_screen_playing(ui_buf, player, *show_controls);
            ui_buffer_render(ui_buf);
        }
        return 1;

    case INPUT_ACTION_TOGGLE_LOOP:
        if (player->is_playing)
        {
            player_toggle_loop(player);
            ui_screen_playing(ui_buf, player, *show_controls);
            ui_buffer_render(ui_buf);
        }
        return 1;

    case INPUT_ACTION_SHOW_SETTINGS:
        ui_screen_settings(ui_buf);
        ui_buffer_render(ui_buf);
        return 1;

    case INPUT_ACTION_SELECT_COLOR:
    case INPUT_ACTION_BACK_TO_MAIN:
        // These are handled by main.c with its screen state
        return 1;

    default:
        return 1;
    }
}

/**
 * Color selection array mapping numbers to color names
 */
static const char *color_options[] = {
    "red",
    "green",
    "yellow",
    "blue",
    "pink",
    "cyan",
    "white",
    "gray",
    NULL // Sentinel
};

/**
 * Handle color selection input (0-8 for colors, q to cancel)
 * Returns: -1 if color was selected or cancelled, 1 to continue
 */
int input_handle_color_selection(int ch)
{
    if (ch >= '0' && ch <= '8')
    {
        int index = ch - '1';
        if (ch == '0')
        {
            // Default color (empty)
            ui_write_color_config("");
            return -1; // Signal to exit color picker
        }
        else if (index >= 0 && index < 8 && color_options[index] != NULL)
        {
            ui_write_color_config(color_options[index]);
            return -1; // Signal to exit color picker
        }
    }
    else if (ch == 'q' || ch == 'Q')
    {
        return -1; // Cancel color picker
    }
    return 1; // Continue in color picker
}
