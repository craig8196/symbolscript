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
 * @file mmanager.h
 * @author Craig Jacobson
 * @brief Symbol Script memory manager.
 *
 * The memory manager has different ways of organizing
 */
#ifndef SYMBOLSCRIPT_CONTEXT_H_
#define SYMBOLSCRIPT_CONTEXT_H_
#ifdef __cplusplus
extern "C" {
#endif


#include <stdbool.h>
#include <stdint.h>


/*******************************************************************************
 * CONTEXT
 *
 * Basically a map.
 * Since most contexts are small and lookup is done at binding time
 * we will start by using an array.
 ******************************************************************************/

// Immutability
#define BINDFLAG_LET  0x00000000

typedef struct
{
    int flags;
    const char *name;
    void *bound;
    void *meta; // aka annotations, but meta is shorter
} binding_t;

typedef struct
{
    char quicklist[8];
    binding_t *bindings;
    size_t bindingslen;
} context_t;

/**
 * Tokenizer.
 * Assumes that input buffers are a valid stream of UTF-8 characters.
 * ```
 * tokenizer_init(t);
 * while (line = get_line())
 * {
 *   tokenizer_set_line(t, line, linelen);
 *   while (tokenize(t, &tok)) { // stuff; }
 * }
 * tokenizer_destroy(t);
 * ```
 */
typedef struct
{
    enum tokstate state;
    const uint8_t *line;
    size_t linelen;
    uint64_t linenum; // Updated with each set line call
    uint64_t colindex; // Updated after each token is output
    uint64_t colprevindex;
} tokenizer_t;

void
tokenizer_init(tokenizer_t *t);
void
tokenizer_destroy(tokenizer_t *t);
void
tokenizer_set_line(tokenizer_t *t, const uint8_t *line, size_t linelen);
bool
tokenize(tokenizer_t *t, token_t *tok);

#ifdef __cplusplus
}
#endif
#endif /* SYMBOLSCRIPT_CONTEXT_H_ */

