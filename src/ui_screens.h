/**
 * ui_screens.h - Complete screen layouts
 *
 * High-level screen builders that compose UI components into full screens.
 * Each function builds a complete screen in the provided buffer:
 * - ui_screen_welcome(): Initial screen with key hints
 * - ui_screen_help(): Detailed help and controls
 * - ui_screen_playing(): Now playing screen with status
 * - ui_screen_loading(): Loading indicator for file loads
 */

#ifndef WALCMAN_UI_SCREENS_H
#define WALCMAN_UI_SCREENS_H

#include "ui_core.h"
#include "player.h"

/**
 * Build welcome screen (shown on startup)
 * buf: Buffer to build screen into
 * show_controls: Whether to display control hints
 */
void ui_screen_welcome(UIBuffer *buf, int show_controls);

/**
 * Build help screen showing all commands
 * buf: Buffer to build screen into
 */
void ui_screen_help(UIBuffer *buf);

/**
 * Build now-playing screen with status
 * buf: Buffer to build screen into
 * player: Player instance to get state from
 * show_controls: Whether to display control hints
 */
void ui_screen_playing(UIBuffer *buf, Player *player, int show_controls);

/**
 * Build loading screen while file is loading
 * buf: Buffer to build screen into
 * filepath: Path to file being loaded
 */
void ui_screen_loading(UIBuffer *buf, const char *filepath);

#endif // WALCMAN_UI_SCREENS_H
