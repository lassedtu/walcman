#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "player.h"
#include "ui.h"
#include "input.h"
#include "util.h"
#include "error.h"
#include "terminal.h"

int main(int argc, char *argv[])
{
    Player *player = player_create();
    if (!player)
    {
        error_print(ERR_PLAYER_INIT, "Could not initialize audio engine");
        return 1;
    }

    // If file path provided as argument, play it immediately
    if (argc > 1)
    {
        const char *filepath = argv[1];
        strip_quotes((char *)filepath);
        ui_show_loading(filepath);
        if (player_play(player, filepath) == 0)
        {
            ui_show_status(player);
            terminal_raw_mode();

            while (1)
            {
                int ch = terminal_getchar();
                if (ch != -1)
                {
                    handle_single_key(player, ch);
                    if (ch != 'h' && ch != 'H' && ch != 'p' && ch != 'P')
                    {
                        fflush(stdout);
                    }
                }
                else
                {
                    usleep(50000);
                }
            }

            terminal_normal_mode();
        }
        else
        {
            error_print(ERR_FILE_LOAD, filepath);
            player_destroy(player);
            return 1;
        }

        player_destroy(player);
        return 0;
    }

    // Interactive mode
    ui_clear();
    print_help();

    terminal_raw_mode();

    while (1)
    {
        int ch = terminal_getchar();
        if (ch != -1)
        {
            handle_single_key(player, ch);
        }
        else
        {
            // Check if song has ended
            if (player->is_playing && !player->is_paused && player_is_at_end(player))
            {
                player_stop(player);
                ui_show_status(player);
            }
            usleep(50000);  // Sleep 50ms to avoid busy waiting
        }
    }

    terminal_normal_mode();
    player_destroy(player);

    return 0;
}
