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
 * @file symmem.h
 * @author Craig Jacobson
 * @brief Binary ops and memory allocation functions.
 */
#ifndef SYMBOLSCRIPT_SYMMEM_H_
#define SYMBOLSCRIPT_SYMMEM_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <string.h>


void *
memget(size_t);
void *
memreget(void *, size_t);
void
memput(void *);
size_t
meminc(size_t);


#ifdef __cplusplus
}
#endif
#endif /* SYMBOLSCRIPT_SYMMEM_H_ */

