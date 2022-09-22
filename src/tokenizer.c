
#include <stdlib.h>

#include "debug.h"
#include "tokenizer.h"


#ifndef memzero
#include <string.h>
static void
memzero(void *p, size_t plen)
{
    memset(p, 0, plen);
}
#endif


static char _EMPTY[] = "";

const char *
toktype_name(enum toktype t)
{
    static const char *names[] =
    {
#define EXPORT_TOKEN_TYPE_NAME(index, id, name, ...) name,
FOR_TOKEN_TYPES( EXPORT_TOKEN_TYPE_NAME )
    };

    return t < (sizeof(names)/sizeof(*names)) ? names[(int)t] : "INVALID";
}

#ifdef DEBUG
static const char *
_tokstate_name(enum tokstate s)
{
    const char *names[] =
    {
    "begin",
    "middle",
    "end",
    "error",
    };
    return names[(int)s];
}
#endif

void
tokenizer_init(tokenizer_t *t)
{
    t->state = TOKSTATE_END;
    t->line = (const uint8_t *)_EMPTY;
    t->linelen = 0;
    t->linenum = 0;
    t->colindex = 0;
    t->colprevindex = 0;
}

void
tokenizer_destroy(tokenizer_t *t)
{
    memzero(t, sizeof(*t));
}

#ifdef DEBUG
void
tokenizer_print(tokenizer_t *t)
{
    printf("State: %s\n", _tokstate_name(t->state));
    printf("Line %d: %s (len %d)\n", (int)t->linenum, t->line, (int)t->linelen);
    printf("Column index/prev: %d/%d\n", (int)t->colindex, (int)t->colprevindex);
}
#endif

/**
 * @brief Set the current segment we're tokenizing.
 */
void
tokenizer_set_line(tokenizer_t *t, size_t linelen, const uint8_t *line)
{
    if (t->state == TOKSTATE_END)
    {
        t->state = TOKSTATE_BEG;
        t->line = line;
        t->linelen = linelen;
        ++t->linenum;
        t->colindex = 0;
        t->colprevindex = 0;
    }
}

static const
uint8_t _map[256] =
{
    // [x00, x0F]
    TOKTYPE_BAD, TOKTYPE_BAD, TOKTYPE_BAD, TOKTYPE_BAD,
    TOKTYPE_BAD, TOKTYPE_BAD, TOKTYPE_BAD, TOKTYPE_BAD,
    TOKTYPE_BAD, TOKTYPE_BAD, TOKTYPE_EOL, TOKTYPE_BAD,
    TOKTYPE_BAD, TOKTYPE_BAD, TOKTYPE_BAD, TOKTYPE_BAD,

    TOKTYPE_BAD, TOKTYPE_BAD, TOKTYPE_BAD, TOKTYPE_BAD,
    TOKTYPE_BAD, TOKTYPE_BAD, TOKTYPE_BAD, TOKTYPE_BAD,
    TOKTYPE_BAD, TOKTYPE_BAD, TOKTYPE_BAD, TOKTYPE_BAD,
    TOKTYPE_BAD, TOKTYPE_BAD, TOKTYPE_BAD, TOKTYPE_BAD,

    TOKTYPE_SPACE, 0, TOKTYPE_BINARY, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 

    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
};

static enum toktype
map(uint8_t c)
{
    return (enum toktype)_map[c];
}

static bool
_tokenize(tokenizer_t *t, token_t *tok)
{
    DPRINTF("_tokenize\n");
    const uint8_t *seg = t->line + t->colindex;
    size_t seglen = t->linelen - t->colindex;
    size_t len = 0;

    tok->col = t->colindex + 1;
    tok->line = t->linenum;

    // Update now because we may modify colindex
    t->colprevindex = t->colindex;

#ifdef DEBUG
    tokenizer_print(t);
#endif

    if (t->colindex < t->linelen)
    {
        DPRINTF("Tokenizing: %s\n", toktype_name(map(*seg)));
        tok->type = map(*seg);
        switch (map(*seg))
        {
            case TOKTYPE_SYMBOL:
                for (++len; len < seglen && TOKTYPE_SYMBOL == map(seg[len]); ++len);
                DPRINTF("Parsed symbol: %.*s\n", (int)len, seg);
                break;
            case TOKTYPE_BINARY:
                ++t->colindex;
                ++seg;
                --seglen;
                for (;;)
                {
                    for (; len < seglen && TOKTYPE_BINARY != map(seg[len]); ++len);
                    // If we hit the end of the line we're done
                    if (len == seglen)
                    {
                        break;
                    }
                    // Check for false positive
                    if (len > 1 && ('\\' != seg[len - 1]))
                    {
                        // Advance column index because of close "
                        ++t->colindex;
                        break;
                    }
                    else
                    {
                        // Move past quote to prevent inf loop
                        ++len;
                    }
                }
                DPRINTF("Parsed binary: \"%.*s\"\n", (int)len, seg);
                break;
            case TOKTYPE_LSPACE:
                return false;
            case TOKTYPE_SPACE:
                for (++len; len < seglen && TOKTYPE_SPACE == map(seg[len]); ++len);
                break;
            case TOKTYPE_EOL:
                return false;
            case TOKTYPE_BAD:
                for (++len; len < seglen && TOKTYPE_BAD == map(seg[len]); ++len);
                break;
            default:
                return false;
        }
    }
    else
    {
        tok->type = TOKTYPE_EOL;
        t->state = TOKSTATE_END;
    }

    tok->tok = seg;
    tok->toklen = len;

    t->colindex += len;

    return true;
}

bool
tokenize(tokenizer_t *t, token_t *tok)
{
    DPRINTF("State: %s\n", _tokstate_name(t->state));
    switch (t->state)
    {
        case TOKSTATE_BEG:
            {
                tok->type = TOKTYPE_LSPACE;
                tok->col = 1;
                tok->line = t->linenum;

                const uint8_t *seg = t->line + t->colindex;
                size_t seglen = t->linelen - t->colindex;
                size_t len = 0;
                for (; len < seglen && ' ' == seg[len]; ++len); 
                tok->tok = seg;
                tok->toklen = len;
                    
                t->colprevindex = t->colindex;
                t->colindex += len;
                t->state = TOKSTATE_MID;
            }
            return true;
        case TOKSTATE_MID:
            return _tokenize(t, tok);
        case TOKSTATE_END:
        case TOKSTATE_ERR:
        default:
            return false;
    }
}

