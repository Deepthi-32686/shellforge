#include <stdlib.h>
#include "token.h"

void free_tokens(Token *tokens, int count)
{
    for (int i = 0; i < count; i++)
    {
        free(tokens[i].value);
    }

    free(tokens);
}
