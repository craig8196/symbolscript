
#include <stdint.h>
#include <string.h>

#include "bdd.h"
#include "tokenizer.h"


//#define DEBUG

#if defined DEBUG && !DPRINTF
#include <stdio.h>
#define DPRINTF(...) printf(__VA_ARGS__);
#else
#define DPRINTF(...)
#endif

#define TOBUF (uint8_t *)

static tokenizer_t _t;
static tokenizer_t *t = &_t;

void
toksetlen(token_t *expect, size_t elen)
{
    size_t i;
    for (i = 0; i < elen; ++i)
    {
        expect[i].toklen = strlen(expect[i].tok);
    };
}

bool
tokeq(const token_t *e, const token_t *t)
{
    DPRINTF("tokeq\n");
    if (e->type != t->type)
    {
        DPRINTF("Token type mismatch: expect/actual (%s/%s)\n",
               toktype_name(e->type), toktype_name(t->type));
        return false;
    }
    if (e->col != t->col)
    {
        DPRINTF("Token col mismatch: expect/actual (%d/%d)\n",
               (int)e->col, (int)t->col);
        return false;
    }
    if (e->line != t->line)
    {
        DPRINTF("Token line mismatch: expect/actual (%d/%d)\n",
               (int)e->line, (int)t->line);
        return false;
    }
    if (e->toklen != t->toklen)
    {
        DPRINTF("Token token length mismatch: expect/actual (%d/%d)\n",
               (int)e->toklen, (int)t->toklen);
        return false;
    }
    if (memcmp(e->tok, t->tok, t->toklen))
    {
        DPRINTF("Token value mismatch: expect/actual (%s/%s)\n",
               e->tok, t->tok);
        return false;
    }

    return true;
}

bool
tokmatch(tokenizer_t *t, const uint8_t *input, size_t ilen, token_t *expect, size_t elen)
{
    tokenizer_set_line(t, TOBUF input, ilen);

    token_t tok;
    const token_t *ex;
    size_t count = 0;
    while (tokenize(t, &tok))
    {
        if (count > elen)
        {
            break;
        }

        ex = expect + count;

        if (!tokeq(ex, &tok))
        {
            return false;
        }

        ++count;
    }

    if (count != elen)
    {
        DPRINTF("Wrong number of tokens tokenized: expect/actual (%d/%d)\n",
               (int)elen, (int)count);
    }

    return count == elen;
}

spec("symbolscript library")
{
    describe("tokenizer")
    {
        before_each()
        {
            tokenizer_init(t);
        }

        after_each()
        {
            tokenizer_destroy(t);
        }

        it("should initialize and destroy")
        {
            token_t tok;
            check(!tokenize(t, &tok), "Was not end of segment.");
        }

        it("tokenizes blank line")
        {
            const char *input = "";
            size_t ilen = strlen(input);

            token_t expect[] =
            {
                {
                    .type = TOKTYPE_LSPACE,
                    .col = 1,
                    .line = 1,
                    .tok = "",
                    .toklen = 0,
                },
                {
                    .type = TOKTYPE_EOL,
                    .col = 1,
                    .line = 1,
                    .tok = "",
                    .toklen = 0,
                },
            };
            const size_t elen = sizeof(expect)/sizeof(*expect);
            toksetlen(expect, elen);

            check(tokmatch(t, input, ilen, expect, elen), "Tokens do not match");
        }

        it("tokenizes a symbol")
        {
            const char *input = "symbolic";
            size_t ilen = strlen(input);

            token_t expect[] =
            {
                {
                    .type = TOKTYPE_LSPACE,
                    .col = 1,
                    .line = 1,
                    .tok = "",
                    .toklen = 0,
                },
                {
                    .type = TOKTYPE_SYMBOL,
                    .col = 1,
                    .line = 1,
                    .tok = "symbolic",
                    .toklen = 0,
                },
                {
                    .type = TOKTYPE_EOL,
                    .col = 9,
                    .line = 1,
                    .tok = "",
                    .toklen = 0,
                },
            };
            const size_t elen = sizeof(expect)/sizeof(*expect);
            toksetlen(expect, elen);

            check(tokmatch(t, input, ilen, expect, elen), "Tokens do not match");
        }

        it("tokenizes space")
        {
            const char *input = "    ";
            size_t ilen = strlen(input);

            token_t expect[] =
            {
                {
                    .type = TOKTYPE_LSPACE,
                    .col = 1,
                    .line = 1,
                    .tok = "    ",
                    .toklen = 0,
                },
                {
                    .type = TOKTYPE_EOL,
                    .col = 5,
                    .line = 1,
                    .tok = "",
                    .toklen = 0,
                },
            };
            const size_t elen = sizeof(expect)/sizeof(*expect);
            toksetlen(expect, elen);

            check(tokmatch(t, input, ilen, expect, elen), "Tokens do not match");
        }

        it("tokenizes binary")
        {
            const char *input = "\"asdf\"";
            size_t ilen = strlen(input);

            token_t expect[] =
            {
                {
                    .type = TOKTYPE_LSPACE,
                    .col = 1,
                    .line = 1,
                    .tok = "",
                    .toklen = 0,
                },
                {
                    .type = TOKTYPE_BINARY,
                    .col = 1,
                    .line = 1,
                    .tok = "asdf",
                    .toklen = 0,
                },
                {
                    .type = TOKTYPE_EOL,
                    .col = 7,
                    .line = 1,
                    .tok = "",
                    .toklen = 0,
                },
            };
            const size_t elen = sizeof(expect)/sizeof(*expect);
            toksetlen(expect, elen);

            check(tokmatch(t, input, ilen, expect, elen), "Tokens do not match");
        }

        it("tokenizes binary to EOL")
        {
            const char *input = "\"asdf";
            size_t ilen = strlen(input);

            token_t expect[] =
            {
                {
                    .type = TOKTYPE_LSPACE,
                    .col = 1,
                    .line = 1,
                    .tok = "",
                    .toklen = 0,
                },
                {
                    .type = TOKTYPE_BINARY,
                    .col = 1,
                    .line = 1,
                    .tok = "asdf",
                    .toklen = 0,
                },
                {
                    .type = TOKTYPE_EOL,
                    .col = 6,
                    .line = 1,
                    .tok = "",
                    .toklen = 0,
                },
            };
            const size_t elen = sizeof(expect)/sizeof(*expect);
            toksetlen(expect, elen);

            check(tokmatch(t, input, ilen, expect, elen), "Tokens do not match");
        }

        it("tokenizes binary with quotes")
        {
            const char *input = "\"asdf\\\"\"";
            size_t ilen = strlen(input);

            token_t expect[] =
            {
                {
                    .type = TOKTYPE_LSPACE,
                    .col = 1,
                    .line = 1,
                    .tok = "",
                    .toklen = 0,
                },
                {
                    .type = TOKTYPE_BINARY,
                    .col = 1,
                    .line = 1,
                    .tok = "asdf\\\"",
                    .toklen = 0,
                },
                {
                    .type = TOKTYPE_EOL,
                    .col = 9,
                    .line = 1,
                    .tok = "",
                    .toklen = 0,
                },
            };
            const size_t elen = sizeof(expect)/sizeof(*expect);
            toksetlen(expect, elen);

            check(tokmatch(t, input, ilen, expect, elen), "Tokens do not match");
        }
    }
}

