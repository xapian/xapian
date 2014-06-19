/* @file urlencode.cc
 * @brief URL encoding as described by RFC3986.
 */
/* Copyright (C) 2011,2014 Olly Betts
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

#include "urlencode.h"

#include <cstring>
#include <string>

using namespace std;

void
url_encode_(string & res, const char * p, size_t len, const char * safe)
{
    while (len--) {
	unsigned char ch = *p++;
	if ((unsigned(ch) | 32u) - unsigned('a') <= unsigned('z' - 'a') ||
	    unsigned(ch) - unsigned('0') <= unsigned('9' - '0') ||
	    strchr(safe, ch)) {
	    // Unreserved by RFC3986.
	    res += ch;
	} else {
	    // RFC3986 says we "should" encode as upper case hex digits.
	    res += '%';
	    res += "0123456789ABCDEF"[ch >> 4];
	    res += "0123456789ABCDEF"[ch & 0x0f];
	}
    }
}

void
url_encode_path_lite(string & res, const char * p, size_t len)
{
    for (size_t i = 0; i != len; ++i) {
	unsigned char ch = p[i];
	if (ch < ' ' || strchr("#%:?", ch)) {
	    url_encode_(res, p, len, "/-._~");
	    return;
	}
    }
    res.append(p, len);
}
