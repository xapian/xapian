/** @file
 * @brief JSON escape a string
 */
/* Copyright (C) 2013,2023 Olly Betts
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

#include "jsonescape.h"

#include <string>

#include <xapian.h>

using namespace std;

enum { UNI = 0, CLR = 1 };

static const char json_tab['"' + 1] = {
    UNI, UNI, UNI, UNI, UNI, UNI, UNI, UNI, // 0-7
    'b', 't', 'n', UNI, 'f', 'r', UNI, UNI, // 8-15
    UNI, UNI, UNI, UNI, UNI, UNI, UNI, UNI, // 16-23
    UNI, UNI, UNI, UNI, UNI, UNI, UNI, UNI, // 24-31
    CLR, CLR, '"'
};

void
json_escape(string &s)
{
    string r;
    for (Xapian::Utf8Iterator i(s); i != Xapian::Utf8Iterator(); ++i) {
	unsigned int ch = *i;
	if (ch >= 128) {
	    Xapian::Unicode::append_utf8(r, ch);
	    continue;
	}
	if (ch > '"') {
	    if (ch != '\\') {
		r += ch;
		continue;
	    }
	} else {
	    unsigned char t = json_tab[ch];
	    if (t == CLR) {
		// Can be sent "in the clear".
		r += ch;
		continue;
	    }
	    if (t == UNI) {
		// Encode as Unicode escape sequence.  We know ch <= 0x1f.
		r.append("\\u0000", 6);
		if (ch >= 16) r[r.size() - 2] = '1';
		r.back() = "0123456789abcdef"[ch & 0x0f];
		continue;
	    }
	    ch = t;
	}
	char buf[2] = {'\\', char(ch)};
	r.append(buf, 2);
    }
    swap(s, r);
}
