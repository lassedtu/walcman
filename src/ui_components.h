/**
 * ui_components.h - Reusable UI widgets
 *
 * A library of composable UI components that can be combined to build screens.
 * Components append their output to the provided UIBuffer.
 *
 * Categories:
 * - Layout: Header, footer, separators
 * - Status: Status lines, file info
 * - Interactive: Key hints
 * - Progress: Progress bars, time displays
 * - Messages: General messages, loading indicators
 */

#ifndef WALCMAN_UI_COMPONENTS_H
#define WALCMAN_UI_COMPONENTS_H

#include "ui_core.h"

// === Layout Components ===

/**
 * Render application header with title and version
 * buf: Buffer to append to
 */
void ui_component_header(UIBuffer *buf);

/**
 * Render a horizontal separator line
 * buf: Buffer to append to
 * width: Width of separator in characters
 */
void ui_component_separator(UIBuffer *buf, int width);

/**
 * Render footer text
 * buf: Buffer to append to
 */
void ui_component_footer(UIBuffer *buf);

// === Status Components ===

/**
 * Render a status line with icon and text
 * buf: Buffer to append to
 * icon: Icon/symbol to display (e.g., "♪", "⏸")
 * text: Status text
 */
void ui_component_status_line(UIBuffer *buf, const char *icon, const char *text);

/**
 * Render file information line
 * buf: Buffer to append to
 * filename: File name/path to display
 */
void ui_component_file_info(UIBuffer *buf, const char *filename);

// === Interactive Components ===

/**
 * Render a single key hint (e.g., "[p] Play")
 * buf: Buffer to append to
 * key: Key to press (e.g., "p", "space")
 * description: What the key does
 */
void ui_component_key_hint(UIBuffer *buf, const char *key, const char *description);

/**
 * Render section header for key hints
 * buf: Buffer to append to
 * title: Section title (e.g., "Commands")
 */
void ui_component_key_hints_section(UIBuffer *buf, const char *title);

// === Progress Components ===

/**
 * Render a progress bar
 * buf: Buffer to append to
 * progress: Progress value 0.0 to 1.0
 * width: Width of progress bar in characters
 */
void ui_component_progress_bar(UIBuffer *buf, float progress, int width);

/**
 * Render current/total time display
 * buf: Buffer to append to
 * current: Current time in seconds
 * total: Total duration in seconds
 */
void ui_component_time_display(UIBuffer *buf, float current, float total);

// === Message Components ===

/**
 * Render a simple message
 * buf: Buffer to append to
 * message: Message text to display
 */
void ui_component_message(UIBuffer *buf, const char *message);

/**
 * Render loading indicator
 * buf: Buffer to append to
 * filepath: File being loaded
 */
void ui_component_loading(UIBuffer *buf, const char *filepath);

#endif // WALCMAN_UI_COMPONENTS_H
