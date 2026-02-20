#include <stdio.h>
#include <stdlib.h>
#include "ui.h"
#include "player.h"

#ifndef VERSION
#define VERSION "unknown"
#endif

void ui_clear(void)
{
    printf("\033[2J\033[H");
    fflush(stdout);
}

void print_help(void)
{
    printf("walcman v" VERSION " - Terminal Music Player\n\n");
    printf("Commands:\n");
    printf("  p         - Play file\n");
    printf("  [space]   - Play/Pause\n");
    printf("  s         - Stop\n");
    printf("  h         - Show this help\n");
    printf("  q         - Quit\n");
    printf("-------------------------------\n\n");
    fflush(stdout);
}

void ui_show_status(Player *player)
{
    ui_clear();
    printf("walcman v" VERSION "\n");
    printf("-------------------------------\n\n");

    if (player->is_playing)
    {
        if (player->is_paused)
            printf("⏸  PAUSED\n");
        else
            printf("▶  PLAYING\n");

        printf("\nFile: %s\n", player->current_file ? player->current_file : "Unknown");
        printf("\nControls:\n");
        printf("  [space] - Resume/Pause\n");
        printf("  [s]     - Stop\n");
        printf("  [q]     - Quit\n");
    }
    else
    {
        printf("Ready to play\n\n");
        printf("Commands:\n");
        printf("  [p]     - Play file\n");
        printf("  [h]     - Help\n");
        printf("  [q]     - Quit\n");
    }

    printf("\n-------------------------------\n\n");
    fflush(stdout);
}

void ui_show_loading(const char *filepath)
{
    ui_clear();
    printf("walcman v" VERSION "\n");
    printf("-------------------------------\n\n");
    printf("Loading: %s\n\n", filepath);
    fflush(stdout);
}
