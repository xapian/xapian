/** @file unittest.cc
 * @brief Unit tests of non-Xapian-specific internal code.
 */
/* Copyright (C) 2006,2007,2010,2012 Olly Betts
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

#include <cfloat>
#include <iostream>

#include "testsuite.h"

using namespace std;

#define XAPIAN_UNITTEST

// Utility code we use:
#include "../common/str.cc"
#include "../common/stringutils.cc"
#include "../common/utils.cc"

// Simpler version of TEST_EXCEPTION macro.
#define TEST_EXCEPTION(TYPE, CODE) \
    do { \
	try { \
	    CODE; \
	    FAIL_TEST("Expected exception "#TYPE" not thrown"); \
	} catch (const TYPE &) { \
	} \
    } while (0)

// Code we're unit testing:
#include "../common/fileutils.cc"
#include "../common/serialise-double.cc"
#include "../net/length.cc"

inline string
r_r_p(string a, const string & b)
{
    resolve_relative_path(a, b);
    return a;
}

DEFINE_TESTCASE_(resolverelativepath1) {
    TEST_EQUAL(r_r_p("/abs/o/lute", ""), "/abs/o/lute");
    TEST_EQUAL(r_r_p("/abs/o/lute", "/"), "/abs/o/lute");
    TEST_EQUAL(r_r_p("/abs/o/lute", "//"), "/abs/o/lute");
    TEST_EQUAL(r_r_p("/abs/o/lute", "foo"), "/abs/o/lute");
    TEST_EQUAL(r_r_p("/abs/o/lute", "foo/"), "/abs/o/lute");
    TEST_EQUAL(r_r_p("/abs/o/lute", "/foo"), "/abs/o/lute");
    TEST_EQUAL(r_r_p("/abs/o/lute", "/foo/"), "/abs/o/lute");
    TEST_EQUAL(r_r_p("/abs/o/lute", "foo/bar"), "/abs/o/lute");
    TEST_EQUAL(r_r_p("/abs/o/lute", "foo/bar/"), "/abs/o/lute");
    TEST_EQUAL(r_r_p("/abs/o/lute", "/foo/bar"), "/abs/o/lute");
    TEST_EQUAL(r_r_p("/abs/o/lute", "/foo/bar/"), "/abs/o/lute");
    TEST_EQUAL(r_r_p("rel/a/tive", ""), "rel/a/tive");
    TEST_EQUAL(r_r_p("rel/a/tive", "/"), "/rel/a/tive");
    TEST_EQUAL(r_r_p("rel/a/tive", "//"), "//rel/a/tive");
    TEST_EQUAL(r_r_p("rel/a/tive", "foo"), "rel/a/tive");
    TEST_EQUAL(r_r_p("rel/a/tive", "foo/"), "foo/rel/a/tive");
    TEST_EQUAL(r_r_p("rel/a/tive", "/foo"), "/rel/a/tive");
    TEST_EQUAL(r_r_p("rel/a/tive", "/foo/"), "/foo/rel/a/tive");
    TEST_EQUAL(r_r_p("rel/a/tive", "foo/bar"), "foo/rel/a/tive");
    TEST_EQUAL(r_r_p("rel/a/tive", "foo/bar/"), "foo/bar/rel/a/tive");
    TEST_EQUAL(r_r_p("rel/a/tive", "/foo/bar"), "/foo/rel/a/tive");
    TEST_EQUAL(r_r_p("rel/a/tive", "/foo/bar/"), "/foo/bar/rel/a/tive");
#ifndef __WIN32__
    TEST_EQUAL(r_r_p("/abs/o/lute", "/foo\\bar"), "/abs/o/lute");
    TEST_EQUAL(r_r_p("rel/a/tive", "/foo\\bar"), "/rel/a/tive");
#else
    TEST_EQUAL(r_r_p("\\dos\\path", ""), "\\dos\\path");
    TEST_EQUAL(r_r_p("\\dos\\path", "/"), "\\dos\\path");
    TEST_EQUAL(r_r_p("\\dos\\path", "\\"), "\\dos\\path");
    TEST_EQUAL(r_r_p("\\dos\\path", "c:"), "c:\\dos\\path");
    TEST_EQUAL(r_r_p("\\dos\\path", "c:\\"), "c:\\dos\\path");
    TEST_EQUAL(r_r_p("\\dos\\path", "c:\\temp"), "c:\\dos\\path");
    TEST_EQUAL(r_r_p("\\dos\\path", "c:\\temp\\"), "c:\\dos\\path");
    TEST_EQUAL(r_r_p("rel/a/tive", "\\"), "\\rel/a/tive");
    TEST_EQUAL(r_r_p("rel/a/tive", "foo\\"), "foo\\rel/a/tive");
    TEST_EQUAL(r_r_p("rel\\a\\tive", "/foo/"), "/foo/rel\\a\\tive");
    TEST_EQUAL(r_r_p("rel/a/tive", "c:/foo/bar"), "c:/foo/rel/a/tive");
    TEST_EQUAL(r_r_p("rel/a/tive", "c:foo/bar/"), "c:foo/bar/rel/a/tive");
    TEST_EQUAL(r_r_p("rel/a/tive", "c:"), "c:rel/a/tive");
    TEST_EQUAL(r_r_p("rel/a/tive", "c:\\"), "c:\\rel/a/tive");
    TEST_EQUAL(r_r_p("C:rel/a/tive", "c:\\foo\\bar"), "C:\\foo\\rel/a/tive");
    TEST_EQUAL(r_r_p("C:rel/a/tive", "c:"), "C:rel/a/tive");
    // This one is impossible to reliably resolve without knowing the current
    // drive - if it is C:, then the answer is: "C:/abs/o/rel/a/tive"
    TEST_EQUAL(r_r_p("C:rel/a/tive", "/abs/o/lute"), "C:rel/a/tive");
    // UNC paths tests:
    TEST_EQUAL(r_r_p("\\\\SRV\\VOL\\FILE", "/a/b"), "\\\\SRV\\VOL\\FILE");
    TEST_EQUAL(r_r_p("rel/a/tive", "\\\\SRV\\VOL\\DIR\\FILE"), "\\\\SRV\\VOL\\DIR\\rel/a/tive");
    TEST_EQUAL(r_r_p("/abs/o/lute", "\\\\SRV\\VOL\\FILE"), "\\\\SRV\\VOL/abs/o/lute");
    TEST_EQUAL(r_r_p("/abs/o/lute", "\\\\S\\V\\FILE"), "\\\\S\\V/abs/o/lute");
    TEST_EQUAL(r_r_p("/abs/o/lute", "\\\\S\\V\\"), "\\\\S\\V/abs/o/lute");
    TEST_EQUAL(r_r_p("/abs/o/lute", "\\\\S\\V"), "\\\\S\\V/abs/o/lute");
    TEST_EQUAL(r_r_p("//SRV/VOL/FILE", "/a/b"), "//SRV/VOL/FILE");
    TEST_EQUAL(r_r_p("rel/a/tive", "//SRV/VOL/DIR/FILE"), "//SRV/VOL/DIR/rel/a/tive");
    TEST_EQUAL(r_r_p("/abs/o/lute", "//SRV/VOL/FILE"), "//SRV/VOL/abs/o/lute");
    TEST_EQUAL(r_r_p("/abs/o/lute", "//S/V/FILE"), "//S/V/abs/o/lute");
    TEST_EQUAL(r_r_p("/abs/o/lute", "//S/V/"), "//S/V/abs/o/lute");
    TEST_EQUAL(r_r_p("/abs/o/lute", "//S/V"), "//S/V/abs/o/lute");
    TEST_EQUAL(r_r_p("/abs/o/lute", "\\\\?\\C:\\wibble"), "\\\\?\\C:\\abs\\o\\lute");
    TEST_EQUAL(r_r_p("/abs/o/lute", "\\\\?\\UNC\\S\\V"), "\\\\?\\UNC\\S\\V\\abs\\o\\lute");
    TEST_EQUAL(r_r_p("/abs/o/lute", "\\\\?\\UNC\\S\\V\\"), "\\\\?\\UNC\\S\\V\\abs\\o\\lute");
    TEST_EQUAL(r_r_p("/abs/o/lute", "\\\\?\\UNC\\S\\V\\TMP\\README.TXT"), "\\\\?\\UNC\\S\\V\\abs\\o\\lute");
    TEST_EQUAL(r_r_p("r/elativ/e", "\\\\?\\C:\\wibble"), "\\\\?\\C:\\r\\elativ\\e");
    TEST_EQUAL(r_r_p("r/elativ/e", "\\\\?\\C:\\wibble\\wobble"), "\\\\?\\C:\\wibble\\r\\elativ\\e");
#if 0 // Is this a valid testcase?  It fails, but isn't relevant to Xapian.
    TEST_EQUAL(r_r_p("r/elativ/e", "\\\\?\\UNC\\S\\V"), "\\\\?\\UNC\\S\\V\\r\\elativ\\e");
#endif
    TEST_EQUAL(r_r_p("r/elativ/e", "\\\\?\\UNC\\S\\V\\"), "\\\\?\\UNC\\S\\V\\r\\elativ\\e");
    TEST_EQUAL(r_r_p("r/elativ/e", "\\\\?\\UNC\\S\\V\\TMP\\README.TXT"), "\\\\?\\UNC\\S\\V\\TMP\\r\\elativ\\e");
#endif
    return true;
}

static void
check_double_serialisation(double u)
{
    string encoded = serialise_double(u);
    const char * ptr = encoded.data();
    const char * end = ptr + encoded.size();
    double v = unserialise_double(&ptr, end);
    if (ptr != end || u != v) {
	cout << u << " -> " << v << ", difference = " << v - u << endl;
	cout << "FLT_RADIX = " << FLT_RADIX << endl;
	cout << "DBL_MAX_EXP = " << DBL_MAX_EXP << endl;
    }
    TEST_EQUAL(static_cast<const void*>(ptr), static_cast<const void*>(end));
}

// Check serialisation of doubles.
DEFINE_TESTCASE_(serialisedouble1) {
    static const double test_values[] = {
	3.14159265,
	1e57,
	123.1,
	257.12,
	1234.567e123,
	255.5,
	256.125,
	257.03125,
    };

    check_double_serialisation(0.0);
    check_double_serialisation(1.0);
    check_double_serialisation(-1.0);
    check_double_serialisation(DBL_MAX);
    check_double_serialisation(-DBL_MAX);
    check_double_serialisation(DBL_MIN);
    check_double_serialisation(-DBL_MIN);

    const double *p;
    for (p = test_values; p < test_values + sizeof(test_values) / sizeof(double); ++p) {
	double val = *p;
	check_double_serialisation(val);
	check_double_serialisation(-val);
	check_double_serialisation(1.0 / val);
	check_double_serialisation(-1.0 / val);
    }

    return true;
}

#ifdef XAPIAN_HAS_REMOTE_BACKEND
// Check serialisation of lengths.
static bool test_serialiselength1()
{
    size_t n = 0;
    while (n < 0xff000000) {
	string s = encode_length(n);
	const char *p = s.data();
	const char *p_end = p + s.size();
	size_t decoded_n = decode_length(&p, p_end, false);
	if (n != decoded_n || p != p_end) tout << "[" << s << "]" << endl;
	TEST_EQUAL(n, decoded_n);
	TEST_EQUAL(p_end - p, 0);
	if (n < 5000) {
	    ++n;
	} else {
	    n += 53643;
	}
    }

    return true;
}

// Regression test: vetting the remaining buffer length
static bool test_serialiselength2()
{
    // Special case tests for 0
    {
	string s = encode_length(0);
	{
	    const char *p = s.data();
	    const char *p_end = p + s.size();
	    TEST(decode_length(&p, p_end, true) == 0);
	    TEST(p == p_end);
	}
	s += 'x';
	{
	    const char *p = s.data();
	    const char *p_end = p + s.size();
	    TEST(decode_length(&p, p_end, true) == 0);
	    TEST_EQUAL(p_end - p, 1);
	}
    }
    // Special case tests for 1
    {
	string s = encode_length(1);
	TEST_EXCEPTION(Xapian_NetworkError,
	    const char *p = s.data();
	    const char *p_end = p + s.size();
	    (void)decode_length(&p, p_end, true);
	);
	s += 'x';
	{
	    const char *p = s.data();
	    const char *p_end = p + s.size();
	    TEST(decode_length(&p, p_end, true) == 1);
	    TEST_EQUAL(p_end - p, 1);
	}
	s += 'x';
	{
	    const char *p = s.data();
	    const char *p_end = p + s.size();
	    TEST(decode_length(&p, p_end, true) == 1);
	    TEST_EQUAL(p_end - p, 2);
	}
    }
    // Nothing magic here, just test a range of odd and even values.
    for (size_t n = 2; n < 1000; n = (n + 1) * 2 + (n >> 1)) {
	string s = encode_length(n);
	TEST_EXCEPTION(Xapian_NetworkError,
	    const char *p = s.data();
	    const char *p_end = p + s.size();
	    (void)decode_length(&p, p_end, true);
	);
	s.append(n - 1, 'x');
	TEST_EXCEPTION(Xapian_NetworkError,
	    const char *p = s.data();
	    const char *p_end = p + s.size();
	    (void)decode_length(&p, p_end, true);
	);
	s += 'x';
	{
	    const char *p = s.data();
	    const char *p_end = p + s.size();
	    TEST(decode_length(&p, p_end, true) == n);
	    TEST_EQUAL(size_t(p_end - p), n);
	}
	s += 'x';
	{
	    const char *p = s.data();
	    const char *p_end = p + s.size();
	    TEST(decode_length(&p, p_end, true) == n);
	    TEST_EQUAL(size_t(p_end - p), n + 1);
	}
    }

    return true;
}
#endif

static const test_desc tests[] = {
    TESTCASE(resolverelativepath1),
    TESTCASE(serialisedouble1),
#ifdef XAPIAN_HAS_REMOTE_BACKEND
    TESTCASE(serialiselength1),
    TESTCASE(serialiselength2),
#endif
    END_OF_TESTCASES
};

int main(int argc, char **argv)
try {
    test_driver::parse_command_line(argc, argv);
    return test_driver::run(tests);
} catch (const char * e) {
    cout << e << endl;
    return 1;
}
