/* apitest.cc - test of the OpenMuscat API
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include "config.h"
#include <iostream>
#include <string>
#include <memory>
#include <map>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>

#include "om/om.h"
#include "testsuite.h"
#include "testutils.h"
#include "textfile_indexer.h"
#include "../indexer/index_utils.h"
#include "backendmanager.h"

#define TEST_EXPECTED_DOCS(A,B) do {\
    if ((A).size() != (B).size()) {\
	if (verbose) {\
	    cout << "Match set is of wrong size: was " << (A).size()\
		 << " - expected " << (B).size() << endl;\
	    cout << "Full mset was: " << mymset << endl;\
	}\
	return false;\
    } else {\
	vector<om_docid>::const_iterator i;\
	vector<OmMSetItem>::const_iterator j;\
	for (i = (B).begin(), j = (A).begin();\
	     i != (B).end() && j != (A).end(); i++, j++) {\
	    if (*i != j->did) {\
		if (verbose) {\
		    cout << "Match set didn't contain expected result:" << endl;\
		    cout << "Found docid " << j->did << " expected " << *i <<endl;\
		    cout << "Full mset was: " << mymset << endl;\
		}\
		return false;\
	    }\
	}\
    }\
} while (0)

// tests the allow query terms expand option
bool test_allowqterms1();
// tests that the MSet max_attained works
bool test_maxattain1();
// tests the collapse-on-key
bool test_collapsekey1();
// tests a reversed boolean query
bool test_reversebool1();
// tests a reversed boolean query, where the full mset isn't returned
bool test_reversebool2();
// tests that get_query_terms() returns the terms in the right order
bool test_getqterms1();
// tests that get_matching_terms() returns the terms in the right order
bool test_getmterms1();
// tests that building a query with boolean sub-queries throws an exception.
bool test_boolsubq1();
// tests that specifying a nonexistent input file throws an exception.
bool test_absentfile1();
// tests that query lengths are calculated correctly
bool test_querylen1();
// tests that query lengths are calculated correctly
bool test_querylen2();
// tests that query lengths are calculated correctly
bool test_querylen3();
// tests that the collapsing on termpos optimisation works
bool test_poscollapse1();
// tests that collapsing of queries includes subqueries
bool test_subqcollapse1();
// test that the batch query functionality works
bool test_batchquery1();
// test that running a query twice returns the same results
bool test_repeatquery1();
// test that searching for a term not in the database fails nicely
bool test_absentterm1();
// as absentterm1, but setting query from a vector of terms
bool test_absentterm2();
// test behaviour when creating a query from an empty vector
bool test_emptyquerypart1();

bool floats_are_equal_enough(double a, double b)
{
    if (fabs(a - b) > 1E-5) return false;
    return true;
}

bool weights_are_equal_enough(double a, double b)
{
    if (floats_are_equal_enough(a, b)) return true;

    if(verbose) {
	cout << "Got weight of " << a << ", expected weight of " << b << endl;
    }
    return false;
}


void expect_mset_order(OmMSet mset, om_docid *order,
		       unsigned int ordersize, string mset_name)
{
    TEST_AND_EXPLAIN(mset.items.size() >= ordersize,
		     "Mset " << mset_name << " too small: was " <<
		     mset << ", expected " <<
		     vector<om_docid>(order, order + 2) << endl);
    for (unsigned int i = 0; i < ordersize; i++) {
	TEST_AND_EXPLAIN(mset.items[i].did == order[i],
			 "Mset " << mset_name << " has wrong contents: was " <<
			 mset << ", expected " <<
			 vector<om_docid>(order, order + 2) << endl);
    }
}


OmDatabaseGroup
make_dbgrp(OmDatabase * db1 = 0,
	   OmDatabase * db2 = 0,
	   OmDatabase * db3 = 0,
	   OmDatabase * db4 = 0)
{
    OmDatabaseGroup result;

    if(db1 != 0) result.add_database(*db1);
    if(db2 != 0) result.add_database(*db2);
    if(db3 != 0) result.add_database(*db3);
    if(db4 != 0) result.add_database(*db4);

    return result;
}

static BackendManager backendmanager;

OmDatabase get_database(const string &dbname, const string &dbname2 = "") {
    return backendmanager.get_database(dbname, dbname2);
}

// #######################################################################
// # Tests start here

// always succeeds
bool test_trivial()
{
    return true;
}

// always fails (for testing the framework)
bool test_alwaysfail()
{
    return false;
}

// tests that the backend doesn't return zero docids
bool test_zerodocid()
{
    bool success = true;
    // open the database (in this case a simple text file
    // we prepared earlier)

    OmDatabase mydb(get_database("apitest_onedoc"));

    OmEnquire enquire(make_dbgrp(&mydb));

    // make a simple query, with one word in it - "word".
    OmQuery myquery("word");
    enquire.set_query(myquery);

    // retrieve the top ten results (we only expect one)
    OmMSet mymset = enquire.get_mset(0, 10);

    // We've done the query, now check that the result is what
    // we expect (1 document, with non-zero docid)
    if ((mymset.items.size() != 1) ||
	(mymset.items[0].did == 0)) {
	if (verbose) {
	    cout << "A query on a database returned a zero docid" << endl;
	}
	success = false;
    }
    return success;
}

OmDatabaseGroup get_simple_database()
{
    OmDatabase mydb(get_database("apitest_simpledata"));
    return make_dbgrp(&mydb);
}

void init_simple_enquire(OmEnquire &enq, const OmQuery &query = OmQuery("thi"))
{
    enq.set_query(query);
}

OmMSet do_get_simple_query_mset(OmQuery query, int maxitems = 10, int first = 0)
{
    // open the database (in this case a simple text file
    // we prepared earlier)
    OmEnquire enquire(get_simple_database());
    init_simple_enquire(enquire, query);

    // retrieve the top results
    return enquire.get_mset(first, maxitems);
}

// tests the document count for a simple query
bool test_simplequery1()
{
    bool success = true;
    OmMSet mymset = do_get_simple_query_mset(OmQuery("word"));
    // We've done the query, now check that the result is what
    // we expect (2 documents)
    if (mymset.items.size() != 2) {
	if (verbose) {
	    cout << "The size of the mset was "
		    << mymset.items.size()
		    << ", expected 2." << endl;
	}
	success = false;
    }
    return success;
}

// tests for the right documents and weights returned with simple query
bool test_simplequery2()
{
    bool success = true;
    OmMSet mymset = do_get_simple_query_mset(OmQuery("word"));

    // We've done the query, now check that the result is what
    // we expect (documents 2 and 4)
    if ((mymset.items.size() != 2) ||
	(mymset.items[0].did != 2) ||
	(mymset.items[1].did != 4)) {
	if (verbose) {
	    cout << "Got docids:";
	    for (size_t i=0; i<mymset.items.size(); ++i) {
		if (i != 0) cout << ", ";
		cout << " " << mymset.items[i].did <<
			"(wt " << mymset.items[i].wt << ")";
	    }
	    cout << ", expected 2 and 4." << endl;
	}
	success = false;
    }

    // Check the weights
    if (success &&
	(!weights_are_equal_enough(mymset.items[0].wt, 0.661095) ||
	 !weights_are_equal_enough(mymset.items[1].wt, 0.56982))) {
	success = false;
    }

    return success;
}

// tests for the right document count for another simple query
bool test_simplequery3()
{
    bool success = true;
    // The search is for "thi" rather than "this" because
    // the index will have stemmed versions of the terms.
    OmMSet mymset = do_get_simple_query_mset(OmQuery("thi"));

    // We've done the query, now check that the result is what
    // we expect (documents 2 and 4)
    if (mymset.items.size() != 6) {
	if (verbose) {
	    cout << "Got "
		    << mymset.items.size()
		    << " documents, expected 6" << endl
		    << "Docids matched:";
	    for (size_t i=0; i<mymset.items.size(); ++i) {
		cout << " " << mymset.items[i].did;
	    }
	    cout << "." << endl;
	}
	success = false;
    }

    return success;
}

// tests a query accross multiple databases
bool test_multidb1()
{
    bool success = true;

    OmDatabase mydb1(get_database("apitest_simpledata", "apitest_simpledata2"));
    OmEnquire enquire1(make_dbgrp(&mydb1));

    OmDatabase mydb2(get_database("apitest_simpledata"));
    OmDatabase mydb3(get_database("apitest_simpledata2"));
    OmEnquire enquire2(make_dbgrp(&mydb2, &mydb3));

    // make a simple query, with one word in it - "word".
    OmQuery myquery("word");
    enquire1.set_query(myquery);
    enquire2.set_query(myquery);

    // retrieve the top ten results from each method of accessing
    // multiple text files
    OmMSet mymset1 = enquire1.get_mset(0, 10);
    OmMSet mymset2 = enquire2.get_mset(0, 10);

    if (mymset1.items.size() != mymset2.items.size()) {
	if (verbose) {
	    cout << "Match sets are of different size: "
		    << mymset1.items.size() << "vs." << mymset2.items.size()
		    << endl;
	}
	success = false;
    } else if (!mset_range_is_same_weights(mymset1, 0,
					   mymset2, 0,
					   mymset1.items.size())) {
	if (verbose) {
	    cout << "Match sets don't compare equal:" << endl;
	    cout << mymset1 << "vs." << endl << mymset2 << endl;
	}
	success = false;
    }
    return success;
}

// tests a query accross multiple databases with terms only
// in one of the two databases
bool test_multidb2()
{
    bool success = true;

    OmDatabase mydb1(get_database("apitest_simpledata",
				  "apitest_simpledata2"));
    OmEnquire enquire1(make_dbgrp(&mydb1));

    OmDatabase mydb2(get_database("apitest_simpledata"));
    OmDatabase mydb3(get_database("apitest_simpledata2"));
    OmEnquire enquire2(make_dbgrp(&mydb2, &mydb3));

    // make a simple query
    OmQuery myquery(OM_MOP_OR,
		    OmQuery("inmemory"),
		    OmQuery("word"));
    enquire1.set_query(myquery);
    enquire2.set_query(myquery);

    // retrieve the top ten results from each method of accessing
    // multiple text files
    OmMSet mymset1 = enquire1.get_mset(0, 10);

    OmMSet mymset2 = enquire2.get_mset(0, 10);

    if (mymset1.items.size() != mymset2.items.size()) {
	if (verbose) {
	    cout << "Match sets are of different size: "
		    << mymset1.items.size() << "vs." << mymset2.items.size()
		    << endl;
	}
	success = false;
    } else if (!mset_range_is_same_weights(mymset1, 0,
					   mymset2, 0,
					   mymset1.items.size())) {
	if (verbose) {
	    cout << "Match sets don't compare equal:" << endl;
	    cout << mymset1 << "vs." << endl << mymset2 << endl;
	}
	success = false;
    }
    return success;
}

// tests that changing a query object after calling set_query()
// doesn't make any difference to get_mset().
bool test_changequery1()
{
    bool success = true;
    // The search is for "thi" rather than "this" because
    // the index will have stemmed versions of the terms.
    // open the database (in this case a simple text file
    // we prepared earlier)
    OmEnquire enquire(get_simple_database());

    OmQuery myquery("thi");
    // make a simple query
    enquire.set_query(myquery);

    // retrieve the top ten results
    OmMSet mset1 = enquire.get_mset(0, 10);

    myquery = OmQuery("foo");
    OmMSet mset2 = enquire.get_mset(0, 10);

    // verify that both msets are identical
    if (!(mset1 == mset2)) {
	success = false;
    }
    return success;
}

// tests that a null query throws an exception
bool test_nullquery1()
{
    bool success = false;
    try {
	OmMSet mymset = do_get_simple_query_mset(OmQuery());
    } catch (const OmError &) {
	success = true;
    }
    return success;
}

// tests that when specifiying maxitems to get_mset, no more than
// that are returned.
bool test_msetmaxitems1()
{
    OmMSet mymset = do_get_simple_query_mset(OmQuery("thi"), 1);
    return (mymset.items.size() == 1);
}

// tests that when specifiying maxitems to get_eset, no more than
// that are returned.
bool test_expandmaxitems1()
{
    OmEnquire enquire(get_simple_database());
    init_simple_enquire(enquire);

    OmMSet mymset = enquire.get_mset(0, 10);

    if (mymset.items.size() < 2) return false;

    OmRSet myrset;
    myrset.add_document(mymset.items[0].did);
    myrset.add_document(mymset.items[1].did);

    OmESet myeset = enquire.get_eset(1, myrset);

    return (myeset.items.size() == 1);
}

// tests that a pure boolean query has all weights set to 1
bool test_boolquery1()
{
    bool success = true;
    OmQuery myboolquery(OmQuery(OM_MOP_FILTER,
				OmQuery(),
				OmQuery("thi")));
    OmMSet mymset = do_get_simple_query_mset(myboolquery);

    if (mymset.items.size() == 0) {
	success = false;
	if (verbose) {
	    cout << "bool query returned no items" << endl;
	}
    }
    if (mymset.max_possible != 0) {
        success = false;
	if (verbose) {
	    cout << "Max weight in mset is " << mymset.max_possible <<
		    ", should be 0." << endl;
	}
    } else {
        for (unsigned int i = 0; i<mymset.items.size(); ++i) {
	   if (mymset.items[i].wt != 0) {
	       success = false;
	       if (verbose) {
	           cout << "Item " << i
		        << " in mset has weight "
			<< mymset.items[i].wt
			<< ", should be 0." << endl;
	       }
	       break;
	   }
	}
    }

    return success;
}

// tests that get_mset() specifying "first" works as expected
bool test_msetfirst1()
{
    bool success = true;

    OmMSet mymset1 = do_get_simple_query_mset(OmQuery("thi"), 6, 0);
    OmMSet mymset2 = do_get_simple_query_mset(OmQuery("thi"), 3, 3);

    if (!mset_range_is_same(mymset1, 3, mymset2, 0, 3)) {
        success = false;
    }
    return success;
}

// tests the converting-to-percent functions
bool test_topercent1()
{
    bool success = true;
    OmMSet mymset = do_get_simple_query_mset(OmQuery("thi"), 20, 0);

    int last_pct = 101;
    for (unsigned i=0; i<mymset.items.size(); ++i) {
	int pct = mymset.convert_to_percent(mymset.items[i]);
	if (pct != mymset.convert_to_percent(mymset.items[i].wt)) {
	    success = false;
	    if (verbose) {
		cout << "convert_to_%(msetitem) != convert_to_%(wt)" << endl;
	    }
	} else if ((pct < 0) || (pct > 100)) {
	    success = false;
	    if (verbose) {
	        cout << "percentage out of range: " << pct << endl;
	    }
	} else if (pct > last_pct) {
	    success = false;
	    if (verbose) {
	        cout << "percentage increased over mset" << endl;
	    }
	}
	last_pct = pct;
    }
    return success;
}

class myExpandFunctor : public OmExpandDecider {
    public:
	int operator()(const om_termname & tname) const {
	    unsigned long sum = 0;
	    for (om_termname::const_iterator i=tname.begin(); i!=tname.end(); ++i) {
		sum += *i;
	    }
//	    if (verbose) {
//		cout << tname << "==> " << sum << endl;
//	    }
	    return (sum % 2) == 0;
	}
};

// tests the expand decision functor
bool test_expandfunctor1()
{
    bool success = true;

    OmEnquire enquire(get_simple_database());
    init_simple_enquire(enquire);

    OmMSet mymset = enquire.get_mset(0, 10);
    if (mymset.items.size() < 2) return false;
    
    OmRSet myrset;
    myrset.add_document(mymset.items[0].did);
    myrset.add_document(mymset.items[1].did);

    myExpandFunctor myfunctor;

    OmESet myeset_orig = enquire.get_eset(1000, myrset);
    unsigned int neweset_size = 0;
    for (unsigned int i=0; i<myeset_orig.items.size(); ++i) {
        if (myfunctor(myeset_orig.items[i].tname)) neweset_size++;
    }
    OmESet myeset = enquire.get_eset(neweset_size, myrset, 0, &myfunctor);

    // Compare myeset with the hand-filtered version of myeset_orig.
    if (verbose) {
	cout << "orig_eset: ";
	copy(myeset_orig.items.begin(), myeset_orig.items.end(),
	     ostream_iterator<OmESetItem>(cout, " "));
	cout << endl;
	
	cout << "new_eset: ";
	copy(myeset.items.begin(), myeset.items.end(),
	     ostream_iterator<OmESetItem>(cout, " "));
	cout << endl;
    }
    vector<OmESetItem>::const_iterator orig,filt;
    for (orig=myeset_orig.items.begin(), filt=myeset.items.begin();
         orig!=myeset_orig.items.end() && filt!=myeset.items.end();
	 ++orig, ++filt) {
	// skip over items that shouldn't be in myeset
	while (orig != myeset_orig.items.end() && !myfunctor(orig->tname)) {
	    ++orig;
	}

	if ((orig->tname != filt->tname) ||
	    (orig->wt != filt->wt)) {
	    success = false;
	    if (verbose) {
	        cout << "Mismatch in items "
	             << orig->tname
		     << " vs. "
		     << filt->tname
		     << " after filtering" << endl;
	    }
	    break;
	}
    }
    
    while (orig != myeset_orig.items.end() && !myfunctor(orig->tname)) {
	++orig;
    }

    if (orig != myeset_orig.items.end()) {
	success = false;
	if (verbose) {
	    cout << "Extra items in the non-filtered eset:";
            copy(orig,
		 const_cast<const OmESet &>(myeset_orig).items.end(),
		 ostream_iterator<OmESetItem>(cout, " "));
	    cout << endl;
	}
    } else if (filt != myeset.items.end()) {
        success = false;
	if (verbose) {
	    cout << "Extra items in the filtered eset." << endl;
	}
    }

    return success;
}

class myMatchDecider : public OmMatchDecider {
    public:
        int operator()(const OmDocument *doc) const {
	    // Note that this is not recommended usage of get_data()
	    return strncmp(doc->get_data().value.c_str(), "This is", 7) == 0;
	}
};

// tests the match decision functor
bool test_matchfunctor1()
{
    // FIXME: check that the functor works both ways.
    bool success = true;

    OmEnquire enquire(get_simple_database());
    init_simple_enquire(enquire);

    myMatchDecider myfunctor;

    OmMSet mymset = enquire.get_mset(0, 100, 0, 0, &myfunctor);

    for (unsigned int i=0; i<mymset.items.size(); ++i) {
	const OmDocument doc(enquire.get_doc(mymset.items[i]));
        if (!myfunctor(&doc)) {
	    success = false;
	    break;
	}
    }

    return success;
}

void print_mset_percentages(const OmMSet &mset)
{
    for (unsigned i=0; i<mset.items.size(); ++i) {
        cout << " ";
	cout << mset.convert_to_percent(mset.items[i]);
    }
}

// tests the percent cutoff option
bool test_pctcutoff1()
{
    bool success = true;

    OmEnquire enquire(get_simple_database());
    init_simple_enquire(enquire);

    OmMSet mymset1 = enquire.get_mset(0, 100);

    if (verbose) {
      cout << "Original mset pcts:";
      print_mset_percentages(mymset1);
      cout << endl;
    }

    unsigned int num_items = 0;
    int my_pct = 100;
    int changes = 0;
    for (unsigned int i=0; i<mymset1.items.size(); ++i) {
        int new_pct = mymset1.convert_to_percent(mymset1.items[i]);
        if (new_pct != my_pct) {
	    changes++;
	    if (changes <= 3) {
	        num_items = i;
		my_pct = new_pct;
	    }
	}
    }

    if (changes <= 3) {
	success = false;
        if (verbose) {
	    cout << "MSet not varied enough to test" << endl;
	}
    }
    if (verbose) {
        cout << "Cutoff percent: " << my_pct << endl;
    }
    
    OmMatchOptions mymopt;
    mymopt.set_percentage_cutoff(my_pct);
    OmMSet mymset2 = enquire.get_mset(0, 100, 0, &mymopt);

    if (verbose) {
        cout << "Percentages after cutoff:";
	print_mset_percentages(mymset2);
        cout << endl;
    }
    
    if (mymset2.items.size() < num_items) {
        success = false;
	if (verbose) {
	    cout << "Match with % cutoff lost too many items" << endl;
	}
    } else if (mymset2.items.size() > num_items) {
        for (unsigned int i=num_items; i<mymset2.items.size(); ++i) {
	    if (mymset2.convert_to_percent(mymset2.items[i]) != my_pct) {
	        success = false;
		if (verbose) {
		    cout << "Match with % cutoff returned "
		            " too many items" << endl;
		}
		break;
	    }
	}
    }

    return success;
}

bool test_allowqterms1()
{
    bool success = true;

    OmEnquire enquire(get_simple_database());
    init_simple_enquire(enquire);

    OmMSet mymset = enquire.get_mset(0, 10);
    if (mymset.items.size() < 2) return false;

    OmRSet myrset;
    myrset.add_document(mymset.items[0].did);
    myrset.add_document(mymset.items[1].did);

    OmExpandOptions eopt;
    eopt.set_use_query_terms(false);

    OmESet myeset = enquire.get_eset(1000, myrset, &eopt);

    for (unsigned i=0; i<myeset.items.size(); ++i) {
        if (myeset.items[i].tname == "thi") {
	    success = false;
	    if (verbose) {
	        cout << "Found query term `"
		     << myeset.items[i].tname
		     << "' in expand set" << endl;
	    }
	    break;
	}
    }

    return success;
}

bool test_maxattain1()
{
    bool success = true;

    OmMSet mymset = do_get_simple_query_mset(OmQuery("thi"), 100, 0);
    
    om_weight mymax = 0;
    for (unsigned i=0; i<mymset.items.size(); ++i) {
        if (mymset.items[i].wt > mymax) {
	    mymax = mymset.items[i].wt;
	}
    }
    if (mymax != mymset.max_attained) {
        success = false;
	if (verbose) {
	    cout << "Max weight in MSet is " << mymax
	         << ", max_attained = " << mymset.max_attained << endl;
        }
    }

    return success;
}

bool test_collapsekey1()
{
    bool success = true;

    OmEnquire enquire(get_simple_database());
    init_simple_enquire(enquire);

    OmMatchOptions mymopt;

    OmMSet mymset1 = enquire.get_mset(0, 100, 0, &mymopt);
    om_doccount mymsize1 = mymset1.items.size();

    for (int key_no = 1; key_no<7; ++key_no) {
	mymopt.set_collapse_key(key_no);
	OmMSet mymset = enquire.get_mset(0, 100, 0, &mymopt);

	if(mymsize1 <= mymset.items.size()) {
	    success = false;
	    if (verbose) {
		cout << "Had no fewer items when performing collapse: don't know whether it worked." << endl;
	    }
	}

        map<string, om_docid> keys;
	for (vector<OmMSetItem>::const_iterator i=mymset.items.begin();
	     i != mymset.items.end();
	     ++i) {
	    OmKey key = enquire.get_doc(*i).get_key(key_no);
	    if (key.value != i->collapse_key.value) {
		success = false;
		if (verbose) {
		    cout << "Expected key value was not found in MSetItem: "
			 << "expected `" << key.value
			 << "' found `" << i->collapse_key.value
			 << "'" << endl;

		}
	    }
	    if (keys[key.value] != 0 && key.value != "") {
	        success = false;
		if (verbose) {
		    cout << "docids " << keys[key.value]
		         << " and " << i->did
			 << " both found in MSet with key `" << key.value
			 << "'" << endl;
		}
		break;
	    } else {
	        keys[key.value] = i->did;
	    }
	}
	// don't bother continuing if we've already failed.
	if (!success) break;
    }

    return success;
}

bool test_reversebool1()
{
    OmEnquire enquire(get_simple_database());
    OmQuery query("thi");
    query.set_bool(true);
    init_simple_enquire(enquire, query);

    OmMatchOptions mymopt;
    OmMSet mymset1 = enquire.get_mset(0, 100, 0, &mymopt);
    mymopt.set_sort_forward();
    OmMSet mymset2 = enquire.get_mset(0, 100, 0, &mymopt);
    mymopt.set_sort_forward(false);
    OmMSet mymset3 = enquire.get_mset(0, 100, 0, &mymopt);

    if(mymset1.items.size() == 0) {
	if (verbose) cout << "Mset was empty" << endl;
	return false;
    }

    // mymset1 and mymset2 should be identical
    if(mymset1.items.size() != mymset2.items.size()) {
	if (verbose) {
	    cout << "mymset1 and mymset2 were of different sizes (" <<
		    mymset1.items.size() << " and " <<
		    mymset2.items.size() << ")" << endl;
	}
	return false;
    }

    {
	vector<OmMSetItem>::const_iterator i;
	vector<OmMSetItem>::const_iterator j;
	for (i = mymset1.items.begin(), j = mymset2.items.begin();
	     i != mymset1.items.end(), j != mymset2.items.end();
	     ++i, j++) {
	    if(i->did != j->did) {
		if (verbose) {
		    cout << "Calling OmMatchOptions::set_sort_forward() was not"
			    "same as default." << endl;
		    cout << "docids " << i->did << " and " << j->did <<
			    " should have been the same" << endl;
		}
		return false;
	    }
	}
    }

    // mymset1 and mymset3 should be same but reversed
    if(mymset1.items.size() != mymset3.items.size()) {
	if (verbose) {
	    cout << "mymset1 and mymset2 were of different sizes (" <<
		    mymset1.items.size() << " and " <<
		    mymset2.items.size() << ")" << endl;
	}
	return false;
    }

    {
	vector<OmMSetItem>::const_iterator i;
	vector<OmMSetItem>::reverse_iterator j;
	for (i = mymset1.items.begin(),
	     j = mymset3.items.rbegin();
	     i != mymset1.items.end();
	     ++i, j++) {
	    if(i->did != j->did) {
		if (verbose) {
		    cout << "Calling OmMatchOptions::set_sort_forward(false) "
			    "did not reverse results." << endl;
		    cout << "docids " << i->did << " and " << j->did <<
			    " should have been the same" << endl;
		}
		return false;
	    }
	}
    }

    return true;
}

bool test_reversebool2()
{
    OmEnquire enquire(get_simple_database());
    OmQuery query("thi");
    query.set_bool(true);
    init_simple_enquire(enquire, query);

    OmMatchOptions mymopt;
    OmMSet mymset1 = enquire.get_mset(0, 100, 0, &mymopt);

    if(mymset1.items.size() == 0) {
	if (verbose) cout << "Mset was empty" << endl;
	return false;
    }
    if(mymset1.items.size() == 1) {
	if (verbose) cout << "Mset was too small to test properly" << endl;
	return false;
    }


    mymopt.set_sort_forward();
    om_doccount msize = mymset1.items.size() / 2;
    OmMSet mymset2 = enquire.get_mset(0, msize, 0, &mymopt);
    mymopt.set_sort_forward(false);
    OmMSet mymset3 = enquire.get_mset(0, msize, 0, &mymopt);

    // mymset2 should be first msize items of mymset1
    if(msize != mymset2.items.size()) return false;
    {
	vector<OmMSetItem>::const_iterator i;
	vector<OmMSetItem>::const_iterator j;
	for (i = mymset1.items.begin(), j = mymset2.items.begin();
	     i != mymset1.items.end(), j != mymset2.items.end();
	     ++i, j++) {
	    if(i->did != j->did) {
		if (verbose) {
		    cout << "Calling OmMatchOptions::set_sort_forward() was not"
			    "same as default." << endl;
		    cout << "docids " << i->did << " and " << j->did <<
			    " should have been the same" << endl;
		}
		return false;
	    }
	}
    }

    // mymset3 should be last msize items of mymset1, in reverse order
    if(msize != mymset3.items.size()) return false;
    {
	vector<OmMSetItem>::reverse_iterator i;
	vector<OmMSetItem>::const_iterator j;
	for (i = mymset1.items.rbegin(),
	     j = mymset3.items.begin();
	     j != mymset3.items.end();
	     ++i, j++) {
	    if(i->did != j->did) {
		if (verbose) {
		    cout << "Calling OmMatchOptions::set_sort_forward(false) "
			    "did not reverse results." << endl;
		    cout << "docids " << i->did << " and " << j->did <<
			    " should have been the same" << endl;
		}
		return false;
	    }
	}
    }

    return true;
}

bool test_getqterms1()
{
    bool success;

    static string answers[4] = {
	"one",
	"two",
	"three",
	"four"
    };

    OmQuery myquery(OM_MOP_OR,
	    OmQuery(OM_MOP_AND,
		    OmQuery("one", 1, 1),
		    OmQuery("three", 1, 3)),
	    OmQuery(OM_MOP_OR,
		    OmQuery("four", 1, 4),
		    OmQuery("two", 1, 2)));

    om_termname_list terms = myquery.get_terms();

    success = (terms == om_termname_list(answers, answers+4));
    if (verbose && !success) {
	cout << "Terms returned in incorrect order: ";
	copy(terms.begin(),
	     terms.end(),
	     ostream_iterator<om_termname>(cout, " "));
	cout << endl << "Expected: one two three four" << endl;
    }

    return success;
}

bool test_getmterms1()
{
    bool success = true;
    
    static string answers[4] = {
	"one",
	"two",
	"three",
	"four"
    };

    OmDatabase mydb(get_database("apitest_termorder"));
    OmEnquire enquire(make_dbgrp(&mydb));

    OmQuery myquery(OM_MOP_OR,
	    OmQuery(OM_MOP_AND,
		    OmQuery("one", 1, 1),
		    OmQuery("three", 1, 3)),
	    OmQuery(OM_MOP_OR,
		    OmQuery("four", 1, 4),
		    OmQuery("two", 1, 2)));

    enquire.set_query(myquery);

    OmMSet mymset = enquire.get_mset(0, 10);

    if (mymset.items.size() != 1) {
	success = false;
	if (verbose) {
	    cout << "Expected one match, but got " << mymset.items.size()
		 << "!" << endl;
	}
    } else {
	om_termname_list mterms = enquire.get_matching_terms(mymset.items[0]);
	if (mterms != om_termname_list(answers, answers+4)) {
	    success = false;
	    if (verbose) {
		cout << "Terms returned in incorrect order: ";
		copy(mterms.begin(),
		     mterms.end(),
		     ostream_iterator<om_termname>(cout, " "));
		cout << endl << "Expected: one two three four" << endl;
	    }
	}

    }

    return success;
}

bool test_boolsubq1()
{
    bool success = false;

    OmQuery mybool("foo");
    mybool.set_bool(true);

    try {
	OmQuery query(OM_MOP_OR,
		      OmQuery("bar"),
		      mybool);
    } catch (OmInvalidArgumentError &) {
	success = true;
    }

    return success;
}

bool test_absentfile1()
{
    bool success = false;

    try {
	OmDatabase mydb(get_database("/this_does_not_exist"));
	OmEnquire enquire(make_dbgrp(&mydb));

	OmQuery myquery("cheese");
	enquire.set_query(myquery);

	OmMSet mymset = enquire.get_mset(0, 10);
    } catch (OmOpeningError &) {
	success = true;
    }

    return success;
}

bool test_querylen1()
{
    // test that a null query has length 0
    bool success = (OmQuery().get_length()) == 0;

    return success;
}

bool test_querylen2()
{
    // test that a simple query has the right length
    bool success = true;

    OmQuery myquery;

    myquery = OmQuery(OM_MOP_OR,
		      OmQuery("foo"),
		      OmQuery("bar"));
    myquery = OmQuery(OM_MOP_AND,
		      myquery,
		      OmQuery(OM_MOP_OR,
			      OmQuery("wibble"),
			      OmQuery("spoon")));

    if (myquery.get_length() != 4) {
	success = false;
	if (verbose) {
	    cout << "Query had length "
		 << myquery.get_length()
		 << ", expected 4" << endl;
	}
    }

    return success;
}

bool test_querylen3()
{
    bool success = true;

    // test with an even bigger and strange query

    om_termname terms[3] = {
	"foo",
	"bar",
	"baz"
    };
    OmQuery queries[3] = {
	OmQuery("wibble"),
	OmQuery("wobble"),
	OmQuery(OM_MOP_OR, std::string("jelly"), std::string("belly"))
    };

    OmQuery myquery;
    vector<om_termname> v1(terms, terms+3);
    vector<OmQuery> v2(queries, queries+3);
    vector<OmQuery *> v3;

    auto_ptr<OmQuery> dynquery1(new OmQuery(OM_MOP_AND,
					    std::string("ball"),
					    std::string("club")));
    auto_ptr<OmQuery> dynquery2(new OmQuery("ring"));
    v3.push_back(dynquery1.get());
    v3.push_back(dynquery2.get());
    
    OmQuery myq1 = OmQuery(OM_MOP_AND, v1.begin(), v1.end());
    if (myq1.get_length() != 3) {
	success = false;
	if (verbose) {
	    cout << "Query myq1 length is "
		    << myq1.get_length()
		    << ", expected 3.  Description: "
		    << myq1.get_description() << endl;
	}
    }

    OmQuery myq2_1 = OmQuery(OM_MOP_OR, v2.begin(), v2.end());
    if (myq2_1.get_length() != 4) {
	success = false;
	if (verbose) {
	    cout << "Query myq2_1 length is "
		    << myq2_1.get_length()
		    << ", expected 4.  Description: "
		    << myq2_1.get_description() << endl;
	}
    }

    OmQuery myq2_2 = OmQuery(OM_MOP_AND, v3.begin(), v3.end());
    if (myq2_2.get_length() != 3) {
	success = false;
	if (verbose) {
	    cout << "Query myq2_2 length is "
		    << myq2_2.get_length()
		    << ", expected 3.  Description: "
		    << myq2_2.get_description() << endl;
	}
    }

    OmQuery myq2 = OmQuery(OM_MOP_OR, myq2_1, myq2_2);
    if (myq2.get_length() != 7) {
	success = false;
	if (verbose) {
	    cout << "Query myq2 length is "
		    << myq2.get_length()
		    << ", expected 7.  Description: "
		    << myq2.get_description() << endl;
	}
    }

    myquery = OmQuery(OM_MOP_OR, myq1, myq2);
    if (myquery.get_length() != 10) {
	success = false;
	if (verbose) {
	    cout << "Query length is "
		 << myquery.get_length()
		 << ", expected 9"
		 << endl;
	    cout << "Query is: "
		 << myquery.get_description()
		 << endl;
	}
    }

    return success;
}

bool test_poscollapse1()
{
    bool success = true;

    OmQuery myquery1 = OmQuery(OM_MOP_OR,
			       OmQuery("thi", 1),
			       OmQuery("thi", 1));
    OmQuery myquery2 = OmQuery("thi", 2, 1);

    if (verbose) {
	cout << myquery1.get_description() << endl;
	cout << myquery2.get_description() << endl;
    }

    OmMSet mymset1 = do_get_simple_query_mset(myquery1);
    OmMSet mymset2 = do_get_simple_query_mset(myquery2);

    if (mymset1 != mymset2) {
	success = false;

	if (verbose) {
	    cout << "MSets different" << endl;
	}
    }

    return success;
}

bool test_subqcollapse1()
{
    bool success = true;

    OmQuery queries1[3] = {
	OmQuery("wibble"),
	OmQuery("wobble"),
	OmQuery(OM_MOP_OR, std::string("jelly"), std::string("belly"))
    };  

    OmQuery queries2[3] = {
	OmQuery(OM_MOP_AND, std::string("jelly"), std::string("belly")),
	OmQuery("wibble"),
	OmQuery("wobble")
    };  

    vector<OmQuery> vec1(queries1, queries1+3);
    OmQuery myquery1(OM_MOP_OR, vec1.begin(), vec1.end());
    string desc1 = myquery1.get_description();

    vector<OmQuery> vec2(queries2, queries2+3);
    OmQuery myquery2(OM_MOP_AND, vec2.begin(), vec2.end());
    string desc2 = myquery2.get_description();

    if(desc1 != "(wibble OR wobble OR jelly OR belly)") {
	success = false;
	if(verbose)
	    cout << "Failed to correctly collapse query: got `" <<
		    desc1 << "'" << endl;
    }

    if(desc2 != "(jelly AND belly AND wibble AND wobble)") {
	success = false;
	if(verbose)
	    cout << "Failed to correctly collapse query: got `" <<
		    desc2 << "'" << endl;
    }

    return success;
}

bool test_batchquery1()
{
    bool success = true;

    OmBatchEnquire::query_desc mydescs[3] = {
	{ OmQuery("thi"), 0, 10, 0, 0, 0},
	{ OmQuery(), 0, 10, 0, 0, 0},
	{ OmQuery("word"), 0, 10, 0, 0, 0}};


    OmBatchEnquire benq(get_simple_database());

    OmBatchEnquire::query_batch myqueries(mydescs, mydescs+3);

    benq.set_queries(myqueries);

    OmBatchEnquire::mset_batch myresults = benq.get_msets();

    if (myresults.size() != 3) {
	success = false;
	if (verbose) {
	    cout << "Results size is "
		 << myresults.size()
		 << ", expected 3."
		 << endl;
	}
    } else {
	if (myresults[0].value() !=
	    do_get_simple_query_mset(OmQuery("thi"))) {
	    success = false;
	    if (verbose) {
		cout << "Query 1 returned different result!" << endl;
	    }
	}
 	if (myresults[1].is_valid()) {
	    success = false;
	    if (verbose) {
		cout << "Query 2 should not be valid" << endl;
	    }
	}
	try {
	    OmMSet unused = myresults[1].value();
	    // should have thrown an exception by now.
	    success = false;
	    if (verbose) {
		cout << "Query 2 mset didn't throw an exception" << endl;
	    }
	} catch (OmInvalidResultError &) {
	}

	if (myresults[2].value() !=
	    do_get_simple_query_mset(OmQuery("word"))) {
	    success = false;
	    if (verbose) {
		cout << "Query 3 returned different result!" << endl;
	    }
	}
    }

    return success;
}

bool test_repeatquery1()
{
    bool success = true;

    OmEnquire enquire(get_simple_database());
    init_simple_enquire(enquire);

    OmQuery myquery(OM_MOP_OR,
		    OmQuery("thi"),
		    OmQuery("word"));
    enquire.set_query(myquery);
    
    OmMSet mymset1 = enquire.get_mset(0, 10);
    OmMSet mymset2 = enquire.get_mset(0, 10);

    if (mymset1 != mymset2) {
	success = false;

	if (verbose) {
	    cout << "MSets are different." << endl;
	}
    }

    return success;
}

bool test_absentterm1()
{
    bool success = true;

    OmEnquire enquire(get_simple_database());
    OmQuery query("frink");
    query.set_bool(true);
    init_simple_enquire(enquire, query);

    OmMSet mymset = enquire.get_mset(0, 10);
    if (mymset.items.size() != 0) {
	success = false;

	if(verbose) {
	    cout << "Expected no items in mset: found " <<
		    mymset.items.size() << endl;
	}
    }

    return success;
}

bool test_absentterm2()
{
    bool success = true;

    OmEnquire enquire(get_simple_database());
    vector<om_termname> terms;
    terms.push_back("frink");

    OmQuery query(OM_MOP_OR, terms.begin(), terms.end());
    enquire.set_query(query);

    OmMSet mymset = enquire.get_mset(0, 10);
    if (mymset.items.size() != 0) {
	success = false;

	if(verbose) {
	    cout << "Expected no items in mset: found " <<
		    mymset.items.size() << endl;
	}
    }

    return success;
}

bool test_emptyquerypart1()
{
    vector<om_termname> emptyterms;
    OmQuery query(OM_MOP_OR, emptyterms.begin(), emptyterms.end());

    return true;
}

bool test_stemlangs()
{
    vector<string> langs;
    langs = OmStem::get_available_languages();

    if(verbose) {
	vector<string>::const_iterator i;
	for(i = langs.begin(); i != langs.end(); i++) {
	    if (i != langs.begin()) cout << ", ";
	    cout << *i;
	}
	cout << endl;
    }

    return true;
}

// test that a multidb with 2 dbs query returns correct docids
bool test_multidb3()
{
    OmDatabase mydb2(get_database("apitest_simpledata"));
    OmDatabase mydb3(get_database("apitest_simpledata2"));
    OmEnquire enquire(make_dbgrp(&mydb2, &mydb3));

    // make a query
    OmQuery myquery(OM_MOP_OR,
		    OmQuery("inmemory"),
		    OmQuery("word"));
    myquery.set_bool(true);
    enquire.set_query(myquery);

    // retrieve the top ten results
    OmMSet mymset = enquire.get_mset(0, 10);

    vector<om_docid> expected_docs;
    expected_docs.push_back(2);
    expected_docs.push_back(3);
    expected_docs.push_back(7);

    TEST_EXPECTED_DOCS(mymset.items, expected_docs);

    return true;
}

// test that a multidb with 3 dbs query returns correct docids
bool test_multidb4()
{
    OmDatabase mydb2(get_database("apitest_simpledata"));
    OmDatabase mydb3(get_database("apitest_simpledata2"));
    OmDatabase mydb4(get_database("apitest_termorder"));
    OmEnquire enquire(make_dbgrp(&mydb2, &mydb3, &mydb4));

    // make a query
    OmQuery myquery(OM_MOP_OR,
		    OmQuery("inmemory"),
		    OmQuery("word"));
    myquery.set_bool(true);
    enquire.set_query(myquery);

    // retrieve the top ten results
    OmMSet mymset = enquire.get_mset(0, 10);

    vector<om_docid> expected_docs;
    expected_docs.push_back(2);
    expected_docs.push_back(3);
    expected_docs.push_back(4);
    expected_docs.push_back(10);
    
    TEST_EXPECTED_DOCS(mymset.items, expected_docs);

    return true;
}

// test that rsets do sensible things
bool test_rset1()
{
    bool success = true;

    OmDatabase mydb(get_database("apitest_rset"));

    OmEnquire enquire(make_dbgrp(&mydb));

    OmQuery myquery(OM_MOP_OR,
		    OmQuery("giraff"),
		    OmQuery("tiger"));

    enquire.set_query(myquery);

    OmMSet mymset1 = enquire.get_mset(0, 10);

    OmRSet myrset;
    myrset.add_document(1);

    OmMSet mymset2 = enquire.get_mset(0, 10, &myrset);

    // We should have the same documents turn up, but 1 and 3 should
    // have higher weights with the RSet.
    if (mymset1.items.size() != 3 ||
	mymset2.items.size() != 3) {
	if (verbose) {
	    cout << "MSets are of different size: " << endl;
	    cout << "mset1: " << mymset1 << endl;
	    cout << "mset2: " << mymset2 << endl;
	}
	success = false;
    }

    return success;
}

// test that rsets do more sensible things
bool test_rset2()
{
    bool success = true;

    OmDatabase mydb(get_database("apitest_rset"));

    OmEnquire enquire(make_dbgrp(&mydb));
    OmStem stemmer("english");
    
    OmQuery myquery(OM_MOP_OR,
		    OmQuery(stemmer.stem_word("cuddly")),
		    OmQuery(stemmer.stem_word("people")));

    enquire.set_query(myquery);

    OmMSet mymset1 = enquire.get_mset(0, 10);

    OmRSet myrset;
    myrset.add_document(2);

    OmMSet mymset2 = enquire.get_mset(0, 10, &myrset);

    TEST_MSET_SIZE(mymset1, 2);
    TEST_MSET_SIZE(mymset2, 2);

    om_docid order1[] = {1, 2};
    om_docid order2[] = {2, 1};

    expect_mset_order(mymset1, order1, 2, "mymset1");
    expect_mset_order(mymset2, order2, 2, "mymset2");

    return success;
}

// test that rsets behave correctly with multiDBs
bool test_rsetmultidb1()
{
    OmDatabase mydb1(get_database("apitest_rset", "apitest_simpledata2"));
    OmDatabase mydb2(get_database("apitest_rset"));
    OmDatabase mydb3(get_database("apitest_simpledata2"));

    OmEnquire enquire1(make_dbgrp(&mydb1));
    OmEnquire enquire2(make_dbgrp(&mydb2, &mydb3));

    OmStem stemmer("english");
    OmQuery myquery(OM_MOP_OR,
		    OmQuery(stemmer.stem_word("cuddly")),
		    OmQuery(stemmer.stem_word("multiple")));

    enquire1.set_query(myquery);
    enquire2.set_query(myquery);

    OmRSet myrset1;
    OmRSet myrset2;
    myrset1.add_document(4);
    myrset2.add_document(2);

    OmMSet mymset1a = enquire1.get_mset(0, 10);
    OmMSet mymset1b = enquire1.get_mset(0, 10, &myrset1);
    OmMSet mymset2a = enquire2.get_mset(0, 10);
    OmMSet mymset2b = enquire2.get_mset(0, 10, &myrset2);

    TEST_EQUAL(mymset1a.items.size(), 2);
    TEST_EQUAL(mymset1b.items.size(), 2);
    TEST_EQUAL(mymset2a.items.size(), 2);
    TEST_EQUAL(mymset2b.items.size(), 2);

    om_docid order1a[] = {1, 4};
    om_docid order1b[] = {4, 1};
    om_docid order2a[] = {1, 2};
    om_docid order2b[] = {2, 1};

    expect_mset_order(mymset1a, order1a, 2, "mymset1a");
    expect_mset_order(mymset1b, order1b, 2, "mymset1b");
    expect_mset_order(mymset2a, order2a, 2, "mymset2a");
    expect_mset_order(mymset2b, order2b, 2, "mymset2b");

    mset_range_is_same_weights(mymset1a, 0, mymset2a, 0, 2);
    mset_range_is_same_weights(mymset1b, 0, mymset2b, 0, 2);
    TEST_NOT_EQUAL(mymset1a, mymset1b);
    TEST_NOT_EQUAL(mymset2a, mymset2b);

    return true;
}

// test that rsets behave correctly with multiDBs
bool test_rsetmultidb2()
{
    OmDatabase mydb1(get_database("apitest_rset", "apitest_simpledata2"));
    OmDatabase mydb2(get_database("apitest_rset"));
    OmDatabase mydb3(get_database("apitest_simpledata2"));

    OmEnquire enquire1(make_dbgrp(&mydb1));
    OmEnquire enquire2(make_dbgrp(&mydb2, &mydb3));

    OmStem stemmer("english");
    OmQuery myquery(stemmer.stem_word("is"));

    enquire1.set_query(myquery);
    enquire2.set_query(myquery);

    OmRSet myrset1;
    OmRSet myrset2;
    myrset1.add_document(4);
    myrset2.add_document(2);

    OmMSet mymset1a = enquire1.get_mset(0, 10);
    OmMSet mymset1b = enquire1.get_mset(0, 10, &myrset1);
    OmMSet mymset2a = enquire2.get_mset(0, 10);
    OmMSet mymset2b = enquire2.get_mset(0, 10, &myrset2);

    TEST_EQUAL(mymset1a.items.size(), 2);
    TEST_EQUAL(mymset1b.items.size(), 2);
    TEST_EQUAL(mymset2a.items.size(), 2);
    TEST_EQUAL(mymset2b.items.size(), 2);

    om_docid order1a[] = {4, 3};
    om_docid order1b[] = {4, 3};
    om_docid order2a[] = {2, 5};
    om_docid order2b[] = {2, 5};

    expect_mset_order(mymset1a, order1a, 2, "mymset1a");
    expect_mset_order(mymset1b, order1b, 2, "mymset1b");
    expect_mset_order(mymset2a, order2a, 2, "mymset2a");
    expect_mset_order(mymset2b, order2b, 2, "mymset2b");

    mset_range_is_same_weights(mymset1a, 0, mymset2a, 0, 2);
    mset_range_is_same_weights(mymset1b, 0, mymset2b, 0, 2);
    TEST_NOT_EQUAL(mymset1a, mymset1b);
    TEST_NOT_EQUAL(mymset2a, mymset2b);

    return true;
}

/// Simple test of the set_max_or_terms() match option.
bool test_maxorterms1()
{
    OmDatabase mydb(get_database("apitest_simpledata"));
    OmEnquire enquire(make_dbgrp(&mydb));

    OmStem stemmer("english");
    OmQuery myquery1(stemmer.stem_word("word"));

    OmQuery myquery2(OM_MOP_OR,
		    OmQuery(stemmer.stem_word("simple")),
		    OmQuery(stemmer.stem_word("word")));

    enquire.set_query(myquery1);
    OmMSet mymset1 = enquire.get_mset(0, 10);

    enquire.set_query(myquery2);
    OmMatchOptions moptions;
    moptions.set_max_or_terms(1);
    OmMSet mymset2 = enquire.get_mset(0, 10, 0, &moptions);

    TEST_EQUAL(mymset1, mymset2);
    
    return true;
}

/// Test the set_max_or_terms() match option works if the OR contains
/// sub-expressions (regression test)
bool test_maxorterms2()
{
    OmDatabase mydb(get_database("apitest_simpledata"));
    OmEnquire enquire(make_dbgrp(&mydb));

    OmStem stemmer("english");

    OmQuery myquery1(OM_MOP_AND,
		     OmQuery(stemmer.stem_word("word")),
		     OmQuery(stemmer.stem_word("search")));

    OmQuery myquery2(OM_MOP_OR,
		     OmQuery(stemmer.stem_word("this")),
		     OmQuery(OM_MOP_AND,
			     OmQuery(stemmer.stem_word("word")),
			     OmQuery(stemmer.stem_word("search"))));

    enquire.set_query(myquery1);
    OmMSet mymset1 = enquire.get_mset(0, 10);

    enquire.set_query(myquery2);
    OmMatchOptions moptions;
    moptions.set_max_or_terms(1);
    OmMSet mymset2 = enquire.get_mset(0, 10, 0, &moptions);

    TEST_EQUAL(mymset1, mymset2);
    
    return true;
}

/// Test that the termfreq returned by termlists is correct.
bool test_termlisttermfreq()
{
    OmDatabase mydb(get_database("apitest_simpledata"));
    OmEnquire enquire(make_dbgrp(&mydb));
    OmStem stemmer("english");
    OmRSet rset1;
    OmRSet rset2;
    rset1.add_document(5);
    rset2.add_document(6);

    OmESet eset1 = enquire.get_eset(1000, rset1);
    OmESet eset2 = enquire.get_eset(1000, rset2);

    // search for weight of term 'another'
    string theterm = stemmer.stem_word("another");

    vector<OmESetItem>::const_iterator i;
    om_weight wt1 = 0;
    om_weight wt2 = 0;
    for(i = eset1.items.begin(); i != eset1.items.end(); i++) {
	if(i->tname == theterm) {
	    wt1 = i->wt;
	    break;
	}
    }
    for(i = eset2.items.begin(); i != eset2.items.end(); i++) {
	if(i->tname == theterm) {
	    wt2 = i->wt;
	    break;
	}
    }
    
    TEST_NOT_EQUAL(wt1, 0);
    TEST_NOT_EQUAL(wt2, 0);
    TEST_EQUAL(wt1, wt2);

    return true;
}

// tests an expand accross multiple databases
bool test_multiexpand1()
{
    OmDatabase mydb1(get_database("apitest_simpledata", "apitest_simpledata2"));
    OmEnquire enquire1(make_dbgrp(&mydb1));

    OmDatabase mydb2(get_database("apitest_simpledata"));
    OmDatabase mydb3(get_database("apitest_simpledata2"));
    OmEnquire enquire2(make_dbgrp(&mydb2, &mydb3));

    // make simple equivalent rsets, with a document from each database in each.
    OmRSet rset1;
    OmRSet rset2;
    rset1.add_document(1);
    rset1.add_document(7);
    rset2.add_document(1);
    rset2.add_document(2);

    // retrieve the top ten results from each method of accessing
    // multiple text files

    // This is the single database one.
    OmESet eset1 = enquire1.get_eset(1000, rset1);

    // This is the multi database with approximation
    OmESet eset2 = enquire2.get_eset(1000, rset2);

    OmExpandOptions eopts;
    eopts.set_use_exact_termfreq(true);
    // This is the multi database without approximation
    OmESet eset3 = enquire2.get_eset(1000, rset2, &eopts);

    TEST_EQUAL(eset1.items.size(), eset2.items.size());
    TEST_EQUAL(eset1.items.size(), eset3.items.size());

    vector<OmESetItem>::const_iterator i;
    vector<OmESetItem>::const_iterator j;
    vector<OmESetItem>::const_iterator k;
    bool all_iwts_equal_jwts = true;
    for(i = eset1.items.begin(), j = eset2.items.begin(),
	k = eset3.items.begin();
	i != eset1.items.end(), j != eset2.items.end(), k != eset3.items.end();
	i++, j++, k++) {
	if (i->wt != j->wt) all_iwts_equal_jwts = false;
	TEST_EQUAL(i->wt, k->wt);
	TEST_EQUAL(i->tname, k->tname);
    }
    TEST(!all_iwts_equal_jwts);
    return true;
}

/// Simple test of NEAR
bool test_near1()
{
    OmDatabase mydb(get_database("apitest_phrase"));
    OmEnquire enquire(make_dbgrp(&mydb));
    OmStem stemmer("english");

    try {
	// make a query
	vector<OmQuery> subqs;
	vector<om_docid> expected_docs;
	subqs.push_back(OmQuery(stemmer.stem_word("phrase")));
	subqs.push_back(OmQuery(stemmer.stem_word("near")));
	enquire.set_query(OmQuery(OM_MOP_NEAR, subqs.begin(), subqs.end(), 2));

	// retrieve the top ten results
	OmMSet mymset = enquire.get_mset(0, 10);
	expected_docs.push_back(3);
    
	TEST_EXPECTED_DOCS(mymset.items, expected_docs);

	subqs.clear();
	expected_docs.clear();
	
	subqs.push_back(OmQuery(stemmer.stem_word("phrase")));
	subqs.push_back(OmQuery(stemmer.stem_word("near")));
	enquire.set_query(OmQuery(OM_MOP_NEAR, subqs.begin(), subqs.end(), 3));

	// retrieve the top ten results
	mymset = enquire.get_mset(0, 10);
	expected_docs.push_back(1);
	expected_docs.push_back(3);
    
	TEST_EXPECTED_DOCS(mymset.items, expected_docs);

	subqs.clear();
	expected_docs.clear();
	
	subqs.push_back(OmQuery(stemmer.stem_word("phrase")));
	subqs.push_back(OmQuery(stemmer.stem_word("near")));
	enquire.set_query(OmQuery(OM_MOP_NEAR, subqs.begin(), subqs.end(), 5));

	// retrieve the top ten results
	mymset = enquire.get_mset(0, 10);
	expected_docs.push_back(1);
	expected_docs.push_back(3);
    
	TEST_EXPECTED_DOCS(mymset.items, expected_docs);

	subqs.clear();
	expected_docs.clear();
	
	subqs.push_back(OmQuery(stemmer.stem_word("phrase")));
	subqs.push_back(OmQuery(stemmer.stem_word("near")));
	enquire.set_query(OmQuery(OM_MOP_NEAR, subqs.begin(), subqs.end(), 6));

	// retrieve the top ten results
	mymset = enquire.get_mset(0, 10);
	expected_docs.push_back(2);
	expected_docs.push_back(1);
	expected_docs.push_back(3);

	TEST_EXPECTED_DOCS(mymset.items, expected_docs);

	subqs.clear();
	expected_docs.clear();
	subqs.push_back(OmQuery(stemmer.stem_word("leave")));
	subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
	subqs.push_back(OmQuery(stemmer.stem_word("on")));
	enquire.set_query(OmQuery(OM_MOP_NEAR, subqs.begin(), subqs.end(), 3));

	// retrieve the top ten results
	mymset = enquire.get_mset(0, 10);
	expected_docs.push_back(6);
	expected_docs.push_back(4);
	expected_docs.push_back(5);
	expected_docs.push_back(7);
	expected_docs.push_back(9);
	expected_docs.push_back(8);
    
	TEST_EXPECTED_DOCS(mymset.items, expected_docs);

	subqs.clear();
	expected_docs.clear();
	
	subqs.push_back(OmQuery(stemmer.stem_word("leave")));
	subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
	subqs.push_back(OmQuery(stemmer.stem_word("on")));
	enquire.set_query(OmQuery(OM_MOP_NEAR, subqs.begin(), subqs.end(), 4));

	// retrieve the top ten results
	mymset = enquire.get_mset(0, 10);
	expected_docs.push_back(6);
	expected_docs.push_back(4);
	expected_docs.push_back(5);
	expected_docs.push_back(7);
	expected_docs.push_back(10);
	expected_docs.push_back(9);
	expected_docs.push_back(8);
	TEST_EXPECTED_DOCS(mymset.items, expected_docs);

	subqs.clear();
	expected_docs.clear();
	
	subqs.push_back(OmQuery(stemmer.stem_word("leave")));
	subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
	subqs.push_back(OmQuery(stemmer.stem_word("on")));
	enquire.set_query(OmQuery(OM_MOP_NEAR, subqs.begin(), subqs.end(), 5));

	// retrieve the top ten results
	mymset = enquire.get_mset(0, 10);
	expected_docs.push_back(6);
	expected_docs.push_back(4);
	expected_docs.push_back(5);
	expected_docs.push_back(7);
	expected_docs.push_back(10);
	expected_docs.push_back(11);
	expected_docs.push_back(9);
	expected_docs.push_back(8);
    
	TEST_EXPECTED_DOCS(mymset.items, expected_docs);

	subqs.clear();
	expected_docs.clear();
	
	subqs.push_back(OmQuery(stemmer.stem_word("leave")));
	subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
	subqs.push_back(OmQuery(stemmer.stem_word("on")));
	enquire.set_query(OmQuery(OM_MOP_NEAR, subqs.begin(), subqs.end(), 6));

	// retrieve the top ten results
	mymset = enquire.get_mset(0, 10);
	expected_docs.push_back(6);
	expected_docs.push_back(4);
	expected_docs.push_back(5);
	expected_docs.push_back(7);
	expected_docs.push_back(10);
	expected_docs.push_back(12);
	expected_docs.push_back(11);
	expected_docs.push_back(9);
	expected_docs.push_back(8);
    
	TEST_EXPECTED_DOCS(mymset.items, expected_docs);

	subqs.clear();
	expected_docs.clear();
	
	subqs.push_back(OmQuery(stemmer.stem_word("leave")));
	subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
	subqs.push_back(OmQuery(stemmer.stem_word("on")));
	enquire.set_query(OmQuery(OM_MOP_NEAR, subqs.begin(), subqs.end(), 7));

	// retrieve the top twenty results
	mymset = enquire.get_mset(0, 20);
	expected_docs.push_back(6);
	expected_docs.push_back(4);
	expected_docs.push_back(5);
	expected_docs.push_back(7);
	expected_docs.push_back(10);
	expected_docs.push_back(12);
	expected_docs.push_back(11);
	expected_docs.push_back(13);
	expected_docs.push_back(9);
	expected_docs.push_back(8);
    
	TEST_EXPECTED_DOCS(mymset.items, expected_docs);

	subqs.clear();
	expected_docs.clear();
	
	subqs.push_back(OmQuery(stemmer.stem_word("leave")));
	subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
	subqs.push_back(OmQuery(stemmer.stem_word("on")));
	enquire.set_query(OmQuery(OM_MOP_NEAR, subqs.begin(), subqs.end(), 8));

	// retrieve the top ten results
	mymset = enquire.get_mset(0, 20);
	expected_docs.push_back(6);
	expected_docs.push_back(4);
	expected_docs.push_back(5);
	expected_docs.push_back(7);
	expected_docs.push_back(10);
	expected_docs.push_back(12);
	expected_docs.push_back(11);
	expected_docs.push_back(13);
	expected_docs.push_back(9);
	expected_docs.push_back(8);
	expected_docs.push_back(14);
	
	subqs.clear();
	// same expected documents as previous query
	subqs.push_back(OmQuery(stemmer.stem_word("leave")));
	subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
	subqs.push_back(OmQuery(stemmer.stem_word("on")));
	// test really large window size
	enquire.set_query(OmQuery(OM_MOP_NEAR, subqs.begin(), subqs.end(),
				  999999999));

	// retrieve the top ten results
	mymset = enquire.get_mset(0, 20);
	
	TEST_EXPECTED_DOCS(mymset.items, expected_docs);
    }
    catch (OmUnimplementedError &err) {
	if (err.get_msg() !=
	    "InMemoryPostList::get_position_list() unimplemented" &&
	    err.get_msg() !=
	    "REMOTE: InMemoryPostList::get_position_list() unimplemented") {
	    cout << "!!!" << err.get_msg() << "!!!\n";
	    throw; // FIXME: ickity-ick
	}
    }

    return true;
}

/// Simple test of PHRASE
bool test_phrase1()
{
    OmDatabase mydb(get_database("apitest_phrase"));
    OmEnquire enquire(make_dbgrp(&mydb));
    OmStem stemmer("english");

    try {
	// make a query
	vector<OmQuery> subqs;
	vector<om_docid> expected_docs;
	subqs.push_back(OmQuery(stemmer.stem_word("phrase")));
	subqs.push_back(OmQuery(stemmer.stem_word("near")));
	enquire.set_query(OmQuery(OM_MOP_PHRASE, subqs.begin(), subqs.end(), 2));

	// retrieve the top ten results
	OmMSet mymset = enquire.get_mset(0, 10);
    
	TEST_EXPECTED_DOCS(mymset.items, expected_docs);

	subqs.clear();
	expected_docs.clear();
	
	subqs.push_back(OmQuery(stemmer.stem_word("phrase")));
	subqs.push_back(OmQuery(stemmer.stem_word("near")));
	enquire.set_query(OmQuery(OM_MOP_PHRASE, subqs.begin(), subqs.end(), 3));

	// retrieve the top ten results
	mymset = enquire.get_mset(0, 10);
	expected_docs.push_back(1);
    
	TEST_EXPECTED_DOCS(mymset.items, expected_docs);

	subqs.clear();
	expected_docs.clear();
	
	subqs.push_back(OmQuery(stemmer.stem_word("phrase")));
	subqs.push_back(OmQuery(stemmer.stem_word("near")));
	enquire.set_query(OmQuery(OM_MOP_PHRASE, subqs.begin(), subqs.end(), 5));

	// retrieve the top ten results
	mymset = enquire.get_mset(0, 10);
	expected_docs.push_back(1);
    
	TEST_EXPECTED_DOCS(mymset.items, expected_docs);

	subqs.clear();
	expected_docs.clear();
	
	subqs.push_back(OmQuery(stemmer.stem_word("phrase")));
	subqs.push_back(OmQuery(stemmer.stem_word("near")));
	enquire.set_query(OmQuery(OM_MOP_PHRASE, subqs.begin(), subqs.end(), 6));

	// retrieve the top ten results
	mymset = enquire.get_mset(0, 10);
	expected_docs.push_back(2);
	expected_docs.push_back(1);

	TEST_EXPECTED_DOCS(mymset.items, expected_docs);

	subqs.clear();
	expected_docs.clear();
	subqs.push_back(OmQuery(stemmer.stem_word("leave")));
	subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
	subqs.push_back(OmQuery(stemmer.stem_word("on")));
	enquire.set_query(OmQuery(OM_MOP_PHRASE, subqs.begin(), subqs.end(), 3));

	// retrieve the top ten results
	mymset = enquire.get_mset(0, 10);
	expected_docs.push_back(4);
    
	TEST_EXPECTED_DOCS(mymset.items, expected_docs);

	subqs.clear();
	expected_docs.clear();
	
	subqs.push_back(OmQuery(stemmer.stem_word("leave")));
	subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
	subqs.push_back(OmQuery(stemmer.stem_word("on")));
	enquire.set_query(OmQuery(OM_MOP_PHRASE, subqs.begin(), subqs.end(), 4));

	// retrieve the top ten results
	mymset = enquire.get_mset(0, 10);
	expected_docs.push_back(4);
	TEST_EXPECTED_DOCS(mymset.items, expected_docs);

	subqs.clear();
	expected_docs.clear();
	
	subqs.push_back(OmQuery(stemmer.stem_word("leave")));
	subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
	subqs.push_back(OmQuery(stemmer.stem_word("on")));
	enquire.set_query(OmQuery(OM_MOP_PHRASE, subqs.begin(), subqs.end(), 5));

	// retrieve the top ten results
	mymset = enquire.get_mset(0, 10);
	expected_docs.push_back(4);
    
	TEST_EXPECTED_DOCS(mymset.items, expected_docs);

	subqs.clear();
	expected_docs.clear();
	
	subqs.push_back(OmQuery(stemmer.stem_word("leave")));
	subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
	subqs.push_back(OmQuery(stemmer.stem_word("on")));
	enquire.set_query(OmQuery(OM_MOP_PHRASE, subqs.begin(), subqs.end(), 6));

	// retrieve the top ten results
	mymset = enquire.get_mset(0, 10);
	expected_docs.push_back(4);
    
	TEST_EXPECTED_DOCS(mymset.items, expected_docs);

	subqs.clear();
	expected_docs.clear();
	
	subqs.push_back(OmQuery(stemmer.stem_word("leave")));
	subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
	subqs.push_back(OmQuery(stemmer.stem_word("on")));
	enquire.set_query(OmQuery(OM_MOP_PHRASE, subqs.begin(), subqs.end(), 7));

	// retrieve the top twenty results
	mymset = enquire.get_mset(0, 20);
	expected_docs.push_back(4);
    
	TEST_EXPECTED_DOCS(mymset.items, expected_docs);

	subqs.clear();
	expected_docs.clear();
	
	subqs.push_back(OmQuery(stemmer.stem_word("leave")));
	subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
	subqs.push_back(OmQuery(stemmer.stem_word("on")));
	enquire.set_query(OmQuery(OM_MOP_PHRASE, subqs.begin(), subqs.end(), 8));

	// retrieve the top ten results
	mymset = enquire.get_mset(0, 20);
	expected_docs.push_back(4);
	
	subqs.clear();
	// same expected documents as previous query
	subqs.push_back(OmQuery(stemmer.stem_word("leave")));
	subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
	subqs.push_back(OmQuery(stemmer.stem_word("on")));
	// test really large window size
	enquire.set_query(OmQuery(OM_MOP_PHRASE, subqs.begin(), subqs.end(),
				  999999999));

	// retrieve the top ten results
	mymset = enquire.get_mset(0, 20);
	
	TEST_EXPECTED_DOCS(mymset.items, expected_docs);
    }
    catch (OmUnimplementedError &err) {
	if (err.get_msg() !=
	    "InMemoryPostList::get_position_list() unimplemented" &&
	    err.get_msg() !=
	    "REMOTE: InMemoryPostList::get_position_list() unimplemented") {
	    cout << "!!!" << err.get_msg() << "!!!\n";
	    throw; // FIXME: ickity-ick
	}
    }

    return true;
}




// #######################################################################
// # End of test cases: now we list the tests to run.

/// The tests which use a backend
test_desc db_tests[] = {
    {"zerodocid", 	   test_zerodocid},
    {"simplequery1",       test_simplequery1},
    {"simplequery2",       test_simplequery2},
    {"simplequery3",       test_simplequery3},
    {"multidb1",           test_multidb1},
    {"multidb2",           test_multidb2},
    {"changequery1",	   test_changequery1},
    {"msetmaxitems1",      test_msetmaxitems1},
    {"expandmaxitems1",    test_expandmaxitems1},
    {"boolquery1",         test_boolquery1},
    {"msetfirst1",         test_msetfirst1},
    {"topercent1",	   test_topercent1},
    {"expandfunctor1",	   test_expandfunctor1},
    {"matchfunctor1",	   test_matchfunctor1},
    {"pctcutoff1",	   test_pctcutoff1},
    {"allowqterms1",       test_allowqterms1},
    {"maxattain1",         test_maxattain1},
    {"collapsekey1",	   test_collapsekey1},
    {"reversebool1",	   test_reversebool1},
    {"reversebool2",	   test_reversebool2},
    {"getmterms1",	   test_getmterms1},
    {"absentfile1",	   test_absentfile1},
    {"poscollapse1",	   test_poscollapse1},
    {"batchquery1",	   test_batchquery1},
    {"repeatquery1",	   test_repeatquery1},
    {"absentterm1",	   test_absentterm1},
    {"absentterm2",	   test_absentterm2},
    {"multidb3",           test_multidb3},
    {"multidb4",           test_multidb4},
    {"rset1",              test_rset1},
    {"rset2",              test_rset2},
    {"rsetmultidb1",       test_rsetmultidb1},
    {"rsetmultidb2",       test_rsetmultidb2},
    {"maxorterms1",        test_maxorterms1},
    {"maxorterms2",        test_maxorterms2},
    {"termlisttermfreq",   test_termlisttermfreq},
    {"multiexpand1",       test_multiexpand1},
    {"near1",		   test_near1},
    {"phrase1",		   test_phrase1},
    {0, 0}
};

/// The tests which don't use any of the backends
test_desc nodb_tests[] = {
    {"trivial",            test_trivial},
    // {"alwaysfail",       test_alwaysfail},
    {"nullquery1",	   test_nullquery1},
    {"getqterms1",	   test_getqterms1},
    {"boolsubq1",	   test_boolsubq1},
    {"querylen1",	   test_querylen1},
    {"querylen2",	   test_querylen2},
    {"querylen3",	   test_querylen3},
    {"subqcollapse1",	   test_subqcollapse1},
    {"emptyquerypart1",    test_emptyquerypart1},
    //{"stemlangs",	   test_stemlangs},
    {0, 0}
};

int main(int argc, char *argv[])
{
    char *srcdir = getenv("srcdir");
    if (srcdir == NULL) {
        cout << "Error: $srcdir must be in the environment!" << endl;
	return(1);
    }

    int result;
    test_driver::result summary = {0, 0};
    test_driver::result sum_temp;
    
    backendmanager.set_datadir(std::string(srcdir) + "/testdata/");
    backendmanager.set_dbtype("void");
    cout << "Running tests with no backend..." << endl;
    result = test_driver::main(argc, argv, nodb_tests, &summary);

#if defined(MUS_BUILD_BACKEND_INMEMORY)
    backendmanager.set_dbtype("inmemory");
    cout << "Running tests with inmemory backend..." << endl;
    result = max(result, test_driver::main(argc, argv, db_tests, &sum_temp));
    summary.succeeded += sum_temp.succeeded;
    summary.failed += sum_temp.failed;
#endif

#if 1 && defined(MUS_BUILD_BACKEND_SLEEPY)
    backendmanager.set_dbtype("sleepycat");
    cout << "Running tests with sleepycat backend..." << endl;
    result = max(result, test_driver::main(argc, argv, db_tests, &sum_temp));
    summary.succeeded += sum_temp.succeeded;
    summary.failed += sum_temp.failed;
#endif

#if 0 && defined(MUS_BUILD_BACKEND_NET)
    backendmanager.set_dbtype("net");
    cout << "Running tests with net backend..." << endl;
    result = max(result, test_driver::main(argc, argv, db_tests, &sum_temp));
    summary.succeeded += sum_temp.succeeded;
    summary.failed += sum_temp.failed;
#endif

    cout << argv[0] << " total: " << summary.succeeded << " passed, "
	 << summary.failed << " failed." << endl;

    return result;
}
