/** @file
 * @brief Efficient keyword to enum lookup
 */
/* Copyright (C) 2012,2016 Olly Betts
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <config.h>

#include "keyword.h"

#include <string.h>

int
keyword(const unsigned char * p, const char * s, size_t len)
{
    if (len == 0 || len > p[0] || p[len] == 1)
	return -1;
    p = p + p[0] + p[len] + 3;
    size_t n = len + 1;
    const unsigned char * q = p + n * (p[-2] + 1);
    /* Binary chop between p and q */
    size_t d = n * 2;
    while (p < q) {
	const unsigned char * m = p + (q - p) / d * n;
	int cmp = memcmp(s, m, len);
	if (cmp < 0) {
	    q = m;
	} else if (cmp > 0) {
	    p = m + n;
	} else {
	    return m[-1];
	}
    }
    return -1;
}

int
keyword2(const unsigned char * p, const char * s, size_t len)
{
    if (len == 0 || len > p[0])
	return -1;
    unsigned offset = (p[len * 2 - 1] | p[len * 2] << 8);
    if (offset == 1)
	return -1;
    p = p + 2 * p[0] + offset + 3;
    size_t n = len + 1;
    const unsigned char * q = p + n * (p[-2] + 1);
    /* Binary chop between p and q */
    size_t d = n * 2;
    while (p < q) {
	const unsigned char * m = p + (q - p) / d * n;
	int cmp = memcmp(s, m, len);
	if (cmp < 0) {
	    q = m;
	} else if (cmp > 0) {
	    p = m + n;
	} else {
	    return m[-1];
	}
    }
    return -1;
}
