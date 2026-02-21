/**
 * util.h - General utility functions
 *
 * Miscellaneous helper functions used across the application.
 */

#ifndef WALCMAN_UTIL_H
#define WALCMAN_UTIL_H

/**
 * Remove surrounding quotes from a string (in-place)
 * Handles both single and double quotes
 * str: String to modify
 */
void strip_quotes(char *str);

/**
 * Remove shell escape characters from a path (in-place)
 * Converts "money\ pinkfloyd.mp3" to "money pinkfloyd.mp3"
 * str: String to modify
 */
void unescape_path(char *str);

#endif // WALCMAN_UTIL_H
