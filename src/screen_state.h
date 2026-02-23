/**
 * screen_state.h - Screen navigation state management
 */

#ifndef WALCMAN_SCREEN_STATE_H
#define WALCMAN_SCREEN_STATE_H

// Screen states for navigation
typedef enum
{
    SCREEN_WELCOME,     // Initial/main screen
    SCREEN_PLAYING,     // Now playing screen
    SCREEN_HELP,        // Help screen
    SCREEN_SETTINGS,    // Settings menu
    SCREEN_COLOR_PICKER // Color picker submenu
} ScreenState;

#endif // WALCMAN_SCREEN_STATE_H
