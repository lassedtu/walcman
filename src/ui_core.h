/**
 * ui_core.h - Core UI rendering system
 *
 * Provides a buffer-based rendering system for atomic terminal updates.
 * UI components append to a UIBuffer, then ui_buffer_render() displays
 * everything at once, preventing flicker.
 *
 * Buffer automatically grows as needed to accommodate content.
 */

#ifndef WALCMAN_UI_CORE_H
#define WALCMAN_UI_CORE_H

#include <stddef.h>

// Dynamic string buffer for building UI screens
typedef struct UIBuffer
{
    char *buffer;    // Dynamically allocated string buffer
    size_t size;     // Current length of content (excluding null)
    size_t capacity; // Total allocated capacity
} UIBuffer;

/**
 * Create a new UI buffer with initial capacity
 * Returns: UIBuffer pointer on success, NULL on failure
 */
UIBuffer *ui_buffer_create(void);

/**
 * Destroy UI buffer and free memory
 * buf: Buffer to destroy
 */
void ui_buffer_destroy(UIBuffer *buf);

/**
 * Clear buffer contents (reset to empty)
 * buf: Buffer to clear
 */
void ui_buffer_clear(UIBuffer *buf);

/**
 * Append text to buffer
 * buf: Buffer to append to
 * text: String to append
 */
void ui_buffer_append(UIBuffer *buf, const char *text);

/**
 * Append formatted text to buffer (printf-style)
 * buf: Buffer to append to
 * fmt: Format string
 * ...: Format arguments
 */
void ui_buffer_appendf(UIBuffer *buf, const char *fmt, ...);

/**
 * Append a single character to buffer
 * buf: Buffer to append to
 * c: Character to append
 */
void ui_buffer_append_char(UIBuffer *buf, char c);

/**
 * Render buffer contents to terminal (atomic display)
 * buf: Buffer to render
 */
void ui_buffer_render(UIBuffer *buf);

/**
 * Clear terminal screen
 */
void ui_clear_screen(void);

#endif // WALCMAN_UI_CORE_H
