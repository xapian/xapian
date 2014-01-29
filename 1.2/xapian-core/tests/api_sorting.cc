/** @file api_sorting.cc
 * @brief tests of MSet sorting
 */
/* Copyright (C) 2007,2008,2009 Olly Betts
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

#include "api_sorting.h"

#include <xapian.h>

#include "apitest.h"
#include "testutils.h"

using namespace std;

DEFINE_TESTCASE(sortfunctor1,backend && !remote) {
    Xapian::Enquire enquire(get_database("apitest_sortrel"));
    enquire.set_query(Xapian::Query("woman"));

    {
	const int keys[] = { 3, 1 };
	Xapian::MultiValueKeyMaker sorter(keys, keys + 2);

	enquire.set_sort_by_key(&sorter, true);
	Xapian::MSet mset = enquire.get_mset(0, 10);
	mset_expect_order(mset, 2, 6, 7, 1, 3, 4, 5, 8, 9);
    }

    {
	Xapian::MultiValueKeyMaker sorter;
	sorter.add_value(3);
	sorter.add_value(1, true);

	enquire.set_sort_by_key(&sorter, true);
	Xapian::MSet mset = enquire.get_mset(0, 10);
	mset_expect_order(mset, 7, 6, 2, 8, 9, 4, 5, 1, 3);
    }

    {
	Xapian::MultiValueKeyMaker sorter;
	sorter.add_value(100); // Value 100 isn't set.
	sorter.add_value(3);
	sorter.add_value(1, true);

	enquire.set_sort_by_key(&sorter, true);
	Xapian::MSet mset = enquire.get_mset(0, 10);
	mset_expect_order(mset, 7, 6, 2, 8, 9, 4, 5, 1, 3);
    }

    {
	Xapian::MultiValueKeyMaker sorter;
	sorter.add_value(10); // Value 10 isn't always set.
	sorter.add_value(1, true);

	enquire.set_sort_by_key(&sorter, true);
	Xapian::MSet mset = enquire.get_mset(0, 10);
	mset_expect_order(mset, 8, 9, 4, 5, 1, 3, 7, 6, 2);
    }

    return true;
}

/// Test reverse sort functor.
DEFINE_TESTCASE(sortfunctor2,writable && !remote) {
    Xapian::WritableDatabase db = get_writable_database();
    Xapian::Document doc;
    doc.add_term("foo");
    doc.add_value(0, "ABB");
    db.add_document(doc);
    doc.add_value(0, "ABC");
    db.add_document(doc);
    doc.add_value(0, string("ABC", 4));
    db.add_document(doc);
    doc.add_value(0, "ABCD");
    db.add_document(doc);
    doc.add_value(0, "ABC\xff");
    db.add_document(doc);

    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("foo"));

    {
	Xapian::MultiValueKeyMaker sorter;
	sorter.add_value(0);
	enquire.set_sort_by_key(&sorter, true);
	Xapian::MSet mset = enquire.get_mset(0, 10);
	mset_expect_order(mset, 5, 4, 3, 2, 1);
    }

    {
	Xapian::MultiValueKeyMaker sorter;
	sorter.add_value(0, true);
	enquire.set_sort_by_key(&sorter, true);
	Xapian::MSet mset = enquire.get_mset(0, 10);
	mset_expect_order(mset, 1, 2, 3, 4, 5);
    }

    {
	Xapian::MultiValueKeyMaker sorter;
	sorter.add_value(0);
	sorter.add_value(1);
	enquire.set_sort_by_key(&sorter, true);
	Xapian::MSet mset = enquire.get_mset(0, 10);
	mset_expect_order(mset, 5, 4, 3, 2, 1);
    }

    {
	Xapian::MultiValueKeyMaker sorter;
	sorter.add_value(0, true);
	sorter.add_value(1);
	enquire.set_sort_by_key(&sorter, true);
	Xapian::MSet mset = enquire.get_mset(0, 10);
	mset_expect_order(mset, 1, 2, 3, 4, 5);
    }

    {
	Xapian::MultiValueKeyMaker sorter;
	sorter.add_value(0);
	sorter.add_value(1, true);
	enquire.set_sort_by_key(&sorter, true);
	Xapian::MSet mset = enquire.get_mset(0, 10);
	mset_expect_order(mset, 5, 4, 3, 2, 1);
    }

    {
	Xapian::MultiValueKeyMaker sorter;
	sorter.add_value(0, true);
	sorter.add_value(1, true);
	enquire.set_sort_by_key(&sorter, true);
	Xapian::MSet mset = enquire.get_mset(0, 10);
	mset_expect_order(mset, 1, 2, 3, 4, 5);
    }

    return true;
}

class NeverUseMeKeyMaker : public Xapian::KeyMaker {
  public:
    std::string operator() (const Xapian::Document &) const
    {
	FAIL_TEST("NeverUseMeKeyMaker was called");
    }
};

/// Regression test for changing away from a sorter.
DEFINE_TESTCASE(changesorter1, backend && !remote) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query("word"));
    NeverUseMeKeyMaker sorter;

    enquire.set_sort_by_key(&sorter, true);
    enquire.set_sort_by_value(0, true);
    Xapian::MSet mset = enquire.get_mset(0, 25);
    TEST_EQUAL(mset.size(), 2); // Check that search is still doing something.

    enquire.set_sort_by_key(&sorter, true);
    enquire.set_sort_by_value_then_relevance(0, true);
    mset = enquire.get_mset(0, 25);
    TEST_EQUAL(mset.size(), 2); // Check that search is still doing something.

    enquire.set_sort_by_key(&sorter, true);
    enquire.set_sort_by_relevance_then_value(0, true);
    mset = enquire.get_mset(0, 25);
    TEST_EQUAL(mset.size(), 2); // Check that search is still doing something.

    enquire.set_sort_by_key(&sorter, true);
    enquire.set_sort_by_relevance();
    mset = enquire.get_mset(0, 25);
    TEST_EQUAL(mset.size(), 2); // Check that search is still doing something.

    return true;
}

/// Regression test - an empty MultiValueSorter hung in 1.0.9 and earlier.
DEFINE_TESTCASE(sortfunctorempty1,backend && !remote) {
    Xapian::Enquire enquire(get_database("apitest_sortrel"));
    enquire.set_query(Xapian::Query("woman"));

    {
	int i;
	Xapian::MultiValueKeyMaker sorter(&i, &i);

	enquire.set_sort_by_key(&sorter, true);
	Xapian::MSet mset = enquire.get_mset(0, 10);
	mset_expect_order(mset, 1, 2, 3, 4, 5, 6, 7, 8, 9);
    }

    return true;
}

DEFINE_TESTCASE(multivaluekeymaker1,!backend) {
    const int keys[] = { 0, 1, 2, 3 };
    Xapian::MultiValueKeyMaker sorter(keys, keys + 4);

    Xapian::Document doc;
    TEST(sorter(doc).empty());

    doc.add_value(1, "foo");
    TEST_EQUAL(sorter(doc), string("\0\0foo", 5));
    doc.add_value(1, string("f\0o", 3));
    TEST_EQUAL(sorter(doc), string("\0\0f\0\xffo", 6));
    doc.add_value(3, "xyz");
    TEST_EQUAL(sorter(doc), string("\0\0f\0\xffo\0\0\0\0xyz", 13));

    sorter.add_value(4, true);
    TEST_EQUAL(sorter(doc), string("\0\0f\0\xffo\0\0\0\0xyz\0\0\xff\xff", 17));

    return true;
}

DEFINE_TESTCASE(sortfunctorremote1,remote) {
    Xapian::Enquire enquire(get_database(string()));
    NeverUseMeKeyMaker sorter;
    enquire.set_query(Xapian::Query("word"));
    enquire.set_sort_by_key(&sorter, true);
    TEST_EXCEPTION(Xapian::UnimplementedError,
	Xapian::MSet mset = enquire.get_mset(0, 10);
    );
    return true;
}
