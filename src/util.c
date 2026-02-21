/**
 * util.c - General utility functions
 *
 * Miscellaneous helper functions used across the application.
 */

#include <string.h>
#include "util.h"

void strip_quotes(char *str)
{
    if (!str || strlen(str) < 2)
        return;

    int len = strlen(str);

    // Handle single quotes or double quotes
    if ((str[0] == '\'' && str[len - 1] == '\'') ||
        (str[0] == '"' && str[len - 1] == '"'))
    {
        memmove(str, str + 1, len - 2);
        str[len - 2] = '\0';
    }
}

void unescape_path(char *str)
{
    if (!str)
        return;

    char *src = str;
    char *dst = str;

    while (*src)
    {
        // Skip backslash if it's escaping a special character
        if (*src == '\\' && *(src + 1) != '\0')
        {
            src++;           // Skip the backslash
            *dst++ = *src++; // Copy the escaped character
        }
        else
        {
            *dst++ = *src++; // Copy normally
        }
    }
    *dst = '\0'; // Null terminate
}
