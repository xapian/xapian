/* utf8test.cc: test the Utf8Iterator class
 *
 * Copyright (C) 2006 Olly Betts
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>

#include <iostream>
#include <string>

#include "utf8itor.h"

using namespace std;

struct testcase {
    const char * a, * b;
};

static const testcase testcases[] = {
    { "abcd", "abcd" }, // Sanity check!
    { "a\x80""bcd", "a\xc2\x80""bcd" },
    { "a\xa0", "a\xc2\xa0" }, 
};

int
main(int argc, char **argv)
{
    const testcase * p;
    for (p = testcases; p < testcases + sizeof(testcases) / sizeof(testcases[0]); ++p) {
	size_t a_len = strlen(p->a);
	Utf8Iterator a(p->a, a_len);

	size_t b_len = strlen(p->b);
	Utf8Iterator b(p->b, b_len);

	while (a != Utf8Iterator() && b != Utf8Iterator()) {
	    if (*a != *b) {
		cerr << "\"" << p->a << "\" and \"" << p->b << "\" don't compare equal but should" << endl;
		exit(1);
	    }
	    ++a;
	    ++b;
	}

	if (a != Utf8Iterator() || b != Utf8Iterator()) {
	    cerr << "\"" << p->a << "\" and \"" << p->b << "\" weren't the same length" << endl;
	    exit(1);
	}
    }
}
