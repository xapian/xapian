/** @file unittest.cc
 * @brief Unit tests of non-Xapian-specific internal code.
 */
/* Copyright (C) 2006,2010,2012 Olly Betts
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

using namespace std;

#include "../common/fileutils.cc"

#include "../common/serialise-double.cc"

// Currently the test harness drags in Xapian (for reporting Xapian::Error
// exceptions, backendmanager-related stuff, and maybe other things, so we
// can't use it here and have to make do with crude versions of what the test
// harness does much better.
// FIXME: Sort out the test harness so we can use it here.

#define TEST_EQ(A, B) do {\
    const string & test_equal_a = (A);\
    const string & test_equal_b = (B);\
    if (test_equal_a != test_equal_b) {\
	cout << test_equal_a << " != " << test_equal_b << endl;\
	return false;\
    }\
} while (0)

inline string
r_r_p(string a, const string & b)
{
    resolve_relative_path(a, b);
    return a;
}

static bool test_resolverelativepath1()
{
    TEST_EQ(r_r_p("/abs/o/lute", ""), "/abs/o/lute");
    TEST_EQ(r_r_p("/abs/o/lute", "/"), "/abs/o/lute");
    TEST_EQ(r_r_p("/abs/o/lute", "//"), "/abs/o/lute");
    TEST_EQ(r_r_p("/abs/o/lute", "foo"), "/abs/o/lute");
    TEST_EQ(r_r_p("/abs/o/lute", "foo/"), "/abs/o/lute");
    TEST_EQ(r_r_p("/abs/o/lute", "/foo"), "/abs/o/lute");
    TEST_EQ(r_r_p("/abs/o/lute", "/foo/"), "/abs/o/lute");
    TEST_EQ(r_r_p("/abs/o/lute", "foo/bar"), "/abs/o/lute");
    TEST_EQ(r_r_p("/abs/o/lute", "foo/bar/"), "/abs/o/lute");
    TEST_EQ(r_r_p("/abs/o/lute", "/foo/bar"), "/abs/o/lute");
    TEST_EQ(r_r_p("/abs/o/lute", "/foo/bar/"), "/abs/o/lute");
    TEST_EQ(r_r_p("rel/a/tive", ""), "rel/a/tive");
    TEST_EQ(r_r_p("rel/a/tive", "/"), "/rel/a/tive");
    TEST_EQ(r_r_p("rel/a/tive", "//"), "//rel/a/tive");
    TEST_EQ(r_r_p("rel/a/tive", "foo"), "rel/a/tive");
    TEST_EQ(r_r_p("rel/a/tive", "foo/"), "foo/rel/a/tive");
    TEST_EQ(r_r_p("rel/a/tive", "/foo"), "/rel/a/tive");
    TEST_EQ(r_r_p("rel/a/tive", "/foo/"), "/foo/rel/a/tive");
    TEST_EQ(r_r_p("rel/a/tive", "foo/bar"), "foo/rel/a/tive");
    TEST_EQ(r_r_p("rel/a/tive", "foo/bar/"), "foo/bar/rel/a/tive");
    TEST_EQ(r_r_p("rel/a/tive", "/foo/bar"), "/foo/rel/a/tive");
    TEST_EQ(r_r_p("rel/a/tive", "/foo/bar/"), "/foo/bar/rel/a/tive");
#ifndef __WIN32__
    TEST_EQ(r_r_p("/abs/o/lute", "/foo\\bar"), "/abs/o/lute");
    TEST_EQ(r_r_p("rel/a/tive", "/foo\\bar"), "/rel/a/tive");
#else
    TEST_EQ(r_r_p("\\dos\\path", ""), "\\dos\\path");
    TEST_EQ(r_r_p("\\dos\\path", "/"), "\\dos\\path");
    TEST_EQ(r_r_p("\\dos\\path", "\\"), "\\dos\\path");
    TEST_EQ(r_r_p("\\dos\\path", "c:"), "c:\\dos\\path");
    TEST_EQ(r_r_p("\\dos\\path", "c:\\"), "c:\\dos\\path");
    TEST_EQ(r_r_p("\\dos\\path", "c:\\temp"), "c:\\dos\\path");
    TEST_EQ(r_r_p("\\dos\\path", "c:\\temp\\"), "c:\\dos\\path");
    TEST_EQ(r_r_p("rel/a/tive", "\\"), "\\rel/a/tive");
    TEST_EQ(r_r_p("rel/a/tive", "foo\\"), "foo\\rel/a/tive");
    TEST_EQ(r_r_p("rel\\a\\tive", "/foo/"), "/foo/rel\\a\\tive");
    TEST_EQ(r_r_p("rel/a/tive", "c:/foo/bar"), "c:/foo/rel/a/tive");
    TEST_EQ(r_r_p("rel/a/tive", "c:foo/bar/"), "c:foo/bar/rel/a/tive");
    TEST_EQ(r_r_p("rel/a/tive", "c:"), "c:rel/a/tive");
    TEST_EQ(r_r_p("rel/a/tive", "c:\\"), "c:\\rel/a/tive");
    TEST_EQ(r_r_p("C:rel/a/tive", "c:\\foo\\bar"), "C:\\foo\\rel/a/tive");
    TEST_EQ(r_r_p("C:rel/a/tive", "c:"), "C:rel/a/tive");
    // This one is impossible to reliably resolve without knowing the current
    // drive - if it is C:, then the answer is: "C:/abs/o/rel/a/tive"
    TEST_EQ(r_r_p("C:rel/a/tive", "/abs/o/lute"), "C:rel/a/tive");
    // UNC paths tests:
    TEST_EQ(r_r_p("\\\\SRV\\VOL\\FILE", "/a/b"), "\\\\SRV\\VOL\\FILE");
    TEST_EQ(r_r_p("rel/a/tive", "\\\\SRV\\VOL\\DIR\\FILE"), "\\\\SRV\\VOL\\DIR\\rel/a/tive");
    TEST_EQ(r_r_p("/abs/o/lute", "\\\\SRV\\VOL\\FILE"), "\\\\SRV\\VOL/abs/o/lute");
    TEST_EQ(r_r_p("/abs/o/lute", "\\\\S\\V\\FILE"), "\\\\S\\V/abs/o/lute");
    TEST_EQ(r_r_p("/abs/o/lute", "\\\\S\\V\\"), "\\\\S\\V/abs/o/lute");
    TEST_EQ(r_r_p("/abs/o/lute", "\\\\S\\V"), "\\\\S\\V/abs/o/lute");
    TEST_EQ(r_r_p("//SRV/VOL/FILE", "/a/b"), "//SRV/VOL/FILE");
    TEST_EQ(r_r_p("rel/a/tive", "//SRV/VOL/DIR/FILE"), "//SRV/VOL/DIR/rel/a/tive");
    TEST_EQ(r_r_p("/abs/o/lute", "//SRV/VOL/FILE"), "//SRV/VOL/abs/o/lute");
    TEST_EQ(r_r_p("/abs/o/lute", "//S/V/FILE"), "//S/V/abs/o/lute");
    TEST_EQ(r_r_p("/abs/o/lute", "//S/V/"), "//S/V/abs/o/lute");
    TEST_EQ(r_r_p("/abs/o/lute", "//S/V"), "//S/V/abs/o/lute");
    TEST_EQ(r_r_p("/abs/o/lute", "\\\\?\\C:\\wibble"), "\\\\?\\C:\\abs\\o\\lute");
    TEST_EQ(r_r_p("/abs/o/lute", "\\\\?\\UNC\\S\\V"), "\\\\?\\UNC\\S\\V\\abs\\o\\lute");
    TEST_EQ(r_r_p("/abs/o/lute", "\\\\?\\UNC\\S\\V\\"), "\\\\?\\UNC\\S\\V\\abs\\o\\lute");
    TEST_EQ(r_r_p("/abs/o/lute", "\\\\?\\UNC\\S\\V\\TMP\\README.TXT"), "\\\\?\\UNC\\S\\V\\abs\\o\\lute");
    TEST_EQ(r_r_p("r/elativ/e", "\\\\?\\C:\\wibble"), "\\\\?\\C:\\r\\elativ\\e");
    TEST_EQ(r_r_p("r/elativ/e", "\\\\?\\C:\\wibble\\wobble"), "\\\\?\\C:\\wibble\\r\\elativ\\e");
#if 0 // Is this a valid testcase?  It fails, but isn't relevant to Xapian.
    TEST_EQ(r_r_p("r/elativ/e", "\\\\?\\UNC\\S\\V"), "\\\\?\\UNC\\S\\V\\r\\elativ\\e");
#endif
    TEST_EQ(r_r_p("r/elativ/e", "\\\\?\\UNC\\S\\V\\"), "\\\\?\\UNC\\S\\V\\r\\elativ\\e");
    TEST_EQ(r_r_p("r/elativ/e", "\\\\?\\UNC\\S\\V\\TMP\\README.TXT"), "\\\\?\\UNC\\S\\V\\TMP\\r\\elativ\\e");
#endif
    return true;
}

#define TEST_EQUAL(A, B) do {\
    if ((A) != (B)) {\
	cout << (A) << " != " << (B) << endl;\
	return false;\
    }\
} while (0)

static bool check_double_serialisation(double u)
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
    TEST_EQUAL(u, v);
    return true;
}

// Check serialisation of doubles.
static bool test_serialisedouble1()
{
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

    if (!check_double_serialisation(0.0)) return false;
    if (!check_double_serialisation(1.0)) return false;
    if (!check_double_serialisation(-1.0)) return false;
    if (!check_double_serialisation(DBL_MAX)) return false;
    if (!check_double_serialisation(-DBL_MAX)) return false;
    if (!check_double_serialisation(DBL_MIN)) return false;
    if (!check_double_serialisation(-DBL_MIN)) return false;

    const double *p;
    for (p = test_values; p < test_values + sizeof(test_values) / sizeof(double); ++p) {
	double val = *p;
	if (!check_double_serialisation(val)) return false;
	if (!check_double_serialisation(-val)) return false;
	if (!check_double_serialisation(1.0 / val)) return false;
	if (!check_double_serialisation(-1.0 / val)) return false;
    }

    return true;
}

#define RUNTEST(T) \
    cout << #T" ... "; \
    if (test_##T()) { \
	cout << "ok" << endl; \
    } else { \
	cout << "FAIL" << endl; \
	result = 1; \
    }

int main()
try {
    int result = 0;

    RUNTEST(resolverelativepath1);
    RUNTEST(serialisedouble1);

    return result;
} catch (const char * e) {
    cout << e << endl;
    return 1;
}
