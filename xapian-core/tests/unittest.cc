/** @file unittest.cc
 * @brief Unit tests of non-Xapian-specific internal code.
 */
/* Copyright (C) 2006,2007,2009,2010,2012,2015,2016,2017,2018,2019 Olly Betts
 * Copyright (C) 2007 Richard Boulton
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

#include <cctype>
#include <cerrno>
#include <cfloat>
#include <climits>
#include <cmath>
#include <cstring>
#include <iostream>
#include <limits>
#include <utility>

#include "safeunistd.h"

#define XAPIAN_UNITTEST
static const char * unittest_assertion_failed = NULL;
#define UNITTEST_CHECK_EXCEPTION \
    if (unittest_assertion_failed) { \
	const char * unittest_assertion_failed_ = unittest_assertion_failed;\
	unittest_assertion_failed = NULL;\
	throw unittest_assertion_failed_;\
    }

#include "testsuite.h"

using namespace std;

#define UNITTEST_ASSERT_LOCATION__(LINE,MSG) __FILE__":"#LINE": "#MSG
#define UNITTEST_ASSERT_LOCATION_(LINE,MSG) UNITTEST_ASSERT_LOCATION__(LINE,MSG)
#define UNITTEST_ASSERT_LOCATION(MSG) UNITTEST_ASSERT_LOCATION_(__LINE__,MSG)
#define UNITTEST_ASSERT_NOTHROW(COND, RET) \
    do {\
	if (rare(!(COND))) {\
	    unittest_assertion_failed = UNITTEST_ASSERT_LOCATION(COND);\
	    return RET;\
	}\
    } while (false)

// Utility code we use:
#include "../backends/multi.h"
#include "../common/stringutils.h"
#include "../common/log2.h"

// Simpler version of TEST_EXCEPTION macro.
#define TEST_EXCEPTION(TYPE, CODE) \
    do { \
	try { \
	    CODE; \
	    UNITTEST_CHECK_EXCEPTION \
	    FAIL_TEST("Expected exception "#TYPE" not thrown"); \
	} catch (const TYPE &) { \
	} \
    } while (0)

// Code we're unit testing:
#include "../common/closefrom.cc"
#include "../common/errno_to_string.cc"
#include "../common/fileutils.cc"
#include "../common/overflow.h"
#include "../common/parseint.h"
#include "../common/serialise-double.cc"
#include "../common/str.cc"
#include "../backends/uuids.cc"
#include "../net/length.cc"
#include "../net/serialise-error.cc"
#include "../api/error.cc"
#include "../api/sortable-serialise.cc"
#include "../include/xapian/intrusive_ptr.h"

// fileutils.cc uses opendir(), etc though not in a function we currently test.
#include "../common/msvc_dirent.cc"

// The UUID code uses hexdigit().
#include "../api/constinfo.cc"

// Stub replacement, which doesn't deal with escaping or producing valid UTF-8.
// The full implementation needs Xapian::Utf8Iterator and
// Xapian::Unicode::append_utf8().
void
description_append(std::string & desc, const std::string &s)
{
    desc += s;
}

DEFINE_TESTCASE_(simple_exceptions_work1) {
    try {
	throw 42;
    } catch (int val) {
	TEST_EQUAL(val, 42);
	return true;
    }
    return false;
}

class TestException { };

DEFINE_TESTCASE_(class_exceptions_work1) {
    try {
	throw TestException();
    } catch (const TestException &) {
	return true;
    }
    return false;
}

static inline string
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
    // Commonly C++ string implementations keep the string nul-terminated, and
    // encoded.data() returns a pointer to a buffer including the nul (the same
    // as encoded.c_str()).  This means that valgrind won't catch a read one
    // past the end of the serialised value, so we copy just the serialised
    // value into a temporary buffer.
    char buf[16];
    string encoded = serialise_double(u);
    TEST(encoded.size() < sizeof(buf));
    memcpy(buf, encoded.data(), encoded.size());
    // Put a NULL pointer either side, to catch incrementing/decrementing at
    // the wrong level of indirection (regression test for a bug in an
    // unreleased version).
    const char * ptr[3] = { NULL, buf, NULL };
    const char * end = ptr[1] + encoded.size();
    double v = unserialise_double(&(ptr[1]), end);
    if (ptr[1] != end || u != v) {
	cout << u << " -> " << v << ", difference = " << v - u << endl;
	cout << "FLT_RADIX = " << FLT_RADIX << endl;
	cout << "DBL_MAX_EXP = " << DBL_MAX_EXP << endl;
    }
    TEST_EQUAL(static_cast<const void*>(ptr[1]), static_cast<const void*>(end));
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
	size_t decoded_n;
	decode_length(&p, p_end, decoded_n);
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
	    size_t r;
	    decode_length_and_check(&p, p_end, r);
	    TEST(r == 0);
	    TEST(p == p_end);
	}
	s += 'x';
	{
	    const char *p = s.data();
	    const char *p_end = p + s.size();
	    size_t r;
	    decode_length_and_check(&p, p_end, r);
	    TEST(r == 0);
	    TEST_EQUAL(p_end - p, 1);
	}
    }
    // Special case tests for 1
    {
	string s = encode_length(1);
	TEST_EXCEPTION(Xapian_NetworkError,
	    const char *p = s.data();
	    const char *p_end = p + s.size();
	    size_t r;
	    decode_length_and_check(&p, p_end, r);
	    (void)r;
	);
	s += 'x';
	{
	    const char *p = s.data();
	    const char *p_end = p + s.size();
	    size_t r;
	    decode_length_and_check(&p, p_end, r);
	    TEST(r == 1);
	    TEST_EQUAL(p_end - p, 1);
	}
	s += 'x';
	{
	    const char *p = s.data();
	    const char *p_end = p + s.size();
	    size_t r;
	    decode_length_and_check(&p, p_end, r);
	    TEST(r == 1);
	    TEST_EQUAL(p_end - p, 2);
	}
    }
    // Nothing magic here, just test a range of odd and even values.
    for (size_t n = 2; n < 1000; n = (n + 1) * 2 + (n >> 1)) {
	string s = encode_length(n);
	TEST_EXCEPTION(Xapian_NetworkError,
	    const char *p = s.data();
	    const char *p_end = p + s.size();
	    size_t r;
	    decode_length_and_check(&p, p_end, r);
	    (void)r;
	);
	s.append(n - 1, 'x');
	TEST_EXCEPTION(Xapian_NetworkError,
	    const char *p = s.data();
	    const char *p_end = p + s.size();
	    size_t r;
	    decode_length_and_check(&p, p_end, r);
	    (void)r;
	);
	s += 'x';
	{
	    const char *p = s.data();
	    const char *p_end = p + s.size();
	    size_t r;
	    decode_length_and_check(&p, p_end, r);
	    TEST(r == n);
	    TEST_EQUAL(size_t(p_end - p), n);
	}
	s += 'x';
	{
	    const char *p = s.data();
	    const char *p_end = p + s.size();
	    size_t r;
	    decode_length_and_check(&p, p_end, r);
	    TEST(r == n);
	    TEST_EQUAL(size_t(p_end - p), n + 1);
	}
    }

    return true;
}

// Check serialisation of Xapian::Error.
static bool test_serialiseerror1()
{
    string enoent_msg = errno_to_string(ENOENT);
    Xapian::DatabaseOpeningError e("Failed to open database", ENOENT);
    // Regression test for bug in 1.0.0 - it didn't convert errno values for
    // get_description() if they hadn't already been converted.
    TEST_STRINGS_EQUAL(e.get_description(), "DatabaseOpeningError: Failed to open database (" + enoent_msg + ")");

    TEST_STRINGS_EQUAL(e.get_error_string(), enoent_msg);

    string serialisation = serialise_error(e);

    // Test if unserialise_error() throws with a flag to avoid the possibility
    // of an "unreachable code" warning when we get around to marking
    // unserialise_error() as "noreturn".
    bool threw = false;
    try {
	// unserialise_error throws an exception.
	unserialise_error(serialisation, "", "");
    } catch (const Xapian::Error & ecaught) {
	TEST_STRINGS_EQUAL(ecaught.get_error_string(), enoent_msg);
	threw = true;
    }
    TEST(threw);

    // Check that the original is still OK.
    TEST_STRINGS_EQUAL(e.get_error_string(), enoent_msg);

    // Regression test - in 1.0.0, copying used to duplicate the error_string
    // pointer, resulting in double calls to free().
    Xapian::DatabaseOpeningError ecopy(e);
    TEST_STRINGS_EQUAL(ecopy.get_error_string(), enoent_msg);

    return true;
}
#endif

// Test log2() (which might be our replacement version).
static bool test_log2()
{
    TEST_EQUAL(log2(1.0), 0.0);
    TEST_EQUAL(log2(2.0), 1.0);
    TEST_EQUAL(log2(1024.0), 10.0);
    TEST_EQUAL(log2(0.5), -1.0);
    return true;
}

static const double test_sortableserialise_numbers[] = {
#ifdef INFINITY
    -INFINITY,
#endif
    -HUGE_VAL,
    -DBL_MAX,
    -exp2(1022),
    -1024.5,
    -3.14159265358979323846,
    -3,
    -2,
    -1.8,
    -1.1,
    -1,
    -0.5,
    -0.2,
    -0.1,
    -0.000005,
    -0.000002,
    -0.000001,
    -exp2(-1023),
    -exp2(-1024),
    -exp2(-1074),
    -DBL_MIN,
    0,
    DBL_MIN,
    exp2(-1074),
    exp2(-1024),
    exp2(-1023),
    0.000001,
    0.000002,
    0.000005,
    0.1,
    0.2,
    0.5,
    1,
    1.1,
    1.8,
    2,
    3,
    3.14159265358979323846,
    1024.5,
    exp2(1022),
    DBL_MAX,
    HUGE_VAL,
#ifdef INFINITY
    INFINITY,
#endif

    64 // Magic number which we stop at.
};

// Test serialisation and unserialisation of various numbers.
// This is actually a public API, but we want extra assertions in the code
// while we test it.
static bool test_sortableserialise1()
{
    double prevnum = 0;
    string prevstr;
    bool started = false;
    for (const double *p = test_sortableserialise_numbers; *p != 64; ++p) {
	double num = *p;
	tout << "Number: " << num << '\n';
	string str = Xapian::sortable_serialise(num);
	tout << "String: " << str << '\n';
	TEST_EQUAL(Xapian::sortable_unserialise(str), num);

	if (started) {
	    int num_cmp = 0;
	    if (prevnum < num) {
		num_cmp = -1;
	    } else if (prevnum > num) {
		num_cmp = 1;
	    }
	    int str_cmp = 0;
	    if (prevstr < str) {
		str_cmp = -1;
	    } else if (prevstr > str) {
		str_cmp = 1;
	    }

	    TEST_AND_EXPLAIN(num_cmp == str_cmp,
			     "Numbers " << prevnum << " and " << num <<
			     " don't sort the same way as their string "
			     "counterparts");
	}

	prevnum = num;
	prevstr = str;
	started = true;
    }
    return true;
}

static bool test_tostring1()
{
    TEST_EQUAL(str(0), "0");
    TEST_EQUAL(str(0u), "0");
    TEST_EQUAL(str(1), "1");
    TEST_EQUAL(str(1u), "1");
    TEST_EQUAL(str(9), "9");
    TEST_EQUAL(str(9u), "9");
    TEST_EQUAL(str(10), "10");
    TEST_EQUAL(str(10u), "10");
    TEST_EQUAL(str(-1), "-1");
    TEST_EQUAL(str(-9), "-9");
    TEST_EQUAL(str(-10), "-10");
    TEST_EQUAL(str(0xffffffff), "4294967295");
    TEST_EQUAL(str(0x7fffffff), "2147483647");
    TEST_EQUAL(str(0x7fffffffu), "2147483647");
    TEST_EQUAL(str(-0x7fffffff), "-2147483647");

#ifdef __WIN32__
    /* Test the 64 bit integer conversion to string.
     * (Currently only exists for windows.)
     */
    TEST_EQUAL(str(10ll), "10");
    TEST_EQUAL(str(-10ll), "-10");
    TEST_EQUAL(str(0x200000000ll), "8589934592");
