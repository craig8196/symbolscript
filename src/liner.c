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

#include <stdlib.h>
#include <stdio.h>

#include "symio.h"
#include "symmem.h"


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
_liner_cl_get_line(void *src)
{
    return (line_io_t){ 0 };
}

error_t
_liner_cl_close(void *src)
{
    return 0;
}

liner_t
mk_liner_from_command_line(void)
{
    const int BUFLEN = 8;
    char *buf = memget(BUFLEN);
    liner_t liner =
    {
        NULL,
        BUFLEN,
        buf,
        _liner_cl_open,
        _liner_cl_get_line,
        _liner_cl_close,
    };
    return liner;
}

error_t
_liner_stdin_open(void *src)
{
    return 0;
}

line_io_t
_liner_stdin_get_line(void *src)
{
    return (line_io_t){ 0 };
}

error_t
_liner_stdin_close(void *src)
{
    return 0;
}

liner_t
mk_liner_from_stdin(void)
{
    const size_t BUFLEN = 8;
    char *buf = memget(BUFLEN);
    liner_t liner =
    {
        NULL,
        BUFLEN,
        buf,
        _liner_stdin_open,
        _liner_stdin_get_line,
        _liner_stdin_close,
    };
    return liner;
}

liner_t
mk_liner_from_file(const char *filepath)
{
    // TODO implement this
    return (liner_t){ 0 };
}

