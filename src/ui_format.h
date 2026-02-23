/**
 * ui_format.h - Text formatting utilities for UI
 *
 * Helper functions for formatting data for display:
 * - Time formatting (seconds to MM:SS or HH:MM:SS)
 * - Filename extraction and truncation
 * - Progress bar visual generation
 * - Version string access
 */

#ifndef WALCMAN_UI_FORMAT_H
#define WALCMAN_UI_FORMAT_H

#include <stddef.h>

/**
 * Format seconds as MM:SS or HH:MM:SS
 * buf: Output buffer
 * size: Buffer size
 * seconds: Time in seconds
 */
void ui_format_time(char *buf, size_t size, float seconds);

/**
 * Extract and truncate filename from full path
 * buf: Output buffer
 * size: Buffer size
 * path: Full file path
 * max_width: Maximum display width
 */
void ui_format_filename(char *buf, size_t size, const char *path, int max_width);

/**
 * Create a progress bar string (e.g., "[=====>    ]")
 * buf: Output buffer
 * size: Buffer size
 * progress: Progress value 0.0 to 1.0
 * width: Width of bar in characters
 */
void ui_format_progress_bar(char *buf, size_t size, float progress, int width);

/**
 * Get the current version string
 * Returns: Version string (e.g., "1.1.1")
 */
const char *ui_get_version(void);

/**
 * Get the UI color ANSI code from config
 * Returns: ANSI color code string (e.g., "\033[1;36m"), or "" if none
 */
const char *ui_get_color(void);

/**
 * Write UI color setting to config file
 * color_name: Name of color (e.g., "cyan"), or NULL/"" for default
 * Returns: 0 on success, -1 on error
 */
int ui_write_color_config(const char *color_name);

/**
 * Format text with a specific color
 * buf: Output buffer
 * size: Buffer size
 * text: Text to colorize
 * color_name: Color name (e.g., "red", "cyan")
 * Result: text wrapped with ANSI color codes
 */
void ui_format_with_color(char *buf, size_t size, const char *text, const char *color_name);

#endif // WALCMAN_UI_FORMAT_H
