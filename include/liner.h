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
 * @file liner.h
 * @author Craig Jacobson
 * @brief Interface for a source of lines.
 */
#ifndef SYMBOLSCRIPT_LINER_H_
#define SYMBOLSCRIPT_LINER_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>

#include "symio.h"


enum liner_either
{
    LINER_LINE,
    LINER_ERROR,
};

typedef struct
{
    uint8_t *s;
    size_t len;
} line_t;

typedef struct
{
    enum liner_either type;
    union
    {
        line_t line;
        error_t error;
    } u;
} line_io_t;

typedef error_t (*liner_open_t)(void *src);
typedef line_io_t (*liner_get_line_t)(void *src, void *context);
typedef void (*liner_free_line_t)(void *src, line_t line);
typedef error_t (*liner_close_t)(void *src);

typedef struct
{
    void *src;
    liner_open_t open;
    liner_get_line_t get_line;
    liner_free_line_t free_line;
    liner_close_t close;
} liner_t;

liner_t
mk_liner_from_command_line(void);
liner_t
mk_liner_from_stdin(void);
liner_t
mk_liner_from_file(const char *filepath);


#ifdef __cplusplus
}
#endif
#endif /* SYMBOLSCRIPT_LINER_H_ */


