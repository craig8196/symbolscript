/*******************************************************************************
 * Copyright (c) 2022 Craig Jacobson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 ******************************************************************************/
/**
 * @file sym.c
 * @author Craig Jacobson
 * @brief SymbolScript interpreter.
 */
#include <errno.h>
#include <string.h>

#include "symcore.h"
#include "symio.h"

#include "liner.h"
#include "tokenizer.h"


/**
 * @brief Chop the line up into tokens.
 */
static error_t
tokenize_line(tokenizer_t *tokenizer, line_t line)
{
    printf("here\n");



    token_t token;
    tokenizer_set_line(tokenizer, line.len, line.s);
    while (tokenize(tokenizer, &token))
    {
        printf("Token: type:%s, c:%lu, l:%lu, value:\"%.*s\"\n",
               toktype_name(token.type), token.col, token.line,
               (int)token.toklen, (const char *)token.tok);
    }
    return 0;
}

/**
 * @brief Reads lines from the liner and feeds them into the tokenizer.
 */
static error_t
read_lines(tokenizer_t *tokenizer, liner_t liner, void *lexer, void *context)
{
    error_t err = liner.open(liner.src);
    if (err)
    {
        return err;
    }

    bool done = false;
    while (!done)
    {
        line_io_t either_line = liner.get_line(liner.src, context);

        switch (either_line.type)
        {
            case LINER_LINE:
                {
                    line_t line = either_line.u.line;
                    printf("Line: %.*s\n", (int)line.len, (char *)line.s);
                    err = tokenize_line(tokenizer, line);
                    if (err)
                    {
                        done = true;
                    }
                    liner.free_line(liner.src, line);
                }
                break;
            case LINER_ERROR:
                err = either_line.u.error;
                if (EOF == err)
                {
                    done = true;
                    err = 0;
                }
                else if (err)
                {
                    done = true;
                }
                break;
            default:
                {
                    fputs("Fatal error: unknown type in either_line\n", stderr);
                    err = EIO;
                }
                break;
        }
    }

    error_t err2 = liner.close(liner.src);
    if (!err)
    {
        err = err2;
    }

    return err;
}

int
main(int argc, char *argv[])
{
    if (argc > 3)
    {
        fputs("Max of one argument allowed\n", stderr);
        return -1;
    }

    liner_t liner;
    tokenizer_t tokenizer;

    if (argc < 2)
    {
        liner = mk_liner_from_command_line();
    }
    else if (!strcmp("-", argv[1]))
    {
        liner = mk_liner_from_stdin();
    }
    else
    {
        liner = mk_liner_from_file(argv[1]);
    }

    tokenizer_init(&tokenizer);

    error_t err = read_lines(&tokenizer, liner, NULL, NULL);

    tokenizer_destroy(&tokenizer);

    return err;
}

