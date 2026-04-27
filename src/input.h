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
#include <stddef.h>

// Action codes returned by input handlers
typedef enum
{
    INPUT_ACTION_NONE = 0,        // No action needed (unrecognized key)
    INPUT_ACTION_QUIT,            // Exit program
    INPUT_ACTION_TOGGLE_PAUSE,    // Toggle play/pause state
    INPUT_ACTION_STOP,            // Stop playback completely
    INPUT_ACTION_PROMPT_FILE,     // Prompt user for file path to play
    INPUT_ACTION_LOAD_PLAYLIST,   // Prompt folder and load playlist
    INPUT_ACTION_ENQUEUE_FILE,    // Prompt file and append to queue
    INPUT_ACTION_SHOW_QUEUE,      // Show queue screen
    INPUT_ACTION_NEXT_TRACK,      // Skip to next track
    INPUT_ACTION_PREVIOUS_TRACK,  // Go to previous track
    INPUT_ACTION_TOGGLE_SHUFFLE,  // Toggle shuffle mode
    INPUT_ACTION_TOGGLE_CONTROLS, // Toggle controls visibility
    INPUT_ACTION_TOGGLE_LOOP,     // Toggle audio looping
    INPUT_ACTION_SHOW_SETTINGS,   // Open settings menu
    INPUT_ACTION_SELECT_COLOR,    // Enter color picker (submenu)
    INPUT_ACTION_BACK_TO_MAIN     // Return to main screen
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

/**
 * Prompt for path input using current UI style.
 * ui_buf: UI buffer for rendering prompt screen
 * prompt_text: Prompt text shown to user (e.g. "Enter file path: ")
 * out_path: Output buffer
 * out_size: Output buffer size
 * Returns: length of captured input, or 0 if empty
 */
int input_prompt_path(UIBuffer *ui_buf, const char *prompt_text, char *out_path, size_t out_size);

/**
 * Handle color selection input (submenu)
 * ch: Character code from terminal
 * Returns: -1 if color was selected or cancelled, 1 to continue in color picker
 */
int input_handle_color_selection(int ch);

#endif // WALCMAN_INPUT_H
