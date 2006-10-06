/* internaltest.cc: test of the Xapian internals
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2006 Olly Betts
 * Copyright 2006 Richard Boulton
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

#include <float.h>

#include <string>
#include <list>

using namespace std;

#include <xapian.h>
#include "testsuite.h"
#include "omstringstream.h"

#include "serialise.h"
#include "serialise-double.h"
#include "omqueryinternal.h"

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
    } catch (Test_Exception & e) {
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
// to if you assignment it to itself and the reference count was 1.
static bool test_refcnt2()
{
    bool deleted = false;

    test_refcnt *p = new test_refcnt(deleted);

    Xapian::Internal::RefCntPtr<test_refcnt> rcp(p);
    
    rcp = rcp;
    
    TEST_AND_EXPLAIN(!deleted, "Object deleted by self-assignment");

    return true;
}

// test string comparisions
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
	tout << "String comparisions don't cope with extra nulls" << endl;
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

static bool test_omstringstream1()
{
    om_ostringstream oss;
    oss << "foo" << 4 << "bar";
    TEST_EQUAL(oss.str(), "foo4bar");

    return true;
}

// Check serialisation of lengths.
static bool test_serialiselength1()
{
    size_t n = 0;
    while (n < 0xff000000) {
	string s = encode_length(n);
	const char *p = s.data();
	const char *p_end = p + s.size();
	size_t decoded_n = decode_length(&p, p_end);
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

// Check serialisation of queries.
static bool test_serialisequery1()
{
    string s;
    list<Xapian::Query> queries;
    
    queries.push_back(Xapian::Query("foo"));

    const char * words[] = { "paragraph", "word" };
    queries.push_back(Xapian::Query(Xapian::Query::OP_OR, words, words + 2));

    list<Xapian::Query>::const_iterator query;
    for(query = queries.begin(); query != queries.end(); query++) {
        Xapian::Query::Internal * qint;

        try {
            s = query->internal->serialise();
        } catch (const Xapian::InternalError & e) {
            SKIP_TEST("query serialisation not compiled in");
        }

        try {
            qint = Xapian::Query::Internal::unserialise(s);
        } catch (const Xapian::InternalError & e) {
            SKIP_TEST("query unserialisation not compiled in");
        }

        TEST(qint->serialise() == s);
        delete qint;
    }

    return true;
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

// ##################################################################
// # End of actual tests                                            #
// ##################################################################

/// The lists of tests to perform
test_desc tests[] = {
    {"except1",			test_except1},
    {"exception1",              test_exception1},
    {"refcnt1",			test_refcnt1},
    {"refcnt2",			test_refcnt2},
    {"stringcomp1",		test_stringcomp1},
    {"omstringstream1",		test_omstringstream1},
    {"serialiselength1",	test_serialiselength1},
    {"serialisedouble1",	test_serialisedouble1},
    {"serialisedoc1",		test_serialisedoc1},
    {"temporarydtor1",		test_temporarydtor1},
    {"serialisequery1",		test_serialisequery1},
    {0, 0}
};

int main(int argc, char **argv)
{
    test_driver::parse_command_line(argc, argv);
    return test_driver::run(tests);
}
