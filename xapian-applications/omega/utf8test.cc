/* utf8test.cc: test the Utf8Iterator class
 *
 * Copyright (C) 2006,2007 Olly Betts
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
    { 0, 0 }
};

struct testcase2 {
    const char * a;
    unsigned long n;
};

static const testcase2 testcases2[] = {
    { "a", 97 },
    { "\x80", 128 },
    { "\xa0", 160 },
    { "\xc2\x80", 128 },
    { "\xc2\xa0", 160 },
    { "\xf0\xa8\xa8\x8f", 166415 },
    { 0, 0 }
};

int
main()
{
    const testcase * p;
    for (p = testcases; p->a; ++p) {
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

    const testcase2 * q;
    for (q = testcases2; q->a; ++q) {
	Utf8Iterator a(q->a, strlen(q->a));

	if (a == Utf8Iterator()) {
	    cerr << "\"" << p->a << "\" yields no characters" << endl;
	    exit(1);
	}
	if (*a != q->n) {
	    cerr << "*a yields " << *a << ", expected " << q->n << endl;
	    exit(1);
	}
	if (++a != Utf8Iterator()) {
	    cerr << "\"" << p->a << "\" yields more than one characters" << endl;
	    exit(1);
	}
    }
}