// We don't currently have an "unsigned long long" version since it's not required
// anywhere in the library.
//    TEST_EQUAL(str(0x200000000ull), "8589934592");
#endif

    return true;
}

/// Regression test for bug fixed in 1.1.1.
static bool test_strbool1()
{
    TEST_EQUAL(str(true), "1");
    TEST_EQUAL(str(false), "0");
    return true;
}

static bool test_closefrom1()
{
#ifndef __WIN32__
    // Simple test.
    closefrom(8);

    // Simple test when there are definitely no fds to close.
    closefrom(42);

    // Test passing a really high threshold.
    closefrom(INT_MAX);

    // Open some fds and check the expected ones are closed.
    TEST_EQUAL(dup2(1, 14), 14);
    TEST_EQUAL(dup2(1, 15), 15);
    TEST_EQUAL(dup2(1, 18), 18);
    closefrom(15);
    TEST_EQUAL(close(14), 0);
    TEST(close(15) == -1 && errno == EBADF);
    TEST(close(18) == -1 && errno == EBADF);
#endif
    return true;
}

static bool test_shard1()
{
    for (Xapian::docid did = 1; did != 10; ++did) {
	for (size_t n = 1; n != 10; ++n) {
	    Xapian::docid s_did = shard_docid(did, n);
	    size_t shard = shard_number(did, n);
	    TEST_EQUAL(s_did, (did - 1) / n + 1);
	    TEST_EQUAL(shard, (did - 1) % n);
	    if (n == 1)
		TEST_EQUAL(did, s_did);
	    if (did == 1)
		TEST_EQUAL(s_did, 1);
	    if (s_did == 1)
		TEST(did <= n);
	    TEST(s_did != 0);
	    TEST(s_did <= did);
	    TEST(shard < n);
	    TEST_EQUAL(did, unshard(s_did, shard, n));
	}
    }
    return true;
}

