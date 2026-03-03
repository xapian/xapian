/** @file
 * @brief Test CSV escaping routines
 */
/* Copyright (C) 2011,2012,2013,2015 Olly Betts
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

#include "csvescape.h"

using namespace std;

struct testcase {
    const char * input;
    const char * result;
};

#define UNCHANGED NULL

static testcase csv_testcases[] = {
    { "", UNCHANGED },
    { "hello world", UNCHANGED },
    { "#$!%", UNCHANGED },
    { "\\ foo \\", UNCHANGED },
    // Check 3 examples from docs/omegascript.rst:
    { "Safe in CSV!", UNCHANGED },
    { "Not \"safe\"", "\"Not \"\"safe\"\"\"" },
    { "3, 2, 1", "\"3, 2, 1\"" },
    // Check CR and LF:
    { "Two\nlines", "\"Two\nlines\"" },
    { "Two\r\nDOS lines", "\"Two\r\nDOS lines\"" },
    // Test every possible character (except '\0') encodes as it should:
    { "\n", "\"\n\"" },
    { "\r", "\"\r\"" },
    { "\"", "\"\"\"\"" },
    { ",", "\",\"" },
    { "\x01\x02\x03\x04\x05\x06\x07", UNCHANGED },
    { "\x08\x09\x0B\x0C\x0E\x0F", UNCHANGED },
    { "\x10\x11\x12\x13\x14\x15\x16\x17", UNCHANGED },
    { "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F", UNCHANGED },
    { " !#$%&'()*+-./", UNCHANGED },
    { "0123456789:;<=>?", UNCHANGED },
    { "@ABCDEFGHIJKLMNO", UNCHANGED },
    { "PQRSTUVWXYZ[\\]^_", UNCHANGED },
    { "`abcdefghijklmno", UNCHANGED },
    { "pqrstuvwxyz{|}~\x7F", UNCHANGED },
    { "\x80\x81\x82\x83\x84\x85\x86\x87", UNCHANGED },
    { "\x88\x89\x8A\x8B\x8C\x8D\x8E\x8F", UNCHANGED },
    { "\x90\x91\x92\x93\x94\x95\x96\x97", UNCHANGED },
    { "\x98\x99\x9A\x9B\x9C\x9D\x9E\x9F", UNCHANGED },
    { "\xA0\xA1\xA2\xA3\xA4\xA5\xA6\xA7", UNCHANGED },
    { "\xA8\xA9\xAA\xAB\xAC\xAD\xAE\xAF", UNCHANGED },
    { "\xB0\xB1\xB2\xB3\xB4\xB5\xB6\xB7", UNCHANGED },
    { "\xB8\xB9\xBA\xBB\xBC\xBD\xBE\xBF", UNCHANGED },
    { "\xC0\xC1\xC2\xC3\xC4\xC5\xC6\xC7", UNCHANGED },
    { "\xC8\xC9\xCA\xCB\xCC\xCD\xCE\xCF", UNCHANGED },
    { "\xD0\xD1\xD2\xD3\xD4\xD5\xD6\xD7", UNCHANGED },
    { "\xD8\xD9\xDA\xDB\xDC\xDD\xDE\xDF", UNCHANGED },
    { "\xE0\xE1\xE2\xE3\xE4\xE5\xE6\xE7", UNCHANGED },
    { "\xE8\xE9\xEA\xEB\xEC\xED\xEE\xEF", UNCHANGED },
    { "\xF0\xF1\xF2\xF3\xF4\xF5\xF6\xF7", UNCHANGED },
    { "\xF8\xF9\xFA\xFB\xFC\xFD\xFE\xFF", UNCHANGED },
    { NULL, NULL }
};

int main() {
    for (testcase * e = csv_testcases; e->input; ++e) {
	const char * input = e->input;
	// Test csv_escape().
	string result = input;
	csv_escape(result);
	const char * expected = e->result;
	if (expected == UNCHANGED) expected = input;
	if (result != expected) {
	    cerr << "csv_escape of '" << input << "' should be "
		    "'" << expected << "', got '" << result << "'" << endl;
	    exit(1);
	}

	// Test csv_escape_always().
	result = input;
	csv_escape_always(result);
	string expected_always = expected;
	if (e->result == UNCHANGED) {
	    expected_always.insert(0, "\"");
	    expected_always.append("\"");
	}
	if (result != expected_always) {
	    cerr << "csv_escape_always of '" << input << "' should be "
		    "'" << expected << "', got '" << result << "'" << endl;
	    exit(1);
	}
    }
}
