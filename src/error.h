#ifndef WALCMAN_ERROR_H
#define WALCMAN_ERROR_H

// Error codes
typedef enum
{
    ERR_NONE = 0,
    ERR_PLAYER_INIT = 1,
    ERR_FILE_LOAD = 2,
    ERR_PLAYBACK_START = 3,
    ERR_FILE_NOT_FOUND = 4,
    ERR_INVALID_FORMAT = 5,
} ErrorCode;

// Print error message with error code
void error_print(ErrorCode code, const char *details);

// Get human-readable error message
const char *error_message(ErrorCode code);

#endif // WALCMAN_ERROR_H
