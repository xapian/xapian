/** @file api_sorting.cc
 * @brief tests of MSet sorting
 */
/* Copyright (C) 2007 Olly Betts
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

DEFINE_TESTCASE(sortfunctor1) {
    Xapian::Enquire enquire(get_database("apitest_sortrel"));
    enquire.set_query(Xapian::Query("woman"));

    {
	const int keys[] = { 3, 1 };
	Xapian::MultiValueSorter sorter(keys, keys + 2);

	enquire.set_sort_by_key(&sorter);
	Xapian::MSet mset = enquire.get_mset(0, 10);
	mset_expect_order(mset, 2, 6, 7, 1, 3, 4, 5, 8, 9);
    }

    {
	Xapian::MultiValueSorter sorter;
	sorter.add(3);
	sorter.add(1, false);

	enquire.set_sort_by_key(&sorter);
	Xapian::MSet mset = enquire.get_mset(0, 10);
	mset_expect_order(mset, 7, 6, 2, 8, 9, 4, 5, 1, 3);
    }

    {
	Xapian::MultiValueSorter sorter;
	sorter.add(13); // Value 13 isn't set.
	sorter.add(3);
	sorter.add(1, false);

	enquire.set_sort_by_key(&sorter);
	Xapian::MSet mset = enquire.get_mset(0, 10);
	mset_expect_order(mset, 7, 6, 2, 8, 9, 4, 5, 1, 3);
    }

    {
	Xapian::MultiValueSorter sorter;
	sorter.add(10); // Value 10 isn't always set.
	sorter.add(1, false);

	enquire.set_sort_by_key(&sorter);
	Xapian::MSet mset = enquire.get_mset(0, 10);
	mset_expect_order(mset, 8, 9, 4, 5, 1, 3, 7, 6, 2);
    }

    return true;
}

/// Test cases for MSet sorting.
test_desc sorter_tests[] = {
    TESTCASE(sortfunctor1),
    END_OF_TESTCASES
};