static bool test_uuid1()
{
    Uuid uuid, uuid2;

    // Test a generated uuid.
    uuid.generate();
    TEST(!uuid.is_null());
    string str = uuid.to_string();
    TEST_EQUAL(str.size(), 36);
    TEST_NOT_EQUAL(str, "00000000-0000-0000-0000-000000000000");
    // Check UUID pattern is correct and that upper case is not used.
    for (int i = 0; i != 8; ++i) {
	unsigned char ch = str[i];
	TEST(isxdigit(ch));
	TEST(!isupper(ch));
    }
    TEST_EQUAL(str[8], '-');
    for (int i = 9; i != 13; ++i) {
	unsigned char ch = str[i];
	TEST(isxdigit(ch));
	TEST(!isupper(ch));
    }
    TEST_EQUAL(str[13], '-');
    for (int i = 14; i != 18; ++i) {
	unsigned char ch = str[i];
	TEST(isxdigit(ch));
	TEST(!isupper(ch));
    }
    TEST_EQUAL(str[18], '-');
    for (int i = 19; i != 23; ++i) {
	unsigned char ch = str[i];
	TEST(isxdigit(ch));
	TEST(!isupper(ch));
    }
    TEST_EQUAL(str[23], '-');
    for (int i = 24; i != 36; ++i) {
	unsigned char ch = str[i];
	TEST(isxdigit(ch));
	TEST(!isupper(ch));
    }

    uuid2.parse(str);
    TEST(memcmp(uuid.data(), uuid2.data(), uuid.BINARY_SIZE) == 0);

    // Check the variant is "10x" and the version between 1 and 5.  Mostly this
    // is to catch bugs where the platform's API for generating UUIDs uses a
    // different endianness for fields (which we've run into under both WIN32
    // and FreeBSD).
    TEST_EQUAL(uuid.data()[8] & 0xc0, 0x80);
    TEST_REL(str[19], >=, '8');
    TEST_REL(str[19], <=, 'b');
    TEST_REL(int(uuid.data()[6]), >=, 0x10);
    TEST_REL(int(uuid.data()[6]), <=, 0x5f);
    TEST_REL(str[14], >=, '1');
    TEST_REL(str[14], <=, '5');

    // Test generating another uuid gives us a different non-null uuid.
    uuid2.generate();
    TEST(!uuid2.is_null());
    TEST(memcmp(uuid.data(), uuid2.data(), uuid.BINARY_SIZE) != 0);

    // Test null uuid.
    uuid.clear();
    TEST(uuid.is_null());
    str = uuid.to_string();
    TEST_EQUAL(str, "00000000-0000-0000-0000-000000000000");
    uuid2.generate();
    TEST(!uuid2.is_null());
    uuid2.parse(str);
    TEST(memcmp(uuid.data(), uuid2.data(), uuid.BINARY_SIZE) == 0);

    return true;
}

