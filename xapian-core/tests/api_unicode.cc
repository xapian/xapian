/** @file api_unicode.cc
 * @brief test the Unicode and Utf8 classes and functions.
 */
/* Copyright (C) 2006,2007 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include <xapian.h>

#include "apitest.h"
#include "testutils.h"

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

// Test handling of invalid utf-8 is as desired.
bool test_utf8iterator1()
{
    const testcase * p;
    for (p = testcases; p->a; ++p) {
	tout << '"' << p->a << "\" and \"" << p->b << '"' << endl;
	size_t a_len = strlen(p->a);
	Xapian::Utf8Iterator a(p->a, a_len);

	size_t b_len = strlen(p->b);
	Xapian::Utf8Iterator b(p->b, b_len);

	while (a != Xapian::Utf8Iterator() && b != Xapian::Utf8Iterator()) {
	    TEST_EQUAL(*a, *b);
	    ++a;
	    ++b;
	}

	// Test that we don't reach the end of one before the other.
	TEST(a == Xapian::Utf8Iterator());
	TEST(b == Xapian::Utf8Iterator());
    }
    return true;
}

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

// Test decoding of utf-8.
bool test_utf8iterator2()
{
    const testcase2 * p;
    for (p = testcases2; p->a; ++p) {
	Xapian::Utf8Iterator a(p->a, strlen(p->a));

	TEST(a != Xapian::Utf8Iterator());
	TEST_EQUAL(*a, p->n);
	TEST(++a == Xapian::Utf8Iterator());
    }
    return true;
}

// Test Unicode categorisation.
bool test_unicode1()
{
    using namespace Xapian;
    TEST_EQUAL(Unicode::get_category('a'), Unicode::LOWERCASE_LETTER);
    TEST_EQUAL(Unicode::get_category('0'), Unicode::DECIMAL_DIGIT_NUMBER);
    TEST_EQUAL(Unicode::get_category('$'), Unicode::CURRENCY_SYMBOL);
    TEST_EQUAL(Unicode::get_category(0xa3), Unicode::CURRENCY_SYMBOL);
    // 0x242 was added in Unicode 5.0.0.
    TEST_EQUAL(Unicode::get_category(0x242), Unicode::LOWERCASE_LETTER);
    TEST_EQUAL(Unicode::get_category(0xFFFF), Unicode::UNASSIGNED);
    // Test characters outside BMP.
    TEST_EQUAL(Unicode::get_category(0x10345), Unicode::OTHER_LETTER);
    TEST_EQUAL(Unicode::get_category(0x10FFFD), Unicode::PRIVATE_USE);
    TEST_EQUAL(Unicode::get_category(0x10FFFF), Unicode::UNASSIGNED);
    // Test some invalid Unicode values.
    TEST_EQUAL(Unicode::get_category(0x11000), Unicode::UNASSIGNED);
    TEST_EQUAL(Unicode::get_category(0xFFFFFFFF), Unicode::UNASSIGNED);
    return true;
}

/** Test cases for the Unicode and Utf8 classes and functions. */
test_desc unicode_tests[] = {
    TESTCASE(utf8iterator1),
    TESTCASE(utf8iterator2),
    TESTCASE(unicode1),
    END_OF_TESTCASES
};
