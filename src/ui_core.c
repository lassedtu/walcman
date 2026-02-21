/**
 * ui_core.c - Core UI rendering system implementation
 *
 * Implements a dynamic string buffer for building UI screens.
 * The buffer automatically grows as content is added, and
 * ui_buffer_render() displays everything atomically to prevent flicker.
 *
 * Buffer growth strategy: doubles capacity when full.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "ui_core.h"

#define INITIAL_BUFFER_SIZE 4096 // Start with 4KB buffer

UIBuffer *ui_buffer_create(void)
{
    UIBuffer *buf = (UIBuffer *)malloc(sizeof(UIBuffer));
    if (!buf)
        return NULL;

    buf->capacity = INITIAL_BUFFER_SIZE;
    buf->buffer = (char *)malloc(buf->capacity);
    if (!buf->buffer)
    {
        free(buf);
        return NULL;
    }

    buf->size = 0;
    buf->buffer[0] = '\0';
    return buf;
}

void ui_buffer_destroy(UIBuffer *buf)
{
    if (!buf)
        return;

    if (buf->buffer)
        free(buf->buffer);

    free(buf);
}

void ui_buffer_clear(UIBuffer *buf)
{
    if (!buf)
        return;

    buf->size = 0;
    if (buf->buffer)
        buf->buffer[0] = '\0';
}

static void ui_buffer_grow(UIBuffer *buf, size_t needed)
{
    if (buf->size + needed < buf->capacity)
        return;

    size_t new_capacity = buf->capacity * 2;
    while (new_capacity < buf->size + needed + 1)
        new_capacity *= 2;

    char *new_buffer = (char *)realloc(buf->buffer, new_capacity);
    if (!new_buffer)
        return;

    buf->buffer = new_buffer;
    buf->capacity = new_capacity;
}

void ui_buffer_append(UIBuffer *buf, const char *text)
{
    if (!buf || !text)
        return;

    size_t text_len = strlen(text);
    ui_buffer_grow(buf, text_len);

    memcpy(buf->buffer + buf->size, text, text_len);
    buf->size += text_len;
    buf->buffer[buf->size] = '\0';
}

void ui_buffer_appendf(UIBuffer *buf, const char *fmt, ...)
{
    if (!buf || !fmt)
        return;

    va_list args;
    va_start(args, fmt);

    // First, calculate how much space we need
    va_list args_copy;
    va_copy(args_copy, args);
    int needed = vsnprintf(NULL, 0, fmt, args_copy);
    va_end(args_copy);

    if (needed < 0)
    {
        va_end(args);
        return;
    }

    ui_buffer_grow(buf, needed);

    vsnprintf(buf->buffer + buf->size, needed + 1, fmt, args);
    buf->size += needed;

    va_end(args);
}

void ui_buffer_append_char(UIBuffer *buf, char c)
{
    if (!buf)
        return;

    ui_buffer_grow(buf, 1);
    buf->buffer[buf->size++] = c;
    buf->buffer[buf->size] = '\0';
}

void ui_buffer_render(UIBuffer *buf)
{
    if (!buf || !buf->buffer)
        return;

    ui_clear_screen();
    printf("%s", buf->buffer);
    fflush(stdout);
}

void ui_clear_screen(void)
{
    printf("\033[2J\033[H");
    fflush(stdout);
}
