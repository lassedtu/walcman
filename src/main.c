/**
 * main.c - Application entry point and main event loop
 *
 * This is the core orchestrator for walcman. It handles:
 * - Initialization of player and UI systems
 * - Command-line argument processing (direct file playback)
 * - Interactive mode with welcome screen
 * - Main event loop with input polling and song end detection
 * - Clean shutdown and resource cleanup
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "player.h"
#include "ui_core.h"
#include "ui_screens.h"
#include "input.h"
#include "util.h"
#include "error.h"
#include "terminal.h"
#include "update.h"

#define INPUT_POLL_INTERVAL_US 50000 // Poll for input every 50ms

/**
 * Application entry point
 *
 * Supports two modes:
 * 1. Direct playback: walcman <filepath> - plays file immediately
 * 2. Interactive: walcman - shows welcome screen, wait for commands
 */
int main(int argc, char *argv[])
{
    // Check for updates in background (silent, non-blocking)
    update_check_background();

    Player *player = player_create();
    if (!player)
    {
        error_print(ERR_PLAYER_INIT, "Could not initialize audio engine");
        return 1;
    }

    UIBuffer *ui_buf = ui_buffer_create();
    if (!ui_buf)
    {
        error_print(ERR_PLAYER_INIT, "Could not initialize UI");
        player_destroy(player);
        return 1;
    }

    int running = 1;
    int show_controls = 0; // Controls hidden by default

    // If file path provided as argument, play it immediately
    if (argc > 1)
    {
        const char *filepath = argv[1];
        strip_quotes((char *)filepath);

        ui_screen_loading(ui_buf, filepath);
        ui_buffer_render(ui_buf);

        if (player_play(player, filepath) == 0)
        {
            ui_screen_playing(ui_buf, player, show_controls);
            ui_buffer_render(ui_buf);

            terminal_raw_mode();

            while (running)
            {
                int ch = terminal_read_char();
                if (ch != -1)
                {
                    InputAction action = input_map_key(ch);
                    running = input_handle_action(player, action, ui_buf, &show_controls);
                }
                else
                {
                    usleep(INPUT_POLL_INTERVAL_US);
                }
            }

            terminal_normal_mode();
        }
        else
        {
            error_print(ERR_FILE_LOAD, filepath);
            ui_buffer_destroy(ui_buf);
            player_destroy(player);
            return 1;
        }

        ui_clear_screen();
        printf("Exiting walcman...\n");
        ui_buffer_destroy(ui_buf);
        player_destroy(player);
        return 0;
    }

    // Interactive mode
    ui_screen_welcome(ui_buf, show_controls);
    ui_buffer_render(ui_buf);

    terminal_raw_mode();

    while (running)
    {
        int ch = terminal_read_char();
        if (ch != -1)
        {
            InputAction action = input_map_key(ch);
            running = input_handle_action(player, action, ui_buf, &show_controls);
        }
        else
        {
            // Check if song has ended
            PlayerState state = player_get_state(player);
            if (state == STATE_PLAYING && player_has_finished(player))
            {
                player_stop(player);
                ui_screen_playing(ui_buf, player, show_controls);
                ui_buffer_render(ui_buf);
            }
            usleep(INPUT_POLL_INTERVAL_US);
        }
    }

    terminal_normal_mode();
    ui_clear_screen();
    printf("Exiting walcman...\n");

    ui_buffer_destroy(ui_buf);
    player_destroy(player);

    return 0;
}
