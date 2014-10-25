/** @file jsonesctest.cc
 * @brief Test JSON escaping
 */
/* Copyright (C) 2011,2012,2013 Olly Betts
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

#include <cstdlib>
#include <iostream>
#include <string>

#include "jsonescape.h"

using namespace std;

struct testcase {
    const char * input;
    const char * result;
};

static testcase json_testcases[] = {
    { "", "" },
    { "hello world", "hello world" },
    { "#$!%", "#$!%" },
    { "\\ foo \\", "\\\\ foo \\\\" },
    // Test every possible character (except '\0') encodes as it should:
    { "\x01\x02\x03\x04\x05\x06\x07", "\\u0001\\u0002\\u0003\\u0004\\u0005\\u0006\\u0007" },
    { "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F", "\\b\\t\\n\\u000b\\f\\r\\u000e\\u000f" },
    { "\x10\x11\x12\x13\x14\x15\x16\x17", "\\u0010\\u0011\\u0012\\u0013\\u0014\\u0015\\u0016\\u0017" },
    { "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F", "\\u0018\\u0019\\u001a\\u001b\\u001c\\u001d\\u001e\\u001f" },
    { " !\"#$%&'()*+,-./", " !\\\"#$%&'()*+,-./" },
    { "0123456789:;<=>?", "0123456789:;<=>?" },
    { "@ABCDEFGHIJKLMNO", "@ABCDEFGHIJKLMNO" },
    { "PQRSTUVWXYZ[\\]^_", "PQRSTUVWXYZ[\\\\]^_" },
    { "`abcdefghijklmno", "`abcdefghijklmno" },
    { "pqrstuvwxyz{|}~\x7F", "pqrstuvwxyz{|}~\x7F" },
    { "\x80\x81\x82\x83\x84\x85\x86\x87", "\xc2\x80\xc2\x81\xc2\x82\xc2\x83\xc2\x84\xc2\x85\xc2\x86\xc2\x87" },
    { "\x88\x89\x8A\x8B\x8C\x8D\x8E\x8F", "\xc2\x88\xc2\x89\xc2\x8a\xc2\x8b\xc2\x8c\xc2\x8d\xc2\x8e\xc2\x8f" },
    { "\x90\x91\x92\x93\x94\x95\x96\x97", "\xc2\x90\xc2\x91\xc2\x92\xc2\x93\xc2\x94\xc2\x95\xc2\x96\xc2\x97" },
    { "\x98\x99\x9A\x9B\x9C\x9D\x9E\x9F", "\xc2\x98\xc2\x99\xc2\x9a\xc2\x9b\xc2\x9c\xc2\x9d\xc2\x9e\xc2\x9f" },
    { "\xA0\xA1\xA2\xA3\xA4\xA5\xA6\xA7", "\xc2\xa0\xc2\xa1\xc2\xa2\xc2\xa3\xc2\xa4\xc2\xa5\xc2\xa6\xc2\xa7" },
    { "\xA8\xA9\xAA\xAB\xAC\xAD\xAE\xAF", "\xc2\xa8\xc2\xa9\xc2\xaa\xc2\xab\xc2\xac\xc2\xad\xc2\xae\xc2\xaf" },
    { "\xB0\xB1\xB2\xB3\xB4\xB5\xB6\xB7", "\xc2\xb0\xc2\xb1\xc2\xb2\xc2\xb3\xc2\xb4\xc2\xb5\xc2\xb6\xc2\xb7" },
    { "\xB8\xB9\xBA\xBB\xBC\xBD\xBE\xBF", "\xc2\xb8\xc2\xb9\xc2\xba\xc2\xbb\xc2\xbc\xc2\xbd\xc2\xbe\xc2\xbf" },
    { "\xC0\xC1\xC2\xC3\xC4\xC5\xC6\xC7", "\xc3\x80\xc3\x81\xc3\x82\xc3\x83\xc3\x84\xc3\x85\xc3\x86\xc3\x87" },
    { "\xC8\xC9\xCA\xCB\xCC\xCD\xCE\xCF", "\xc3\x88\xc3\x89\xc3\x8a\xc3\x8b\xc3\x8c\xc3\x8d\xc3\x8e\xc3\x8f" },
    { "\xD0\xD1\xD2\xD3\xD4\xD5\xD6\xD7", "\xc3\x90\xc3\x91\xc3\x92\xc3\x93\xc3\x94\xc3\x95\xc3\x96\xc3\x97" },
    { "\xD8\xD9\xDA\xDB\xDC\xDD\xDE\xDF", "\xc3\x98\xc3\x99\xc3\x9a\xc3\x9b\xc3\x9c\xc3\x9d\xc3\x9e\xc3\x9f" },
    { "\xE0\xE1\xE2\xE3\xE4\xE5\xE6\xE7", "\xc3\xa0\xc3\xa1\xc3\xa2\xc3\xa3\xc3\xa4\xc3\xa5\xc3\xa6\xc3\xa7" },
    { "\xE8\xE9\xEA\xEB\xEC\xED\xEE\xEF", "\xc3\xa8\xc3\xa9\xc3\xaa\xc3\xab\xc3\xac\xc3\xad\xc3\xae\xc3\xaf" },
    { "\xF0\xF1\xF2\xF3\xF4\xF5\xF6\xF7", "\xc3\xb0\xc3\xb1\xc3\xb2\xc3\xb3\xc3\xb4\xc3\xb5\xc3\xb6\xc3\xb7" },
    { "\xF8\xF9\xFA\xFB\xFC\xFD\xFE\xFF", "\xc3\xb8\xc3\xb9\xc3\xba\xc3\xbb\xc3\xbc\xc3\xbd\xc3\xbe\xc3\xbf" },
    { NULL, NULL }
};

int main() {
    for (testcase * e = json_testcases; e->input; ++e) {
	string result = e->input;
	json_escape(result);
	if (result != e->result) {
	    cerr << "json_escape of " << e->input << " should be " << e->result
		 << "\", got \"" << result << "\"" << endl;
	    exit(1);
	}
    }
}
