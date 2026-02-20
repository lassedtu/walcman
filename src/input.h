#ifndef WALCMAN_INPUT_H
#define WALCMAN_INPUT_H

#include "player.h"

// Handle user input and execute commands
void handle_input(Player *player, const char *command);

// Handle single key input (for raw terminal mode)
void handle_single_key(Player *player, int key);

// Enter line-reading mode (for file paths)
void enter_file_input_mode(Player *player);

#endif // WALCMAN_INPUT_H
