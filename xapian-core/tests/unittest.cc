/** @file
 * @brief Unit tests of non-Xapian-specific internal code.
 */
/* Copyright (C) 2006-2024 Olly Betts
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
#include "../common/io_utils.cc"
#include "../common/fileutils.cc"
#include "../common/overflow.h"
#include "../common/parseint.h"
#include "../common/posixy_wrapper.cc"
#include "../common/serialise-double.cc"
#include "../common/str.cc"
#include "../backends/uuids.cc"
#include "../net/length.cc"
#include "../net/serialise-error.cc"
#include "../api/error.cc"
#include "../api/sortable-serialise.cc"

// fileutils.cc uses opendir(), etc though not in a function we currently test.
#include "../common/msvc_dirent.cc"

// The UUID code uses hex_decode().
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
    }
}

class TestException { };

DEFINE_TESTCASE_(class_exceptions_work1) {
    try {
	throw TestException();
    } catch (const TestException &) {
    }
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
	cout << u << " -> " << v << ", difference = " << v - u << '\n';
	cout << "FLT_RADIX = " << FLT_RADIX << '\n';
	cout << "DBL_MAX_EXP = " << DBL_MAX_EXP << '\n';
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
}

#ifdef XAPIAN_HAS_REMOTE_BACKEND
// Check serialisation of lengths.
static void test_serialiselength1()
{
    size_t n = 0;
    while (n < 0xff000000) {
	string s = encode_length(n);
	const char *p = s.data();
	const char *p_end = p + s.size();
	size_t decoded_n;
	decode_length(&p, p_end, decoded_n);
	if (n != decoded_n || p != p_end) tout << "[" << s << "]\n";
	TEST_EQUAL(n, decoded_n);
	TEST_EQUAL(p_end - p, 0);
	if (n < 5000) {
	    ++n;
	} else {
	    n += 53643;
	}
    }
}

// Regression test: vetting the remaining buffer length
static void test_serialiselength2()
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
}

// Check serialisation of Xapian::Error.
static void test_serialiseerror1()
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
}
#endif

// Test log2() (which might be our replacement version).
static void test_log2()
{
    TEST_EQUAL(log2(1.0), 0.0);
    TEST_EQUAL(log2(2.0), 1.0);
    TEST_EQUAL(log2(1024.0), 10.0);
    TEST_EQUAL(log2(0.5), -1.0);
}

static const double test_sortableserialise_numbers[] = {
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

    64 // Magic number which we stop at.
};

// Test serialisation and unserialisation of various numbers.
// This is actually a public API, but we want extra assertions in the code
// while we test it.
static void test_sortableserialise1()
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
}

template<typename S>
inline static void tostring_helper() {
    const S max_val = numeric_limits<S>::max();
    const S min_val = numeric_limits<S>::min();
    tout << "Testing with tostring_helper\n";
    std::ostringstream oss;
    oss << (long long)max_val;
    TEST_EQUAL(str(max_val), oss.str());
    oss.str("");
    oss.clear();

    oss << (long long)min_val;
    TEST_EQUAL(str(min_val), oss.str());
    oss.str("");
    oss.clear();
}

static void test_tostring1()
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
    TEST_EQUAL(str(0x7f), "127");
    TEST_EQUAL(str(-0x80), "-128");
    TEST_EQUAL(str(0x7fff), "32767");
    TEST_EQUAL(str(0xffffffff), "4294967295");
    TEST_EQUAL(str(0x7fffffff), "2147483647");
    TEST_EQUAL(str(0x7fffffffu), "2147483647");
    TEST_EQUAL(str(-0x7fffffff), "-2147483647");

    tostring_helper<char>();
    tostring_helper<short>();
    tostring_helper<int>();
    tostring_helper<long>();
    tostring_helper<long long>();

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
}

/// Regression test for bug fixed in 1.1.1.
static void test_strbool1()
{
    TEST_EQUAL(str(true), "1");
    TEST_EQUAL(str(false), "0");
}

static void test_closefrom1()
{
#ifndef __WIN32__
    // Simple test.  Start from 13 as on macOS the FDTracker seems to get fd
    // 10 and we don't want to collide with that.
    closefrom(13);

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
}

static void test_uuid1()
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

static void test_movesupport1()
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
}

static void test_addoverflows1()
{
    const auto ulong_max = numeric_limits<unsigned long>::max();
    const auto uint_max = numeric_limits<unsigned int>::max();
    const auto ushort_max = numeric_limits<unsigned short>::max();
    const auto uchar_max = numeric_limits<unsigned char>::max();

    unsigned long res_ulong;
    unsigned res_uint;
    unsigned short res_ushort;
    unsigned char res_uchar;

    TEST(!add_overflows(0UL, 0UL, res_ulong));
    TEST_EQUAL(res_ulong, 0);
    TEST(!add_overflows(0UL, 0UL, res_uint));
    TEST_EQUAL(res_uint, 0);
    TEST(!add_overflows(0UL, 0UL, res_ushort));
    TEST_EQUAL(res_ushort, 0);
    TEST(!add_overflows(0UL, 0UL, res_uchar));
    TEST_EQUAL(res_uchar, 0);

    TEST(add_overflows(ulong_max, 1UL, res_ulong));
    TEST_EQUAL(res_ulong, 0);
    TEST(add_overflows(uint_max, 1UL, res_uint));
    TEST_EQUAL(res_uint, 0);
    TEST(add_overflows(ushort_max, 1UL, res_ushort));
    TEST_EQUAL(res_ushort, 0);
    TEST(add_overflows(uchar_max, 1UL, res_uchar));
    TEST_EQUAL(res_uchar, 0);

    TEST(add_overflows(1UL, ulong_max, res_ulong));
    TEST_EQUAL(res_ulong, 0);
    TEST(add_overflows(1UL, uint_max, res_uint));
    TEST_EQUAL(res_uint, 0);
    TEST(add_overflows(1UL, ushort_max, res_ushort));
    TEST_EQUAL(res_ushort, 0);
    TEST(add_overflows(1UL, uchar_max, res_uchar));
    TEST_EQUAL(res_uchar, 0);

    TEST(add_overflows(ulong_max, ulong_max, res_ulong));
    TEST_EQUAL(res_ulong, ulong_max - 1UL);
    TEST(add_overflows(uint_max, uint_max, res_uint));
    TEST_EQUAL(res_uint, uint_max - 1UL);
    TEST(add_overflows(ushort_max, ushort_max, res_ushort));
    TEST_EQUAL(res_ushort, ushort_max - 1UL);
    TEST(add_overflows(uchar_max, uchar_max, res_uchar));
    TEST_EQUAL(res_uchar, uchar_max - 1UL);

    res_uchar = 1;
    TEST(add_overflows(res_uchar, unsigned(uchar_max) + 1U, res_uchar));
    TEST_EQUAL(res_uchar, 1);
}

static void test_suboverflows1()
{
    unsigned long res;
    TEST(!sub_overflows(0UL, 0UL, res));
    TEST_EQUAL(res, 0);

    TEST(sub_overflows(0UL, 1UL, res));
    TEST_EQUAL(res, ULONG_MAX);

    TEST(sub_overflows(ULONG_MAX - 1UL, ULONG_MAX, res));
    TEST_EQUAL(res, ULONG_MAX);

    TEST(sub_overflows(0UL, ULONG_MAX, res));
    TEST_EQUAL(res, 1);
}

static void test_muloverflows1()
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
}

template<typename U>
inline static void parseunsigned_helper() {
    U val;
    const U max_val = numeric_limits<U>::max();
    tout << "Testing with parseunsigned_helper\n";
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

static void test_parseunsigned1()
{
    parseunsigned_helper<unsigned char>();
    parseunsigned_helper<unsigned short>();
    parseunsigned_helper<unsigned>();
    parseunsigned_helper<unsigned long>();
    parseunsigned_helper<unsigned long long>();
}

template<typename S>
inline static void parsesigned_helper() {
    S val;
    const S max_val = numeric_limits<S>::max();
    const S min_val = numeric_limits<S>::min();
    tout << "Testing with parsesigned_helper\n";
    TEST(parse_signed("0", val));
    TEST_EQUAL(val, 0);
    TEST(parse_signed("99", val));
    TEST_EQUAL(val, 99);
    TEST(parse_signed("-99", val));
    TEST_EQUAL(val, -99);
    TEST(parse_signed(str(max_val).c_str(), val));
    TEST_EQUAL(val, max_val);
    TEST(parse_signed(str(min_val).c_str(), val));
    TEST_EQUAL(val, min_val);
    TEST(!parse_signed("", val));
    TEST(!parse_signed("abc", val));
    TEST(!parse_signed("0a", val));
    TEST(!parse_signed("-99a", val));
    TEST(!parse_signed("-a99", val));
    TEST(!parse_signed("--99", val));

    unsigned long long one_too_large = max_val + 1ull;
    TEST(!parse_signed(str(one_too_large).c_str(), val));

    unsigned long long one_too_small_negated = 1ull - min_val;
    TEST(!parse_signed(("-" + str(one_too_small_negated)).c_str(), val));
}

static void test_parsesigned1()
{
    parsesigned_helper<signed char>();
    parsesigned_helper<short>();
    parsesigned_helper<int>();
    parsesigned_helper<long>();
    parsesigned_helper<long long>();
}

/// Test working with a block-based file using functions from io_utils.h.
static void test_ioblock1()
try {
    const char* tmp_file = ".unittest_ioutils1";
    int fd = -1;
    try {
	constexpr int BLOCK_SIZE = 1024;

	fd = io_open_block_wr(tmp_file, true);
	TEST_REL(fd, >=, 0);

	string buf(BLOCK_SIZE, 'x');
	string out;

	// ZFS default blocksize is 128K so we need to write at least that far
	// into the file to be able to successfully detect support for sparse
	// files below.  We won't detect sparse file support if the blocksize
	// is larger, but that's not a problem.
	io_write_block(fd, buf.data(), BLOCK_SIZE, 128);
	out.resize(BLOCK_SIZE);
	io_read_block(fd, &out[0], BLOCK_SIZE, 128);
	TEST(buf == out);

	// Call io_sync() and check it claims to work.  Checking it actually has
	// any effect is much harder to do.
	TEST(io_sync(fd));

	io_write_block(fd, buf.data(), BLOCK_SIZE, 129);
	out.resize(BLOCK_SIZE);
	io_read_block(fd, &out[0], BLOCK_SIZE, 129);
	TEST(buf == out);

	// Call io_full_sync() and check it claims to work.  Checking it actually
	// has any effect is much harder to do.
	TEST(io_full_sync(fd));

	if (sizeof(off_t) <= 4) {
	    SKIP_TEST("Skipping rest of testcase - no Large File Support");
	}

#ifdef SEEK_HOLE
	struct stat statbuf;
	TEST(fstat(fd, &statbuf) == 0);

	off_t hole = lseek(fd, 0, SEEK_HOLE);
	if (hole < 0) {
	    SKIP_TEST("Skipping rest of testcase - SEEK_HOLE failed");
	}
	if (hole >= statbuf.st_size) {
	    SKIP_TEST("Skipping rest of testcase - sparse file support not "
		      "detected");
	}

	// Write a block at an offset a little above 4GB and check that we wrote
	// the specified block by checking the filesize.  This should catch bugs
	// which truncate the offset used.
	constexpr off_t high_offset = (off_t{1} << 32) + BLOCK_SIZE;
	constexpr off_t high_block = high_offset / BLOCK_SIZE;
	try {
	    io_write_block(fd, buf.data(), BLOCK_SIZE, high_block);
	} catch (const Xapian::DatabaseError& e) {
	    if (e.get_error_string() == errno_to_string(EFBIG))
		SKIP_TEST("Skipping rest of testcase - FS doesn't allow a > 4GB "
			  "file");
	    throw;
	}
	TEST(fstat(fd, &statbuf) == 0);
	TEST_EQUAL(statbuf.st_size, high_offset + BLOCK_SIZE);

	close(fd);

	fd = io_open_block_rd(tmp_file);

	// We can't easily test that io_readahead_block() actually does anything if
	// it returns true, but we can at least call it to check it doesn't crash.
	(void)io_readahead_block(fd, BLOCK_SIZE, high_block);

	// Check we can read back the same data we wrote.
	io_read_block(fd, &out[0], BLOCK_SIZE, high_block);
	TEST(buf == out);

	close(fd);
	fd = -1;
	io_unlink(tmp_file);
#else
	SKIP_TEST("Skipping rest of testcase - SEEK_HOLE not supported");
#endif
    } catch (...) {
	close(fd);
	io_unlink(tmp_file);
	throw;
    }
} catch (const Xapian::Error& e) {
    // Translate Xapian::Error exceptions to std::string exceptions which
    // utestsuite can catch.
    throw e.get_description();
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
    TESTCASE(uuid1),
    TESTCASE(movesupport1),
    TESTCASE(addoverflows1),
    TESTCASE(suboverflows1),
    TESTCASE(muloverflows1),
    TESTCASE(parseunsigned1),
    TESTCASE(parsesigned1),
    TESTCASE(ioblock1),
    END_OF_TESTCASES
};

int main(int argc, char **argv)
try {
    test_driver::parse_command_line(argc, argv);
    return test_driver::run(tests);
} catch (const char * e) {
    cout << e << '\n';
    return 1;
}
