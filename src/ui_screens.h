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
#include "queue.h"

/**
 * Build welcome screen (shown on startup)
 * buf: Buffer to build screen into
 * show_controls: Whether to display control hints
 * repeat_symbol: Compact repeat symbol (e.g. "⇾", "↺", "↻")
 * shuffle_symbol: Compact shuffle symbol ("-" or "~")
 * repeat_label: Repeat mode label (e.g. "Off", "Song", "Playlist")
 */
void ui_screen_welcome(UIBuffer *buf, int show_controls, const char *repeat_symbol, const char *shuffle_symbol, const char *repeat_label);

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
 * repeat_symbol: Compact repeat symbol (e.g. "⇾", "↺", "↻")
 * repeat_label: Repeat mode label (e.g. "Off", "Song", "Playlist")
 */
void ui_screen_playing(UIBuffer *buf, Player *player, int show_controls, const char *repeat_symbol, const char *repeat_label);

/**
 * Build loading screen while file is loading
 * buf: Buffer to build screen into
 * filepath: Path to file being loaded
 */
void ui_screen_loading(UIBuffer *buf, const char *filepath);

/**
 * Build settings menu screen
 * buf: Buffer to build screen into
 */
void ui_screen_settings(UIBuffer *buf);

/**
 * Build color picker screen
 * buf: Buffer to build screen into
 * selected_color: Currently selected color name (NULL for default)
 */
void ui_screen_color_picker(UIBuffer *buf, const char *selected_color);

/**
 * Build queue view screen
 * buf: Buffer to build screen into
 * queue: Queue state to display
 * repeat_symbol: Compact repeat symbol
 * repeat_label: Current repeat mode label
 */
void ui_screen_queue(UIBuffer *buf, const Queue *queue, const char *repeat_symbol, const char *repeat_label);

#endif // WALCMAN_UI_SCREENS_H
