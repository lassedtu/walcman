/**
 * error.c - Error handling implementation
 *
 * Provides centralized error messages and printing.
 * Maps error codes to user-friendly messages.
 */

#include <stdio.h>
#include "error.h"

const char *error_message(ErrorCode code)
{
    switch (code)
    {
    case ERR_NONE:
        return "No error";
    case ERR_PLAYER_INIT:
        return "Failed to initialize player";
    case ERR_FILE_LOAD:
        return "Could not load file";
    case ERR_PLAYBACK_START:
        return "Could not start playback";
    case ERR_FILE_NOT_FOUND:
        return "File not found";
    case ERR_INVALID_FORMAT:
        return "Invalid audio format";
    default:
        return "Unknown error";
    }
}

void error_print(ErrorCode code, const char *details)
{
    fprintf(stderr, "Error [%d]: %s", code, error_message(code));
    if (details)
    {
        fprintf(stderr, " (%s)", details);
    }
    fprintf(stderr, "\n");
}
