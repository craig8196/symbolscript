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
 * @file tokizer.h
 * @author Craig Jacobson
 * @brief SymbolScript tokenizer.
 */
#ifndef SYMBOLSCRIPT_TOKENIZER_H_
#define SYMBOLSCRIPT_TOKENIZER_H_
#ifdef __cplusplus
extern "C" {
#endif


#include <stdbool.h>
#include <stdint.h>


/*******************************************************************************
 * TOKENIZER
 *
 * Takes a pointer into a line and returns the next symbol.
 * Binary/strings with quotes go to the end of the quote or end of line,
 * whichever comes first.
 ******************************************************************************/

/*******************************************************************************
 *  TOKTYPE_SYMBOL, // Symbols
 *  TOKTYPE_BINARY, // Binary/string type (aka "stuff in quotes")
 *  TOKTYPE_LSPACE, // Lead space
 *  TOKTYPE_BSPACE, // Spaces between other tokens
 *  TOKTYPE_EOL, // End-of-line
 *  TOKTYPE_BAD, // Bad token char (control and other wspace char)
 ******************************************************************************/
#define FOR_TOKEN_TYPES(DO) \
    DO(0, SYMBOL, "symbol") \
    DO(1, BINARY, "binary") \
    DO(2, LSPACE, "lead-space") \
    DO(3, SPACE, "space") \
    DO(4, EOL, "end-of-line") \
    DO(5, BAD, "bad")

enum toktype
{
#define DEFINE_TOKEN_TYPE_ENUM(index, id, ...) TOKTYPE_ ## id = index,
FOR_TOKEN_TYPES( DEFINE_TOKEN_TYPE_ENUM )
};

const char *
toktype_name(enum toktype t);

typedef struct
{
    enum toktype type;
    uint64_t col;
    uint64_t line;
    const uint8_t *tok;
    size_t toklen;
} token_t;

enum tokstate
{
    TOKSTATE_BEG,
    TOKSTATE_MID,
    TOKSTATE_END,
    TOKSTATE_ERR,
};


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
#endif /* SYMBOLSCRIPT_TOKENIZER_H_ */

