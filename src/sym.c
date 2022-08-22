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
#include "sym.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "liner.h"


static int
read_lines(void)
{
    int err = 0;
    char *line = NULL;
    size_t linelen = 0;

    for (;;)
    {
        if (!flushing)
        {
            err = fputs(">>> ", stdout);
            if (EOF == err)
            {
                printf("here %d\n", err);
                break;
            }
            else
            {
                err = 0;
            }
        }

        line = fgets(linebuf, linebuflen, stdin);

        if (!line)
        {
            // Ignoring err output because we already encountered an error
            if (errno)
            {
                fputs("\nAn error occurred on stdin, exiting...\n", stderr);
                err = errno;
            }
            else
            {
                fputs("\nExiting\n", stdout);
                err = 0;
            }
            break;
        }

        char *nl = memchr(line, '\n', linebuflen);

        // Remove newline, or skip when flushing
        if (nl)
        {
            *nl = '\0';
            flushing = false;
        }
        else
        {
            if (!flushing)
            {
                err = fputs("Line was too long and was ignored\n"
                            "Try breaking the line up\n",
                            stderr);
                if (EOF == err)
                {
                    break;
                }
                else
                {
                    err = 0;
                }
                flushing = true;
            }
        }

        if (!flushing)
        {
            // do something with line
        }
    }

    free(linebuf);

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

    return read_lines();
}

