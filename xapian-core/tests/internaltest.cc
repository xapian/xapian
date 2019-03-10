/** @file internaltest.cc
 * @brief test of the Xapian internals
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2006,2007,2008,2009,2010,2011,2012,2015 Olly Betts
 * Copyright 2006 Lemur Consulting Ltd
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

#include <xapian.h>

#include <iostream>
#include <string>

using namespace std;

#include "testsuite.h"
#include "testutils.h"

#include "pack.h"
#include "str.h"

class Test_Exception {
    public:
	int value;
	Test_Exception(int value_) : value(value_) {}
};

// test that nested exceptions work correctly.
static bool test_exception1()
{
    try {
	try {
	    throw Test_Exception(1);
	} catch (...) {
	    try {
		throw Test_Exception(2);
	    } catch (...) {
	    }
	    throw;
	}
    } catch (const Test_Exception & e) {
	TEST_EQUAL(e.value, 1);
	return true;
    }
    return false;
}

// ###########################################
// # Tests of the reference counted pointers #
// ###########################################

class test_refcnt : public Xapian::Internal::intrusive_base {
    private:
	bool &deleted;
    public:
	test_refcnt(bool &deleted_) : deleted(deleted_) {
	    tout << "constructor\n";
	}
	Xapian::Internal::intrusive_ptr<const test_refcnt> test() {
	    return Xapian::Internal::intrusive_ptr<const test_refcnt>(this);
	}
	~test_refcnt() {
	    deleted = true;
	    tout << "destructor\n";
	}
};

static bool test_refcnt1()
{
    bool deleted = false;

    test_refcnt *p = new test_refcnt(deleted);

    TEST_EQUAL(p->_refs, 0);

    {
	Xapian::Internal::intrusive_ptr<test_refcnt> rcp(p);

	TEST_EQUAL(rcp->_refs, 1);

	{
	    Xapian::Internal::intrusive_ptr<test_refcnt> rcp2;
	    rcp2 = rcp;
	    TEST_EQUAL(rcp->_refs, 2);
	    // rcp2 goes out of scope here
	}

	TEST_AND_EXPLAIN(!deleted, "Object prematurely deleted!");
	TEST_EQUAL(rcp->_refs, 1);
	// rcp goes out of scope here
    }

    TEST_AND_EXPLAIN(deleted, "Object not properly deleted");

    return true;
}

// This is a regression test - our home-made equivalent of intrusive_ptr
// (which was called RefCntPtr) used to delete the object pointed to if you
// assigned it to itself and the reference count was 1.
static bool test_refcnt2()
{
    bool deleted = false;

    test_refcnt *p = new test_refcnt(deleted);

    Xapian::Internal::intrusive_ptr<test_refcnt> rcp(p);

#ifdef __has_warning
# if __has_warning("-Wself-assign-overloaded")
    // Suppress warning from newer clang about self-assignment so we can
    // test that self-assignment works!
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wself-assign-overloaded"
# endif
#endif
    rcp = rcp;
#ifdef __has_warning
# if __has_warning("-Wself-assign-overloaded")
#  pragma clang diagnostic pop
# endif
#endif

    TEST_AND_EXPLAIN(!deleted, "Object deleted by self-assignment");

    return true;
}

// test string comparisons
static bool test_stringcomp1()
{
    bool success = true;

    string s1;
    string s2;

    s1 = "foo";
    s2 = "foo";

    if ((s1 != s2) || (s1 > s2)) {
	success = false;
	tout << "String comparisons BADLY wrong" << endl;
    }

    s1 += '\0';

    if ((s1 == s2) || (s1 < s2)) {
	success = false;
	tout << "String comparisons don't cope with extra nulls" << endl;
    }

    s2 += '\0';

    s1 += 'a';
    s2 += 'z';

    if ((s1.length() != 5) || (s2.length() != 5)) {
	success = false;
	tout << "Lengths with added nulls wrong" << endl;
    }

    if ((s1 == s2) || !(s1 < s2)) {
	success = false;
	tout << "Characters after a null ignored in comparisons" << endl;
    }

    return success;
}

// By default Sun's C++ compiler doesn't call the destructor on a
// temporary object until the end of the block (contrary to what
// ISO C++ requires).  This is done in the name of "compatibility".
// Passing -features=tmplife to CC fixes this.  This check ensures
// that this actually works for Sun's C++ and any other compilers
// that might have this problem.
struct TempDtorTest {
    static int count;
    static TempDtorTest factory() { return TempDtorTest(); }
    TempDtorTest() { ++count; }
    ~TempDtorTest() { --count; }
};

int TempDtorTest::count = 0;

static bool test_temporarydtor1()
{
    TEST_EQUAL(TempDtorTest::count, 0);
    TempDtorTest::factory();
    TEST_EQUAL(TempDtorTest::count, 0);

    return true;
}

/// Test pack_uint_preserving_sort()
static bool test_pack_uint_preserving_sort1()
{
    string prev_packed;
    for (unsigned int i = 0; i != 1000; ++i) {
	string packed;
	pack_uint_preserving_sort(packed, i);
	const char * ptr = packed.data();
	const char * end = ptr + packed.size();
	unsigned int result;
	TEST(unpack_uint_preserving_sort(&ptr, end, &result));
	TEST_EQUAL(result, i);
	TEST(ptr == end);
	TEST_REL(prev_packed, <, packed);
	swap(prev_packed, packed);
    }
    for (unsigned int i = 2345; i < 65000; i += 113) {
	string packed;
	pack_uint_preserving_sort(packed, i);
	const char * ptr = packed.data();
	const char * end = ptr + packed.size();
	unsigned int result;
	TEST(unpack_uint_preserving_sort(&ptr, end, &result));
	TEST_EQUAL(result, i);
	TEST(ptr == end);
	TEST_REL(prev_packed, <, packed);
	swap(prev_packed, packed);
    }
    unsigned int prev = 64999;
    for (unsigned int i = 65000; i > prev; prev = i, i = (i << 1) ^ 1337) {
	string packed;
	pack_uint_preserving_sort(packed, i);
	const char * ptr = packed.data();
	const char * end = ptr + packed.size();
	unsigned int result;
	TEST(unpack_uint_preserving_sort(&ptr, end, &result));
	TEST_EQUAL(result, i);
	TEST(ptr == end);
	TEST_REL(prev_packed, <, packed);
	swap(prev_packed, packed);
    }

    /* Test packing multiple numbers to one string. */
    string packed;
    for (unsigned int i = 23456; i < 765432; i += 1131) {
	pack_uint_preserving_sort(packed, i);
    }
    const char * ptr = packed.data();
    const char * end = ptr + packed.size();
    for (unsigned int i = 23456; i < 765432; i += 1131) {
	unsigned int result;
	TEST(unpack_uint_preserving_sort(&ptr, end, &result));
	TEST_EQUAL(result, i);
    }
    TEST(ptr == end);

    return true;
}

