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
#include <sys/stat.h>
#include "player.h"
#include "app_controller.h"
#include "ui_core.h"
#include "ui_screens.h"
#include "input.h"
#include "util.h"
#include "error.h"
#include "terminal.h"
#include "update.h"
#include "screen_state.h"

#define INPUT_POLL_INTERVAL_US 50000 // Poll for input every 50ms

static int path_is_directory(const char *path)
{
    struct stat st;

    if (!path)
        return 0;

    if (stat(path, &st) != 0)
        return 0;

    return S_ISDIR(st.st_mode) ? 1 : 0;
}

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

    AppController *controller = app_controller_create(player);
    if (!controller)
    {
        error_print(ERR_PLAYER_INIT, "Could not initialize controller");
        ui_buffer_destroy(ui_buf);
        player_destroy(player);
        return 1;
    }

    int running = 1;
    int show_controls = 0; // Controls hidden by default

    // Interactive mode
    ScreenState current_screen = SCREEN_WELCOME;

    // If path provided as argument, play file or load folder as playlist.
    if (argc > 1)
    {
        char filepath[512];
        strncpy(filepath, argv[1], sizeof(filepath) - 1);
        filepath[sizeof(filepath) - 1] = '\0';
        strip_quotes(filepath);
        unescape_path(filepath);

        ui_screen_loading(ui_buf, filepath);
        ui_buffer_render(ui_buf);

        if (path_is_directory(filepath))
        {
            int loaded = app_controller_load_playlist_folder(controller, filepath);
            if (loaded <= 0)
            {
                error_print(ERR_FILE_LOAD, "Could not load playable files from folder");
                app_controller_destroy(controller);
                ui_buffer_destroy(ui_buf);
                player_destroy(player);
                return 1;
            }

            current_screen = SCREEN_QUEUE;
            ui_screen_queue(ui_buf, app_controller_get_queue(controller),
                            app_controller_get_repeat_symbol(controller),
                            app_controller_get_repeat_label(controller));
        }
        else
        {
            if (app_controller_play_file_now(controller, filepath) != 0)
            {
                error_print(ERR_FILE_LOAD, filepath);
                app_controller_destroy(controller);
                ui_buffer_destroy(ui_buf);
                player_destroy(player);
                return 1;
            }

            current_screen = SCREEN_PLAYING;
            ui_screen_playing(ui_buf, player, show_controls,
                              app_controller_get_repeat_symbol(controller),
                              app_controller_get_repeat_label(controller));
        }
    }
    else
    {
        ui_screen_welcome(ui_buf, show_controls,
                          app_controller_get_repeat_symbol(controller),
                          app_controller_get_shuffle_symbol(controller),
                          app_controller_get_repeat_label(controller));
    }

    ui_buffer_render(ui_buf);

    terminal_raw_mode();

    while (running)
    {
        int ch = terminal_read_char();
        if (ch != -1)
        {
            // Handle input based on current screen
            if (current_screen == SCREEN_COLOR_PICKER)
            {
                int color_result = input_handle_color_selection(ch);
                if (color_result < 0)
                {
                    // Color was selected or cancelled, go back to settings
                    current_screen = SCREEN_SETTINGS;
                    ui_screen_settings(ui_buf);
                    ui_buffer_render(ui_buf);
                }
                // If color_result is 1, stay in color picker and wait for next input
            }
            else if (current_screen == SCREEN_SETTINGS)
            {
                if (ch == 'c' || ch == 'C')
                {
                    // Enter color picker
                    current_screen = SCREEN_COLOR_PICKER;
                    ui_screen_color_picker(ui_buf, NULL);
                    ui_buffer_render(ui_buf);
                }
                else if (ch == 'q' || ch == 'Q')
                {
                    // Exit settings, go back to main screen
                    current_screen = SCREEN_WELCOME;
                    if (player->is_playing)
                    {
                        current_screen = SCREEN_PLAYING;
                        ui_screen_playing(ui_buf, player, show_controls,
                                          app_controller_get_repeat_symbol(controller),
                                          app_controller_get_repeat_label(controller));
                    }
                    else
                    {
                        ui_screen_welcome(ui_buf, show_controls,
                                          app_controller_get_repeat_symbol(controller),
                                          app_controller_get_shuffle_symbol(controller),
                                          app_controller_get_repeat_label(controller));
                    }
                    ui_buffer_render(ui_buf);
                }
            }
            else
            {
                // Normal screen input handling
                InputAction action = input_map_key(ch);

                if (action == INPUT_ACTION_QUIT)
                {
                    if (current_screen == SCREEN_QUEUE)
                    {
                        // In queue view, q behaves like Back (same as settings)
                        current_screen = SCREEN_WELCOME;
                        if (player->is_playing)
                        {
                            current_screen = SCREEN_PLAYING;
                            ui_screen_playing(ui_buf, player, show_controls,
                                              app_controller_get_repeat_symbol(controller),
                                              app_controller_get_repeat_label(controller));
                        }
                        else
                        {
                            ui_screen_welcome(ui_buf, show_controls,
                                              app_controller_get_repeat_symbol(controller),
                                              app_controller_get_shuffle_symbol(controller),
                                              app_controller_get_repeat_label(controller));
                        }
                        ui_buffer_render(ui_buf);
                    }
                    else
                    {
                        running = 0;
                    }
                }
                else if (action == INPUT_ACTION_SHOW_QUEUE)
                {
                    current_screen = SCREEN_QUEUE;
                    ui_screen_queue(ui_buf, app_controller_get_queue(controller),
                                    app_controller_get_repeat_symbol(controller),
                                    app_controller_get_repeat_label(controller));
                    ui_buffer_render(ui_buf);
                }
                else if (action == INPUT_ACTION_SHOW_SETTINGS)
                {
                    current_screen = SCREEN_SETTINGS;
                    ui_screen_settings(ui_buf);
                    ui_buffer_render(ui_buf);
                }
                else if (action == INPUT_ACTION_PROMPT_FILE)
                {
                    char filepath[512];
                    int len = input_prompt_path(ui_buf, "Enter file path: ", filepath, sizeof(filepath));
                    if (len > 0)
                    {
                        ui_screen_loading(ui_buf, filepath);
                        ui_buffer_render(ui_buf);

                        if (path_is_directory(filepath))
                        {
                            int loaded = app_controller_load_playlist_folder(controller, filepath);
                            if (loaded > 0)
                            {
                                current_screen = SCREEN_QUEUE;
                                ui_screen_queue(ui_buf, app_controller_get_queue(controller),
                                                app_controller_get_repeat_symbol(controller),
                                                app_controller_get_repeat_label(controller));
                                ui_buffer_render(ui_buf);
                            }
                            else
                            {
                                error_print(ERR_FILE_LOAD, "Could not load playable files from folder");
                                ui_screen_welcome(ui_buf, show_controls,
                                                  app_controller_get_repeat_symbol(controller),
                                                  app_controller_get_shuffle_symbol(controller),
                                                  app_controller_get_repeat_label(controller));
                                ui_buffer_render(ui_buf);
                            }
                        }
                        else
                        {
                            if (app_controller_play_file_now(controller, filepath) == 0)
                            {
                                current_screen = SCREEN_PLAYING;
                                ui_screen_playing(ui_buf, player, show_controls,
                                                  app_controller_get_repeat_symbol(controller),
                                                  app_controller_get_repeat_label(controller));
                                ui_buffer_render(ui_buf);
                            }
                            else
                            {
                                error_print(ERR_FILE_LOAD, filepath);
                                ui_screen_welcome(ui_buf, show_controls,
                                                  app_controller_get_repeat_symbol(controller),
                                                  app_controller_get_shuffle_symbol(controller),
                                                  app_controller_get_repeat_label(controller));
                                ui_buffer_render(ui_buf);
                            }
                        }
                    }
                    else
                    {
                        if (player->is_playing)
                        {
                            ui_screen_playing(ui_buf, player, show_controls,
                                              app_controller_get_repeat_symbol(controller),
                                              app_controller_get_repeat_label(controller));
                        }
                        else
                        {
                            ui_screen_welcome(ui_buf, show_controls,
                                              app_controller_get_repeat_symbol(controller),
                                              app_controller_get_shuffle_symbol(controller),
                                              app_controller_get_repeat_label(controller));
                        }
                        ui_buffer_render(ui_buf);
                    }
                }
                else if (action == INPUT_ACTION_LOAD_PLAYLIST)
                {
                    char folderpath[512];
                    int len = input_prompt_path(ui_buf, "Enter folder path: ", folderpath, sizeof(folderpath));
                    if (len > 0)
                    {
                        ui_screen_loading(ui_buf, folderpath);
                        ui_buffer_render(ui_buf);

                        int loaded = app_controller_load_playlist_folder(controller, folderpath);
                        if (loaded > 0)
                        {
                            current_screen = SCREEN_QUEUE;
                            ui_screen_queue(ui_buf, app_controller_get_queue(controller),
                                            app_controller_get_repeat_symbol(controller),
                                            app_controller_get_repeat_label(controller));
                        }
                        else
                        {
                            error_print(ERR_FILE_LOAD, "Could not load playable files from folder");
                            current_screen = SCREEN_WELCOME;
                            ui_screen_welcome(ui_buf, show_controls,
                                              app_controller_get_repeat_symbol(controller),
                                              app_controller_get_shuffle_symbol(controller),
                                              app_controller_get_repeat_label(controller));
                        }
                        ui_buffer_render(ui_buf);
                    }
                    else
                    {
                        if (player->is_playing)
                        {
                            ui_screen_playing(ui_buf, player, show_controls,
                                              app_controller_get_repeat_symbol(controller),
                                              app_controller_get_repeat_label(controller));
                        }
                        else
                        {
                            ui_screen_welcome(ui_buf, show_controls,
                                              app_controller_get_repeat_symbol(controller),
                                              app_controller_get_shuffle_symbol(controller),
                                              app_controller_get_repeat_label(controller));
                        }
                        ui_buffer_render(ui_buf);
                    }
                }
                else if (action == INPUT_ACTION_ENQUEUE_FILE)
                {
                    char filepath[512];
                    int len = input_prompt_path(ui_buf, "Enter file path to add: ", filepath, sizeof(filepath));
                    if (len > 0)
                    {
                        if (app_controller_enqueue_file(controller, filepath) == 0)
                        {
                            if (current_screen == SCREEN_QUEUE)
                            {
                                ui_screen_queue(ui_buf, app_controller_get_queue(controller),
                                                app_controller_get_repeat_symbol(controller),
                                                app_controller_get_repeat_label(controller));
                            }
                            else
                            {
                                current_screen = SCREEN_PLAYING;
                                ui_screen_playing(ui_buf, player, show_controls,
                                                  app_controller_get_repeat_symbol(controller),
                                                  app_controller_get_repeat_label(controller));
                            }
                            ui_buffer_render(ui_buf);
                        }
                        else
                        {
                            error_print(ERR_FILE_LOAD, filepath);
                            if (player->is_playing)
                            {
                                ui_screen_playing(ui_buf, player, show_controls,
                                                  app_controller_get_repeat_symbol(controller),
                                                  app_controller_get_repeat_label(controller));
                            }
                            else
                            {
                                ui_screen_welcome(ui_buf, show_controls,
                                                  app_controller_get_repeat_symbol(controller),
                                                  app_controller_get_shuffle_symbol(controller),
                                                  app_controller_get_repeat_label(controller));
                            }
                            ui_buffer_render(ui_buf);
                        }
                    }
                }
                else if (action == INPUT_ACTION_NEXT_TRACK)
                {
                    app_controller_play_next(controller);

                    if (current_screen == SCREEN_QUEUE)
                    {
                        ui_screen_queue(ui_buf, app_controller_get_queue(controller),
                                        app_controller_get_repeat_symbol(controller),
                                        app_controller_get_repeat_label(controller));
                    }
                    else if (player->is_playing)
                    {
                        ui_screen_playing(ui_buf, player, show_controls,
                                          app_controller_get_repeat_symbol(controller),
                                          app_controller_get_repeat_label(controller));
                    }
                    else
                    {
                        ui_screen_welcome(ui_buf, show_controls,
                                          app_controller_get_repeat_symbol(controller),
                                          app_controller_get_shuffle_symbol(controller),
                                          app_controller_get_repeat_label(controller));
                    }
                    ui_buffer_render(ui_buf);
                }
                else if (action == INPUT_ACTION_PREVIOUS_TRACK)
                {
                    app_controller_play_previous(controller);

                    if (current_screen == SCREEN_QUEUE)
                    {
                        ui_screen_queue(ui_buf, app_controller_get_queue(controller),
                                        app_controller_get_repeat_symbol(controller),
                                        app_controller_get_repeat_label(controller));
                    }
                    else if (player->is_playing)
                    {
                        ui_screen_playing(ui_buf, player, show_controls,
                                          app_controller_get_repeat_symbol(controller),
                                          app_controller_get_repeat_label(controller));
                    }
                    else
                    {
                        ui_screen_welcome(ui_buf, show_controls,
                                          app_controller_get_repeat_symbol(controller),
                                          app_controller_get_shuffle_symbol(controller),
                                          app_controller_get_repeat_label(controller));
                    }
                    ui_buffer_render(ui_buf);
                }
                else if (action == INPUT_ACTION_TOGGLE_LOOP)
                {
                    app_controller_cycle_repeat(controller);

                    if (current_screen == SCREEN_QUEUE)
                    {
                        ui_screen_queue(ui_buf, app_controller_get_queue(controller),
                                        app_controller_get_repeat_symbol(controller),
                                        app_controller_get_repeat_label(controller));
                    }
                    else if (player->is_playing)
                    {
                        ui_screen_playing(ui_buf, player, show_controls,
                                          app_controller_get_repeat_symbol(controller),
                                          app_controller_get_repeat_label(controller));
                    }
                    else
                    {
                        ui_screen_welcome(ui_buf, show_controls,
                                          app_controller_get_repeat_symbol(controller),
                                          app_controller_get_shuffle_symbol(controller),
                                          app_controller_get_repeat_label(controller));
                    }
                    ui_buffer_render(ui_buf);
                }
                else if (action == INPUT_ACTION_TOGGLE_SHUFFLE)
                {
                    app_controller_toggle_shuffle(controller);

                    if (current_screen == SCREEN_QUEUE)
                    {
                        ui_screen_queue(ui_buf, app_controller_get_queue(controller),
                                        app_controller_get_repeat_symbol(controller),
                                        app_controller_get_repeat_label(controller));
                    }
                    else if (player->is_playing)
                    {
                        ui_screen_playing(ui_buf, player, show_controls,
                                          app_controller_get_repeat_symbol(controller),
                                          app_controller_get_repeat_label(controller));
                    }
                    else
                    {
                        ui_screen_welcome(ui_buf, show_controls,
                                          app_controller_get_repeat_symbol(controller),
                                          app_controller_get_shuffle_symbol(controller),
                                          app_controller_get_repeat_label(controller));
                    }
                    ui_buffer_render(ui_buf);
                }
                else
                {
                    // Let the normal action handler deal with it
                    running = input_handle_action(player, action, ui_buf, &show_controls);

                    // Keep current screen unless explicitly changed by action branches.
                    if (action != INPUT_ACTION_NONE)
                    {
                        if (current_screen == SCREEN_QUEUE)
                        {
                            ui_screen_queue(ui_buf, app_controller_get_queue(controller),
                                            app_controller_get_repeat_symbol(controller),
                                            app_controller_get_repeat_label(controller));
                        }
                        else if (current_screen == SCREEN_PLAYING)
                        {
                            ui_screen_playing(ui_buf, player, show_controls,
                                              app_controller_get_repeat_symbol(controller),
                                              app_controller_get_repeat_label(controller));
                        }
                        else if (current_screen == SCREEN_WELCOME)
                        {
                            ui_screen_welcome(ui_buf, show_controls,
                                              app_controller_get_repeat_symbol(controller),
                                              app_controller_get_shuffle_symbol(controller),
                                              app_controller_get_repeat_label(controller));
                        }
                        ui_buffer_render(ui_buf);
                    }
                }
            }
        }
        else
        {
            // Check if song has ended
            PlayerState state = player_get_state(player);
            if (state == STATE_PLAYING && player_has_finished(player))
            {
                app_controller_handle_track_end(controller);

                if (current_screen == SCREEN_QUEUE)
                {
                    ui_screen_queue(ui_buf, app_controller_get_queue(controller),
                                    app_controller_get_repeat_symbol(controller),
                                    app_controller_get_repeat_label(controller));
                    ui_buffer_render(ui_buf);
                }
                else if (current_screen == SCREEN_PLAYING)
                {
                    ui_screen_playing(ui_buf, player, show_controls,
                                      app_controller_get_repeat_symbol(controller),
                                      app_controller_get_repeat_label(controller));
                    ui_buffer_render(ui_buf);
                }
            }
            usleep(INPUT_POLL_INTERVAL_US);
        }
    }

    terminal_normal_mode();
    ui_clear_screen();
    printf("Exiting walcman...\n");

    ui_buffer_destroy(ui_buf);
    app_controller_destroy(controller);
    player_destroy(player);

    return 0;
}
