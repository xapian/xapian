/* @file jsonescape.cc
 * @brief JSON escape a string
 */
/* Copyright (C) 2013 Olly Betts
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

#include <cstdio>
#include <string>

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
    for (size_t i = 0; i != s.size(); ++i) {
	unsigned char ch = s[i];
	if (ch > '"') {
	    if (ch != '\\')
		continue;
	} else {
	    unsigned char t = json_tab[ch];
	    if (t == CLR)
		continue;
	    if (t == UNI) {
		char buf[7];
		sprintf(buf, "\\u00%02x", ch);
		s.replace(i, 1, buf, 6);
		i += 5;
		continue;
	    }
	    ch = t;
	}
	char buf[2] = {'\\', char(ch)};
	s.replace(i, 1, buf, 2);
	++i;
    }
}
