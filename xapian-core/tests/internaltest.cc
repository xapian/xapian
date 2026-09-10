/** @file
 * @brief test of the Xapian internals
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002-2026 Olly Betts
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
 * along with this program; if not, see
 * <https://www.gnu.org/licenses/>.
 */

#include <config.h>

#include <xapian.h>

#include <iostream>
#include <string>

using namespace std;

#include "testsuite.h"
#include "testutils.h"

#include "str.h"

class Test_Exception {
  public:
    int value;
    Test_Exception(int value_) : value(value_) {}
};

// test that nested exceptions work correctly.
static void test_exception1()
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
    } catch (const Test_Exception& e) {
        TEST_EQUAL(e.value, 1);
    }
}

// ###########################################
// # Tests of the reference counted pointers #
// ###########################################

class test_refcnt : public Xapian::Internal::intrusive_base {
    bool& deleted;

  public:
    test_refcnt(bool& deleted_) : deleted(deleted_) {
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

static void test_refcnt1()
{
    bool deleted = false;

    test_refcnt* p = new test_refcnt(deleted);

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
}

// This is a regression test - our home-made equivalent of intrusive_ptr
// (which was called RefCntPtr) used to delete the object pointed to if you
// assigned it to itself and the reference count was 1.
static void test_refcnt2()
{
    bool deleted = false;

    test_refcnt* p = new test_refcnt(deleted);

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
}

// test string comparisons
static void test_stringcomp1()
{
    string s1;
    string s2;

    s1 = "foo";
    s2 = "foo";

    if ((s1 != s2) || (s1 > s2)) {
        FAIL_TEST("String comparisons BADLY wrong");
    }

    s1 += '\0';

    if ((s1 == s2) || (s1 < s2)) {
        FAIL_TEST("String comparisons don't cope with extra nulls");
    }

    s2 += '\0';

    s1 += 'a';
    s2 += 'z';

    if ((s1.length() != 5) || (s2.length() != 5)) {
        FAIL_TEST("Lengths with added nulls wrong");
    }

    if ((s1 == s2) || !(s1 < s2)) {
        FAIL_TEST("Characters after a null ignored in comparisons");
    }
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

static void test_temporarydtor1()
{
    TEST_EQUAL(TempDtorTest::count, 0);
    TempDtorTest::factory();
    TEST_EQUAL(TempDtorTest::count, 0);
}

// ##################################################################
// # End of actual tests                                            #
// ##################################################################

/// The lists of tests to perform
static const test_desc tests[] = {
    TESTCASE(exception1),
    TESTCASE(refcnt1),
    TESTCASE(refcnt2),
    TESTCASE(stringcomp1),
    TESTCASE(temporarydtor1),
    {nullptr, nullptr}
};

int main(int argc, char** argv)
try {
    test_driver::parse_command_line(argc, argv);
    return test_driver::run(tests);
} catch (const char* e) {
    cout << e << '\n';
    return 1;
}
