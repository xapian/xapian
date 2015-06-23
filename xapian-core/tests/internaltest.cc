/* internaltest.cc: test of the Xapian internals
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2006,2007,2008,2009,2010 Olly Betts
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

#include <cfloat>
#include "safeerrno.h"

#include <iostream>
#include <string>

using namespace std;

#include "autoptr.h"
#include "testsuite.h"
#include "testutils.h"

#include "net/length.h"
#include "omassert.h"
#include "omqueryinternal.h"
#include "pack.h"
#include "serialise.h"
#include "serialise-double.h"
#include "str.h"

#if defined XAPIAN_ENABLE_VISIBILITY && defined __GNUC__ && (__GNUC__ >= 4)
# define XAPIAN_UNITTEST
# include "../net/length.cc"
#else
# define Xapian_NetworkError Xapian::NetworkError
#endif

static bool test_except1()
{
    try {
	throw 1;
    } catch (int) {
    }
    return true;
}

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

class test_refcnt : public Xapian::Internal::RefCntBase {
    private:
	bool &deleted;
    public:
	test_refcnt(bool &deleted_) : deleted(deleted_) {
	    tout << "constructor\n";
	}
	Xapian::Internal::RefCntPtr<const test_refcnt> test() {
	    return Xapian::Internal::RefCntPtr<const test_refcnt>(this);
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

    TEST_EQUAL(p->ref_count, 0);

    {
	Xapian::Internal::RefCntPtr<test_refcnt> rcp(p);

	TEST_EQUAL(rcp->ref_count, 1);

	{
	    Xapian::Internal::RefCntPtr<test_refcnt> rcp2;
	    rcp2 = rcp;
	    TEST_EQUAL(rcp->ref_count, 2);
	    // rcp2 goes out of scope here
	}

	TEST_AND_EXPLAIN(!deleted, "Object prematurely deleted!");
	TEST_EQUAL(rcp->ref_count, 1);
	// rcp goes out of scope here
    }

    TEST_AND_EXPLAIN(deleted, "Object not properly deleted");

    return true;
}

// This is a regression test - a RefCntPtr used to delete the object pointed
// to if you assigned it to itself and the reference count was 1.
static bool test_refcnt2()
{
    bool deleted = false;

    test_refcnt *p = new test_refcnt(deleted);

    Xapian::Internal::RefCntPtr<test_refcnt> rcp(p);

    rcp = rcp;

    TEST_AND_EXPLAIN(!deleted, "Object deleted by self-assignment");

    return true;
}

// Class for testing AutoPtr<>.
class test_autoptr {
    bool &deleted;
  public:
    test_autoptr(bool &deleted_) : deleted(deleted_) {
	tout << "test_autoptr constructor\n";
    }
    ~test_autoptr() {
	deleted = true;
	tout << "test_autoptr destructor\n";
    }
};

// Test autoptr self-assignment.
static bool test_autoptr1()
{
    bool deleted = false;

    test_autoptr * raw_ptr = new test_autoptr(deleted);
    {
	AutoPtr<test_autoptr> ptr(raw_ptr);

	TEST_EQUAL(ptr.get(), raw_ptr);

	TEST(!deleted);

	ptr = ptr;

	TEST_EQUAL(ptr.get(), raw_ptr);

	TEST(!deleted);
    }

    TEST(deleted);

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

static bool test_tostring1()
{
    TEST_EQUAL(str(0), "0");
    TEST_EQUAL(str(10), "10");
    TEST_EQUAL(str(10u), "10");
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
	    unsigned result;
	    decode_length_and_check(&p, p_end, result);
	    TEST(result == 0);
	    TEST(p == p_end);
	}
	s += 'x';
	{
	    const char *p = s.data();
	    const char *p_end = p + s.size();
	    unsigned result;
	    decode_length_and_check(&p, p_end, result);
	    TEST(result == 0);
	    TEST_EQUAL(p_end - p, 1);
	}
    }
    // Special case tests for 1
    {
	string s = encode_length(1);
	TEST_EXCEPTION(Xapian_NetworkError,
	    const char *p = s.data();
	    const char *p_end = p + s.size();
	    unsigned result;
	    decode_length_and_check(&p, p_end, result);
	    TEST(result == 1);
	);
	s += 'x';
	{
	    const char *p = s.data();
	    const char *p_end = p + s.size();
	    unsigned result;
	    decode_length_and_check(&p, p_end, result);
	    TEST(result == 1);
	    TEST_EQUAL(p_end - p, 1);
	}
	s += 'x';
	{
	    const char *p = s.data();
	    const char *p_end = p + s.size();
	    unsigned result;
	    decode_length_and_check(&p, p_end, result);
	    TEST(result == 1);
	    TEST_EQUAL(p_end - p, 2);
	}
    }
    // Nothing magic here, just test a range of odd and even values.
    for (size_t n = 2; n < 1000; n = (n + 1) * 2 + (n >> 1)) {
	string s = encode_length(n);
	TEST_EXCEPTION(Xapian_NetworkError,
	    const char *p = s.data();
	    const char *p_end = p + s.size();
	    unsigned result;
	    decode_length_and_check(&p, p_end, result);
	    TEST(result == n);
	);
	s.append(n-1, 'x');
	TEST_EXCEPTION(Xapian_NetworkError,
	    const char *p = s.data();
	    const char *p_end = p + s.size();
	    unsigned result;
	    decode_length_and_check(&p, p_end, result);
	    TEST(result == n);
	);
	s += 'x';
	{
	    const char *p = s.data();
	    const char *p_end = p + s.size();
	    unsigned result;
	    decode_length_and_check(&p, p_end, result);
	    TEST(result == n);
	    TEST_EQUAL(size_t(p_end - p), n);
	}
	s += 'x';
	{
	    const char *p = s.data();
	    const char *p_end = p + s.size();
	    unsigned result;
	    decode_length_and_check(&p, p_end, result);
	    TEST(result == n);
	    TEST_EQUAL(size_t(p_end - p), n + 1);
	}
    }

    return true;
}
#endif

static void check_double_serialisation(double u)
{
    string encoded = serialise_double(u);
    const char * ptr = encoded.data();
    const char * end = ptr + encoded.size();
    double v = unserialise_double(&ptr, end);
    if (ptr != end || u != v) {
	tout << u << " -> " << v << ", difference = " << v - u << endl;
	tout << "FLT_RADIX = " << FLT_RADIX << endl;
	tout << "DBL_MAX_EXP = " << DBL_MAX_EXP << endl;
    }
    TEST(ptr == end);
    TEST_EQUAL(u, v);
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
// Check serialisation of documents.
static bool test_serialisedoc1()
{
    Xapian::Document doc;

    string s;

    s = serialise_document(doc);
    TEST(serialise_document(unserialise_document(s)) == s);

    doc.set_data("helllooooo");
    doc.add_term("term");
    doc.add_value(1, "foo");

    s = serialise_document(doc);
    TEST(serialise_document(unserialise_document(s)) == s);

    return true;
}

static void
serialisequery1_helper(const Xapian::Query & query)
{
    string before = query.internal->serialise();
    Xapian::Registry reg;
    Xapian::Query::Internal * qint;
    qint = Xapian::Query::Internal::unserialise(before, reg);
    string after = qint->serialise();
    delete qint;
    TEST(before == after);
}

// Check serialisation of queries.
static bool test_serialisequery1()
{
    string s;

    serialisequery1_helper(Xapian::Query("foo"));

    // Regression test for bug in 0.9.10 and earlier.
    serialisequery1_helper(Xapian::Query("foo", 1, 1));

    serialisequery1_helper(Xapian::Query(Xapian::Query::OP_OR,
					 Xapian::Query("foo", 1, 1),
					 Xapian::Query("bar", 1, 1)));

    static const char * words[] = { "paragraph", "word" };
    serialisequery1_helper(Xapian::Query(Xapian::Query::OP_OR, words, words + 2));

    static const char * words2[] = { "milk", "on", "fridge" };
    serialisequery1_helper(
	    Xapian::Query(Xapian::Query::OP_SCALE_WEIGHT,
			  Xapian::Query(Xapian::Query::OP_OR,
					Xapian::Query("leave"),
					Xapian::Query(Xapian::Query::OP_PHRASE, words2, words2 + 3)
					),
			  2.5)
	    );

    return true;
}

// Check serialisation of Xapian::Error.
static bool test_serialiseerror1()
{
    string enoent_msg(strerror(ENOENT));
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

static bool test_static_assert1()
{
    STATIC_ASSERT(true);
    STATIC_ASSERT(1);
    STATIC_ASSERT(-1);
    STATIC_ASSERT(42);
    STATIC_ASSERT(sizeof(char) == 1);

    // FIXME: We should test cases which should fail, but these are hard to
    // check with our current test framework.

    STATIC_ASSERT_UNSIGNED_TYPE(bool);
    STATIC_ASSERT_UNSIGNED_TYPE(unsigned char);
    STATIC_ASSERT_UNSIGNED_TYPE(unsigned short);
    STATIC_ASSERT_UNSIGNED_TYPE(unsigned int);
    STATIC_ASSERT_UNSIGNED_TYPE(unsigned long);

    // FIXME: We should test cases which should fail, but these are hard to
    // check with our current test framework.

    STATIC_ASSERT_TYPE_DOMINATES(unsigned long, unsigned long);
    STATIC_ASSERT_TYPE_DOMINATES(unsigned int, unsigned int);
    STATIC_ASSERT_TYPE_DOMINATES(unsigned short, unsigned short);
    STATIC_ASSERT_TYPE_DOMINATES(unsigned char, unsigned char);

    STATIC_ASSERT_TYPE_DOMINATES(long, long);
    STATIC_ASSERT_TYPE_DOMINATES(int, int);
    STATIC_ASSERT_TYPE_DOMINATES(short, short);
    STATIC_ASSERT_TYPE_DOMINATES(signed char, signed char);

    STATIC_ASSERT_TYPE_DOMINATES(char, char);

    STATIC_ASSERT_TYPE_DOMINATES(unsigned long, unsigned int);
    STATIC_ASSERT_TYPE_DOMINATES(unsigned int, unsigned short);
    STATIC_ASSERT_TYPE_DOMINATES(unsigned short, unsigned char);

    STATIC_ASSERT_TYPE_DOMINATES(long, int);
    STATIC_ASSERT_TYPE_DOMINATES(int, short);
    STATIC_ASSERT_TYPE_DOMINATES(short, signed char);

    STATIC_ASSERT_TYPE_DOMINATES(long, unsigned char);
    STATIC_ASSERT_TYPE_DOMINATES(int, unsigned char);
    STATIC_ASSERT_TYPE_DOMINATES(short, unsigned char);

    // FIXME: We should test cases which should fail, but these are hard to
    // check with our current test framework.

    return true;
}

/// Regression test for bug fixed in 1.1.1.
static bool test_strbool1()
{
    TEST_EQUAL(str(true), "1");
    TEST_EQUAL(str(false), "0");
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
    return true;
}

// ##################################################################
// # End of actual tests					    #
// ##################################################################

/// The lists of tests to perform
static const test_desc tests[] = {
    {"except1",			test_except1},
    {"exception1",		test_exception1},
    {"refcnt1",			test_refcnt1},
    {"refcnt2",			test_refcnt2},
    {"autoptr1",		test_autoptr1},
    {"stringcomp1",		test_stringcomp1},
    {"temporarydtor1",		test_temporarydtor1},
    {"tostring1",		test_tostring1},
    {"serialisedouble1",	test_serialisedouble1},
#ifdef XAPIAN_HAS_REMOTE_BACKEND
    {"serialiselength1",	test_serialiselength1},
    {"serialiselength2",	test_serialiselength2},
    {"serialisedoc1",		test_serialisedoc1},
    {"serialisequery1",		test_serialisequery1},
    {"serialiseerror1",		test_serialiseerror1},
#endif
    {"static_assert1",		test_static_assert1},
    {"strbool1",		test_strbool1},
    {"pack1",			test_pack_uint_preserving_sort1},
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