// Classes used by movesupport1 test
class A : public Xapian::Internal::intrusive_base {
    int x = 0;
  public:
    explicit A(int x_) : x(x_) {}

    int get_x() const {
	return x;
    }
};

class B : public Xapian::Internal::opt_intrusive_base {
    int x = 0;
    bool & alive;
  public:
    B(int x_, bool & alive_) : x(x_), alive(alive_) {
	alive = true;
    }

    ~B() {
	alive = false;
    }

    int get_x() const {
	return x;
    }

    B * release() {
	opt_intrusive_base::release();
	return this;
    }
};

static bool test_movesupport1()
{
    {
	// Test move semantics support for intrusive_ptr class
	Xapian::Internal::intrusive_ptr<A> p1(new A{5});
	Xapian::Internal::intrusive_ptr<A> p3;

	// Test move constructor
	Xapian::Internal::intrusive_ptr<A> p2(std::move(p1));
	TEST_EQUAL(p2->get_x(), 5);
	TEST_EQUAL(p1.get(), 0);

	// Test move assignment
	p3 = std::move(p2);
	TEST_EQUAL(p3->get_x(), 5);
	TEST_EQUAL(p2.get(), 0);
    }

    {
	// Same test for intrusive_ptr_nonnull class
	Xapian::Internal::intrusive_ptr_nonnull<A> p1(new A{5});
	Xapian::Internal::intrusive_ptr_nonnull<A> p3(new A{6});

	// Test move constructor
	Xapian::Internal::intrusive_ptr_nonnull<A> p2(std::move(p1));
	TEST_EQUAL(p2->get_x(), 5);

	// Test move assignment
	p3 = std::move(p2);
	TEST_EQUAL(p3->get_x(), 5);
    }

    bool alive = false;
    {
	// Same test for opt_intrusive_ptr class
	B * b1 = new B{5, alive};
	b1->release();
	Xapian::Internal::opt_intrusive_ptr<B> p1(b1);
	Xapian::Internal::opt_intrusive_ptr<B> p3;

	// Test move constructor
	Xapian::Internal::opt_intrusive_ptr<B> p2(std::move(p1));
	TEST_EQUAL(p2->get_x(), 5);
	TEST_EQUAL(p1.get(), 0);
	TEST_EQUAL(alive, true);

	// Test move assignment
	p3 = std::move(p2);
	TEST_EQUAL(p3->get_x(), 5);
	TEST_EQUAL(p2.get(), 0);
	TEST_EQUAL(alive, true);
    }
    // Test that object b1 has been deleted.
    TEST_EQUAL(alive, false);
    return true;
}

