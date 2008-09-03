/** @file api_percentages.cc
 * @brief Tests of percentage calculations.
 */
/* Copyright 2008 Lemur Consulting Ltd
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

#include "api_percentages.h"

#include <xapian.h>

#include "apitest.h"
#include "testutils.h"

#include <cfloat>

using namespace std;

// #######################################################################
// # Tests start here

class MyPostingSource : public Xapian::PostingSource {
    std::vector<std::pair<Xapian::docid, Xapian::weight> > weights;
    std::vector<std::pair<Xapian::docid, Xapian::weight> >::const_iterator i;
    Xapian::weight maxwt;
    bool started;

  public:
    MyPostingSource()
        : maxwt(0.0), started(false)
    {}

    void append_docweight(Xapian::docid did, Xapian::weight wt)
    {
	weights.push_back(make_pair(did, wt));
	if (wt > maxwt) maxwt = wt;
    }
    void set_maxweight(Xapian::weight wt)
    {
	if (wt > maxwt) maxwt = wt;
    }

    void reset() { started = false; }

    Xapian::weight get_weight() const {
        return i->second;
    }

    Xapian::weight get_maxweight() const {
        return maxwt;
    }

    Xapian::doccount get_termfreq_min() const { return weights.size(); }
    Xapian::doccount get_termfreq_est() const { return weights.size(); }
    Xapian::doccount get_termfreq_max() const { return weights.size(); }

    void next(Xapian::weight wt) {
        (void)wt;
	if (!started) {
	    i = weights.begin();
	    started = true;
	} else {
	    ++i;
	}
    }

    bool at_end() const {
        bool result = (i == weights.end());
	return result;
    }

    Xapian::docid get_docid() const { return i->first; }

    std::string get_description() const {
        return "MyPostingSource";
    }
};


// Test for rounding errors in percentage weight calculations and cutoffs.
DEFINE_TESTCASE(pctcutoff4, backend && !remote && !multi) {
    // Find the number of DBL_EPSILONs to subtract which result in the
    // percentage of the second hit being 49% instead of 50%.
    int epsilons = 0;
    Xapian::Database db(get_database("apitest_simpledata"));
    Xapian::Enquire enquire(db);
    while(true) {
	MyPostingSource source;
	source.append_docweight(1, 100);
	source.append_docweight(2, 50 - epsilons * DBL_EPSILON);
	enquire.set_query(Xapian::Query(&source));
	Xapian::MSet mset = enquire.get_mset(0, 10);
	TEST_EQUAL(mset.size(), 2);
	if (mset[1].get_percent() != 50) break;
	++epsilons;
    }

    {
	// Make a set of document weights including ones on either side of the
	// 49% / 50% boundary.
	MyPostingSource source;
	source.append_docweight(1, 100);
	source.append_docweight(2, 50);
	source.append_docweight(3, 50 - (epsilons - 1) * DBL_EPSILON);
	source.append_docweight(4, 50 - epsilons * DBL_EPSILON);
	source.append_docweight(5, 25);

	enquire.set_query(Xapian::Query(&source));
	Xapian::MSet mset1 = enquire.get_mset(0, 10);
	TEST_EQUAL(mset1.size(), 5);
	TEST_EQUAL(mset1[2].get_percent(), 50);
	TEST_EQUAL(mset1[3].get_percent(), 49);

	// Use various different percentage cutoffs, and check that the values returned are as expected.
	int percent = 100;
	for (Xapian::MSetIterator i = mset1.begin(); i != mset1.end(); ++i) {
	    int new_percent = mset1.convert_to_percent(i);
	    tout << "mset1 item = " << i.get_percent() << "%\n";
	    if (new_percent != percent) {
		enquire.set_cutoff(percent);
		Xapian::MSet mset2 = enquire.get_mset(0, 10);
		tout << "cutoff = " << percent << "%, "
			"mset size = " << mset2.size() << "\n";
		TEST_EQUAL(mset2.size(), i.get_rank());
		percent = new_percent;
	    }
	}
    }

    return true;
}