/// Test C_isupper() etc.
static bool test_chartype1()
{
    char tested[128];
    memset(tested, 0, sizeof(tested));
    for (int ch = '0'; ch != '9' + 1; ++ch) {
	tested[ch] = 1;
	TEST(!C_isupper(ch));
	TEST(!C_islower(ch));
	TEST(!C_isalpha(ch));
	TEST(C_isalnum(ch));
	TEST(C_isdigit(ch));
	TEST(C_isxdigit(ch));
	TEST(!C_isspace(ch));
	TEST(C_isnotupper(ch));
	TEST(C_isnotlower(ch));
	TEST(C_isnotalpha(ch));
	TEST(!C_isnotalnum(ch));
	TEST(!C_isnotdigit(ch));
	TEST(!C_isnotxdigit(ch));
	TEST(C_isnotspace(ch));
	TEST_EQUAL(hex_digit(ch), ch - '0');
    }

    for (int ch = 'A'; ch != 'F' + 1; ++ch) {
	tested[ch] = 1;
	TEST(C_isupper(ch));
	TEST(!C_islower(ch));
	TEST(C_isalpha(ch));
	TEST(C_isalnum(ch));
	TEST(!C_isdigit(ch));
	TEST(C_isxdigit(ch));
	TEST(!C_isspace(ch));
	TEST(!C_isnotupper(ch));
	TEST(C_isnotlower(ch));
	TEST(!C_isnotalpha(ch));
	TEST(!C_isnotalnum(ch));
	TEST(C_isnotdigit(ch));
	TEST(!C_isnotxdigit(ch));
	TEST(C_isnotspace(ch));
	TEST_EQUAL(hex_digit(ch), ch - 'A' + 10);
    }

    for (int ch = 'G'; ch != 'Z' + 1; ++ch) {
	tested[ch] = 1;
	TEST(C_isupper(ch));
	TEST(!C_islower(ch));
	TEST(C_isalpha(ch));
	TEST(C_isalnum(ch));
	TEST(!C_isdigit(ch));
	TEST(!C_isxdigit(ch));
	TEST(!C_isspace(ch));
	TEST(!C_isnotupper(ch));
	TEST(C_isnotlower(ch));
	TEST(!C_isnotalpha(ch));
	TEST(!C_isnotalnum(ch));
	TEST(C_isnotdigit(ch));
	TEST(C_isnotxdigit(ch));
	TEST(C_isnotspace(ch));
    }

    for (int ch = 'a'; ch != 'f' + 1; ++ch) {
	tested[ch] = 1;
	TEST(!C_isupper(ch));
	TEST(C_islower(ch));
	TEST(C_isalpha(ch));
	TEST(C_isalnum(ch));
	TEST(!C_isdigit(ch));
	TEST(C_isxdigit(ch));
	TEST(!C_isspace(ch));
	TEST(C_isnotupper(ch));
	TEST(!C_isnotlower(ch));
	TEST(!C_isnotalpha(ch));
	TEST(!C_isnotalnum(ch));
	TEST(C_isnotdigit(ch));
	TEST(!C_isnotxdigit(ch));
	TEST(C_isnotspace(ch));
	TEST_EQUAL(hex_digit(ch), ch - 'a' + 10);
    }

    for (int ch = 'g'; ch != 'z' + 1; ++ch) {
	tested[ch] = 1;
	TEST(!C_isupper(ch));
	TEST(C_islower(ch));
	TEST(C_isalpha(ch));
	TEST(C_isalnum(ch));
	TEST(!C_isdigit(ch));
	TEST(!C_isxdigit(ch));
	TEST(!C_isspace(ch));
	TEST(C_isnotupper(ch));
	TEST(!C_isnotlower(ch));
	TEST(!C_isnotalpha(ch));
	TEST(!C_isnotalnum(ch));
	TEST(C_isnotdigit(ch));
	TEST(C_isnotxdigit(ch));
	TEST(C_isnotspace(ch));
    }

    for (const char *p = "\t\n\f\r "; *p; ++p) {
	int ch = *p;
	tested[ch] = 1;
	TEST(!C_isupper(ch));
	TEST(!C_islower(ch));
	TEST(!C_isalpha(ch));
	TEST(!C_isalnum(ch));
	TEST(!C_isdigit(ch));
	TEST(!C_isxdigit(ch));
	TEST(C_isspace(ch));
	TEST(C_isnotupper(ch));
	TEST(C_isnotlower(ch));
	TEST(C_isnotalpha(ch));
	TEST(C_isnotalnum(ch));
	TEST(C_isnotdigit(ch));
	TEST(C_isnotxdigit(ch));
	TEST(!C_isnotspace(ch));
    }

    // Check remaining non-top-bit-set characters aren't anything.
    for (int ch = 0; ch != 128; ++ch) {
	if (tested[ch]) continue;
	TEST(!C_isupper(ch));
	TEST(!C_islower(ch));
	TEST(!C_isalpha(ch));
	TEST(!C_isalnum(ch));
	TEST(!C_isdigit(ch));
	TEST(!C_isxdigit(ch));
	TEST(!C_isspace(ch));
	TEST(C_isnotupper(ch));
	TEST(C_isnotlower(ch));
	TEST(C_isnotalpha(ch));
	TEST(C_isnotalnum(ch));
	TEST(C_isnotdigit(ch));
	TEST(C_isnotxdigit(ch));
	TEST(C_isnotspace(ch));
    }

    // Non-ASCII characters aren't anything for these functions.
    for (unsigned char ch = 128; ch != 0; ++ch) {
	TEST(!C_isupper(ch));
	TEST(!C_islower(ch));
	TEST(!C_isalpha(ch));
	TEST(!C_isalnum(ch));
	TEST(!C_isdigit(ch));
	TEST(!C_isxdigit(ch));
	TEST(!C_isspace(ch));
	TEST(C_isnotupper(ch));
	TEST(C_isnotlower(ch));
	TEST(C_isnotalpha(ch));
	TEST(C_isnotalnum(ch));
	TEST(C_isnotdigit(ch));
	TEST(C_isnotxdigit(ch));
	TEST(C_isnotspace(ch));
    }

    // Check signed char values work the same way.
    for (signed char ch = -128; ch != 0; ++ch) {
	TEST(!C_isupper(ch));
	TEST(!C_islower(ch));
	TEST(!C_isalpha(ch));
	TEST(!C_isalnum(ch));
	TEST(!C_isdigit(ch));
	TEST(!C_isxdigit(ch));
	TEST(!C_isspace(ch));
	TEST(C_isnotupper(ch));
	TEST(C_isnotlower(ch));
	TEST(C_isnotalpha(ch));
	TEST(C_isnotalnum(ch));
	TEST(C_isnotdigit(ch));
	TEST(C_isnotxdigit(ch));
	TEST(C_isnotspace(ch));
    }

    return true;
}

// ##################################################################
// # End of actual tests					    #
// ##################################################################

/// The lists of tests to perform
static const test_desc tests[] = {
    TESTCASE(exception1),
    TESTCASE(refcnt1),
    TESTCASE(refcnt2),
    TESTCASE(stringcomp1),
    TESTCASE(temporarydtor1),
    TESTCASE(pack_uint_preserving_sort1),
    TESTCASE(chartype1),
    {0, 0}
};

int main(int argc, char **argv)
try {
    test_driver::parse_command_line(argc, argv);
    return test_driver::run(tests);
} catch (const char * e) {
    cout << e << endl;
    return 1;
}