static bool test_addoverflows1()
{
    unsigned long res;
    TEST(!add_overflows(0UL, 0UL, res));
    TEST_EQUAL(res, 0);

    TEST(add_overflows(ULONG_MAX, 1UL, res));
    TEST_EQUAL(res, 0);

    TEST(add_overflows(1UL, ULONG_MAX, res));
    TEST_EQUAL(res, 0);

    TEST(add_overflows(ULONG_MAX, ULONG_MAX, res));
    TEST_EQUAL(res, ULONG_MAX - 1UL);

    return true;
}

static bool test_muloverflows1()
{
    unsigned long res;
    TEST(!mul_overflows(0UL, 0UL, res));
    TEST_EQUAL(res, 0);

    TEST(!mul_overflows(ULONG_MAX, 0UL, res));
    TEST_EQUAL(res, 0);

    TEST(!mul_overflows(0UL, ULONG_MAX, res));
    TEST_EQUAL(res, 0);

    TEST(!mul_overflows(ULONG_MAX, 1UL, res));
    TEST_EQUAL(res, ULONG_MAX);

    TEST(!mul_overflows(1UL, ULONG_MAX, res));
    TEST_EQUAL(res, ULONG_MAX);

    TEST(mul_overflows((ULONG_MAX >> 1UL) + 1UL, 2UL, res));
    TEST_EQUAL(res, 0);

    TEST(mul_overflows(2UL, (ULONG_MAX >> 1UL) + 1UL, res));
    TEST_EQUAL(res, 0);

    TEST(mul_overflows(ULONG_MAX, ULONG_MAX, res));

    return true;
}

