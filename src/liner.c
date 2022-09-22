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
 * @file liner.c
 * @author Craig Jacobson
 * @brief Liner interpreter.
 */
#include "liner.h"

// TODO why doesn't include in symio not work?
#include <errno.h>

#include "symcore.h"
#include "symio.h"
#include "symmem.h"

error_t
_liner_stdin_open(void *src)
{
    return 0;
}

line_io_t
_liner_stdin_get_line(void *src, void *context)
{
    error_t err = 0;
    const size_t BUFLEN = 128;
    uint8_t *buf = memget(BUFLEN);
    size_t buflen = BUFLEN;
    uint8_t *seg = buf;
    size_t seglen = BUFLEN;
    uint8_t *nl = NULL; // Newline
    size_t linelen = 0;

    if (!buf)
    {
        return (line_io_t){ LINER_ERROR, { .error=ENOMEM } };
    }

    for (;;)
    {
        // Try to get a line
        seg = (uint8_t *)fgets((char *)seg, seglen, stdin);

        if (!seg)
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
                err = EOF;
            }
            break;
        }

        nl = memchr(seg, '\n', seglen);

        if (nl)
        {
            *nl = '\0';
            linelen = (nl - buf);
            break;
        }
        else
        {
            buflen = meminc(buflen);
            buf = memreget(buf, buflen);

            seg = seg + (seglen - 1);
            seglen = buflen - (seg - buf);
        }
    }

    if (err)
    {
        return (line_io_t){ LINER_ERROR, { .error=err } };
    }
    else
    {
        return (line_io_t){ LINER_LINE, { .line.s=buf, .line.len=linelen } };
    }
}

void
_liner_stdin_free_line(void *src, line_t line)
{
    memput(line.s);
}

error_t
_liner_stdin_close(void *src)
{
    return 0;
}

liner_t
mk_liner_from_stdin(void)
{
    liner_t liner =
    {
        NULL,
        _liner_stdin_open,
        _liner_stdin_get_line,
        _liner_stdin_free_line,
        _liner_stdin_close,
    };
    return liner;
}

static void
_print_header(void)
{
    printf("Welcome to SymbolScript!\n");
    printf("Type 'tutorial-syntax' for a tutorial on the syntax\n");
    printf("Type 'tutorial-core' for a tutorial on the core library\n");
    printf("Type 'help <symbol>' for help with that symbol\n");
    printf("Type 'doc <symbol>' for the technical documentation for that symbol\n");
}

error_t
_liner_cl_open(void *src)
{
    _print_header();
    return 0;
}

line_io_t
_liner_cl_get_line(void *src, void *context)
{
    // Print prompt
    int err = fputs(">>> ", stdout);
    if (EOF == err || err < 0)
    {
        fputs("\n", stdout);
        return (line_io_t){ LINER_ERROR, { .error=EOF } };
    }

    return _liner_stdin_get_line(src, context);
}

void
_liner_cl_free_line(void *src, line_t line)
{
    _liner_stdin_free_line(src, line);
}

error_t
_liner_cl_close(void *src)
{
    return 0;
}

liner_t
mk_liner_from_command_line(void)
{
    liner_t liner =
    {
        NULL,
        _liner_cl_open,
        _liner_cl_get_line,
        _liner_cl_free_line,
        _liner_cl_close,
    };
    return liner;
}

liner_t
mk_liner_from_file(const char *filepath)
{
    // TODO implement this
    return (liner_t){ 0 };
}

