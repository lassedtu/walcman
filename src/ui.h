#ifndef WALCMAN_UI_H
#define WALCMAN_UI_H

#include "player.h"

// Display help/usage information
void print_help(void);

// Clear terminal screen
void ui_clear(void);

// Display player status
void ui_show_status(Player *player);

// Display loading message
void ui_show_loading(const char *filepath);

#endif // WALCMAN_UI_H
