/**
 * input.h - Keyboard input handling and command dispatch
 *
 * Implements a command pattern for input handling:
 * 1. input_map_key() maps raw key presses to semantic actions
 * 2. input_handle_action() executes actions (modifies player state, renders UI)
 *
 * This architecture makes it easy to add new commands without modifying
 * the main event loop - just add an action enum and handle it.
 */

#ifndef WALCMAN_INPUT_H
#define WALCMAN_INPUT_H

#include "player.h"
#include "ui_core.h"

// Action codes returned by input handlers
typedef enum
{
    INPUT_ACTION_NONE = 0,        // No action needed (unrecognized key)
    INPUT_ACTION_QUIT,            // Exit program
    INPUT_ACTION_TOGGLE_PAUSE,    // Toggle play/pause state
    INPUT_ACTION_STOP,            // Stop playback completely
    INPUT_ACTION_SHOW_HELP,       // Display help screen
    INPUT_ACTION_PROMPT_FILE,     // Prompt user for file path to play
    INPUT_ACTION_TOGGLE_CONTROLS, // Toggle controls visibility
    INPUT_ACTION_TOGGLE_LOOP      // Toggle audio looping
} InputAction;

/**
 * Map a key press to an action (pure function)
 * ch: Character code from terminal_read_char()
 * Returns: InputAction enum value
 */
InputAction input_map_key(int ch);

/**
 * Execute an action (may modify player state and render to buffer)
 * player: Player instance to control
 * action: Action to execute
 * ui_buf: UI buffer for rendering
 * show_controls: Pointer to controls visibility flag (can be modified)
 * Returns: 1 if program should continue, 0 if should exit
 */
int input_handle_action(Player *player, InputAction action, UIBuffer *ui_buf, int *show_controls);

#endif // WALCMAN_INPUT_H
