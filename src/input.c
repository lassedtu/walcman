#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "input.h"
#include "player.h"
#include "util.h"
#include "ui.h"
#include "error.h"
#include "terminal.h"

void handle_input(Player *player, const char *command)
{
    if (!command || strlen(command) == 0)
        return;

    char cmd = command[0];

    switch (cmd)
    {
    case 'p':
    {
        // Extract filepath from command "p <filepath>"
        if (strlen(command) > 2)
        {
            const char *filepath = command + 2;
            printf("Loading: %s\n", filepath);
            if (player_play(player, filepath) == 0)
            {
                printf("Now playing: %s\n", filepath);
            }
            else
            {
                error_print(ERR_FILE_LOAD, filepath);
            }
        }
        else
        {
            printf("Usage: p <filepath>\n");
        }
        break;
    }
    case ' ':
        if (player->is_playing)
        {
            if (player->is_paused)
            {
                player_resume(player);
                printf("▶ Resumed\n");
            }
            else
            {
                player_pause(player);
                printf("⏸ Paused\n");
            }
        }
        else
        {
            printf("No file playing\n");
        }
        break;
    case 's':
        if (player->is_playing)
        {
            player_stop(player);
            printf("⏹ Stopped\n");
        }
        else
        {
            printf("No file playing\n");
        }
        break;
    case 'q':
        printf("Exiting...\n");
        exit(0);
    case 'h':
        print_help();
        break;
    default:
        printf("Unknown command. Type 'h' for help.\n");
    }
}

void handle_single_key(Player *player, int key)
{
    switch (key)
    {
    case ' ':
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
            ui_show_status(player);
        }
        break;
    case 's':
    case 'S':
        if (player->is_playing)
        {
            player_stop(player);
            ui_show_status(player);
        }
        break;
    case 'p':
    case 'P':
        enter_file_input_mode(player);
        break;
    case 'q':
    case 'Q':
        ui_clear();
        printf("Exiting walcman...\n");
        exit(0);
    case 'h':
    case 'H':
        ui_clear();
        print_help();
        break;
    }
}

void enter_file_input_mode(Player *player)
{
    ui_clear();
    printf("walcman v1.0.0\n");
    printf("-------------------------------\n\n");
    printf("Enter file path: ");
    fflush(stdout);

    char filepath[512];
    int len = terminal_readline(filepath, sizeof(filepath));

    if (len > 0)
    {
        strip_quotes(filepath);
        ui_show_loading(filepath);
        if (player_play(player, filepath) == 0)
        {
            ui_show_status(player);
        }
        else
        {
            error_print(ERR_FILE_LOAD, filepath);
            ui_show_status(player);
        }
    }
    else
    {
        ui_show_status(player);
    }
}