template<typename U>
inline static void parseunsigned_helper() {
    U val;
    const U max_val = numeric_limits<U>::max();
    tout << "Testing with parseunsigned_helper" << endl;
    TEST(parse_unsigned("0", val));
    TEST_EQUAL(val, 0);
    TEST(parse_unsigned("99", val));
    TEST_EQUAL(val, 99);
    TEST(parse_unsigned(str(max_val).c_str(), val));
    TEST_EQUAL(val, max_val);
    TEST(!parse_unsigned("", val));
    TEST(!parse_unsigned("-1", val));
    TEST(!parse_unsigned("abc", val));
    TEST(!parse_unsigned("0a", val));
    // Only test if we can construct a value one larger easily.
    if (max_val + 1ull != 0)
	TEST(!parse_unsigned(str(max_val + 1ull).c_str(), val));
}

static bool test_parseunsigned1()
{
    parseunsigned_helper<unsigned char>();
    parseunsigned_helper<unsigned short>();
    parseunsigned_helper<unsigned>();
    parseunsigned_helper<unsigned long>();
    parseunsigned_helper<unsigned long long>();

    return true;
}

static const test_desc tests[] = {
    TESTCASE(simple_exceptions_work1),
    TESTCASE(class_exceptions_work1),
    TESTCASE(resolverelativepath1),
    TESTCASE(serialisedouble1),
#ifdef XAPIAN_HAS_REMOTE_BACKEND
    TESTCASE(serialiselength1),
    TESTCASE(serialiselength2),
    TESTCASE(serialiseerror1),
#endif
    TESTCASE(log2),
    TESTCASE(sortableserialise1),
    TESTCASE(tostring1),
    TESTCASE(strbool1),
    TESTCASE(closefrom1),
    TESTCASE(shard1),
    TESTCASE(uuid1),
    TESTCASE(movesupport1),
    TESTCASE(addoverflows1),
    TESTCASE(muloverflows1),
    TESTCASE(parseunsigned1),
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
