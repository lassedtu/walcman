/**
 * error.h - Error handling and reporting
 *
 * Centralized error management with error codes and user-friendly messages.
 * Use error_print() to display errors consistently throughout the application.
 */

#ifndef WALCMAN_ERROR_H
#define WALCMAN_ERROR_H

// Application error codes
typedef enum
{
    ERR_NONE = 0,           // No error
    ERR_PLAYER_INIT = 1,    // Failed to initialize audio engine
    ERR_FILE_LOAD = 2,      // Failed to load audio file
    ERR_PLAYBACK_START = 3, // Failed to start playback
    ERR_FILE_NOT_FOUND = 4, // File not found
    ERR_INVALID_FORMAT = 5, // Unsupported file format
} ErrorCode;

/**
 * Print an error message to stderr
 * code: Error code
 * details: Additional context (e.g., filename)
 */
void error_print(ErrorCode code, const char *details);

/**
 * Get error message for an error code
 * code: Error code
 * Returns: Human-readable error message
 */
const char *error_message(ErrorCode code);

#endif // WALCMAN_ERROR_H
