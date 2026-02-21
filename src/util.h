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

#endif // WALCMAN_UTIL_H
