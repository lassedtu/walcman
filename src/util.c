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
