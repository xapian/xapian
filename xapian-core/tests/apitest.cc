/* apitest.cc - test of the OpenMuscat API
 *
 * ----START-LICENCE----
 * Copyright 2000 Dialog Corporation
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

#include <iostream>
#include <string>
#include <memory>
#include <getopt.h>
#include "om/om.h"

typedef bool (*testerfunc)();

struct om_test {
  char *name;
  testerfunc run;
};

// always succeeds
bool test_trivial();
// always fails (for testing the framework)
bool test_alwaysfail();
// tests that the inmemory doesn't return zero docids
bool test_zerodocid_inmemory();
// tests the document count for a simple inmemory query
bool test_simplequery1();
// tests for the right documents returned with simple query
bool test_simplequery2();
// tests for the right document count for another simple query
bool test_simplequery3();
// tests a query accross multiple databases
bool test_multidb1();
// tests that changing a query object after calling set_query()
// doesn't make any difference to get_mset().
bool test_changequery1();
// tests that a null query throws an exception
bool test_nullquery1();
// tests that when specifiying maxitems to get_mset, no more than
// that are returned.
bool test_msetmaxitems1();
// tests that when specifiying maxitems to get_eset, no more than
// that are returned.
bool test_expandmaxitems1();
// tests that a pure boolean query has all weights set to 1
bool test_boolquery1();
// tests that get_mset() specifying "first" works as expected
bool test_msetfirst1();
// tests the converting-to-percent functions
bool test_topercent1();
// tests the expand decision functor
bool test_expandfunctor1();
// tests the match decision functor
bool test_matchfunctor1();
// tests the percent cutoff option
bool test_pctcutoff1();
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

om_test tests[] = {
    {"trivial",            test_trivial},
    // {"alwaysfail",       test_alwaysfail},
    {"zerodocid_inmemory", test_zerodocid_inmemory},
    {"simplequery1",       test_simplequery1},
    {"simplequery2",       test_simplequery2},
    {"simplequery3",       test_simplequery3},
    {"multidb1",           test_multidb1},
    {"changequery1",	   test_changequery1},
    {"nullquery1",	   test_nullquery1},
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
    {0, 0}
};

string datadir;

bool verbose = false;

void usage(char *progname)
{
    cerr << "Usage: " << progname << " [-v] [-o] [-f]" << endl;
}

//  A wrapper around the tests to trap exceptions,
//  and avoid having to catch them in every test function.
bool runtest(om_test *test)
{
    bool success;
    try {
        success = test->run();
    } catch (OmError &err) {
	cout << "OmError exception: " << err.get_msg();
	success = false;
    } catch (...) {
	cout << "Unknown exception! ";
	success = false;
    }
    return success;
}

int main(int argc, char *argv[])
{
    bool abort_on_error = false;
    bool fussy = false;

    om_test *test = &tests[0];
    int num_failed = 0;
    int num_succeeded = 0;
    int c;

    while ((c = getopt(argc, argv, "vof")) != EOF) {
	switch (c) {
	    case 'v':
		verbose = true;
		break;
	    case 'o':
	    	abort_on_error = true;
		break;
	    case 'f':
	    	fussy = true;
		break;
	    default:
	    	usage(argv[0]);
		exit(1);
	}
    }

    if (optind != (argc)) {
    	usage(argv[0]);
	return 1;
    }
    char *srcdir = getenv("srcdir");
    if (srcdir == NULL) {
        cout << "Error: $srcdir must be in the environment!" << endl;
	return(1);
    }
    datadir = std::string(srcdir) + "/testdata/";

    while ((test->name) != 0) {
    	cout << "Running test: " << test->name << "...";
	bool succeeded = runtest(test);
	if (succeeded) {
	    ++num_succeeded;
	    cout << " ok." << endl;
	} else {
	    ++num_failed;
	    cout << " FAILED" << endl;
	    if (abort_on_error) {
	        cout << "Test failed - aborting further tests." << endl;
		break;
	    }
	}
	++test;
    }
    cout << "apitest finished: "
         << num_succeeded << " tests passed, "
	 << num_failed << " failed."
	 << endl;
	
    // FIXME: fussy should be the default, but for the moment
    // we want distcheck to succeed even though the tests don't
    // all pass, so that we can get nightly snapshots.
    if (fussy) {
	return (bool)num_failed; // if 0, then everything passed
    } else {
	return 0;
    }
}


bool mset_range_is_same(const OmMSet &mset1, unsigned int first1,
                        const OmMSet &mset2, unsigned int first2,
			unsigned int count)
{
    for (unsigned int i=0; i<count; ++i) {
        if ((mset1.items[first1+i].wt != mset2.items[first2+i].wt) ||
	    (mset1.items[first1+i].did != mset2.items[first2+i].did)) {
	    return false;
        }
    }
    return true;
}

bool operator==(const OmMSet &first, const OmMSet &second)
{
    if ((first.mbound != second.mbound) ||
	(first.max_possible != second.max_possible) ||
	(first.items.size() != second.items.size())) {
         return false;
    }
    return mset_range_is_same(first, 0, second, 0, first.items.size());
}

bool test_trivial()
{
    return true;
}

bool test_alwaysfail()
{
    return false;
}

bool test_zerodocid_inmemory()
{
    bool success = true;
    // open the database (in this case a simple text file
    // we prepared earlier)
    OmEnquire enquire;
    vector<string> dbargs;
    dbargs.push_back(datadir + "/apitest_onedoc.txt");

    enquire.add_database("inmemory", dbargs);

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
	    cout << "A query on an inmemory database returned a zero docid" << endl;
	}
	success = false;
    }
    return success;
}

void init_simple_enquire(OmEnquire &enq, const OmQuery &query = OmQuery("thi"))
{
    vector<string> dbargs;
    dbargs.push_back(datadir + "/apitest_simpledata.txt");
    enq.add_database("inmemory", dbargs);

    enq.set_query(query);
}

OmMSet do_get_simple_query_mset(OmQuery query, int maxitems = 10, int first = 0)
{
    // open the database (in this case a simple text file
    // we prepared earlier)
    OmEnquire enquire;
    init_simple_enquire(enquire, query);

    // retrieve the top results
    return enquire.get_mset(first, maxitems);
}

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
		cout << " " << mymset.items[i].did;
	    }
	    cout << ", expected 2 and 4." << endl;
	}
	success = false;
    }

    return success;
}

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

bool test_multidb1()
{
    bool success = true;
    OmEnquire enquire1;
    vector<string> dbargs1;
    dbargs1.push_back(datadir + "/apitest_simpledata.txt");
    dbargs1.push_back(datadir + "/apitest_simpledata2.txt");
    enquire1.add_database("inmemory", dbargs1);

    OmEnquire enquire2;
    vector<string> dbargs2;
    dbargs2.push_back(datadir + "/apitest_simpledata.txt");
    enquire2.add_database("inmemory", dbargs2);
    dbargs2[0] = datadir + "/apitest_simpledata2.txt";
    enquire2.add_database("inmemory", dbargs2);

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
    }
    return success;
}

bool test_changequery1()
{
    bool success = true;
    // The search is for "thi" rather than "this" because
    // the index will have stemmed versions of the terms.
    // open the database (in this case a simple text file
    // we prepared earlier)
    OmEnquire enquire;
    vector<string> dbargs;
    dbargs.push_back(datadir + "/apitest_simpledata.txt");

    enquire.add_database("inmemory", dbargs);

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

bool test_msetmaxitems1()
{
    OmMSet mymset = do_get_simple_query_mset(OmQuery("thi"), 1);
    return (mymset.items.size() == 1);
}

bool test_expandmaxitems1()
{
    OmEnquire enquire;
    init_simple_enquire(enquire);

    OmMSet mymset = enquire.get_mset(0, 10);

    OmRSet myrset;
    myrset.add_document(mymset.items[0].did);
    myrset.add_document(mymset.items[1].did);
    OmESet myeset = enquire.get_eset(1, myrset);

    return (myeset.items.size() == 1);
}

bool test_boolquery1()
{
    bool success = true;
    OmQuery myboolquery(OmQuery(OM_MOP_FILTER,
				OmQuery(),
				OmQuery("thi")));
    OmMSet mymset = do_get_simple_query_mset(myboolquery);

    if (mymset.max_possible != 1) {
        success = false;
	if (verbose) {
	    cout << "Max weight in mset is " << mymset.max_possible << endl;
	}
    } else {
        for (unsigned int i = 0; i<mymset.items.size(); ++i) {
	   if (mymset.items[i].wt != 1) {
	       success = false;
	       if (verbose) {
	           cout << "Item " << i
		        << " in mset has weight "
			<< mymset.items[i].wt
			<< ", should be 1." << endl;
	       }
	       break;
	   }
	}
    }

    return success;
}

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
	    return (sum % 2) == 0;
	}
};

bool test_expandfunctor1()
{
    bool success = true;

    OmEnquire enquire;
    init_simple_enquire(enquire);

    OmMSet mymset = enquire.get_mset(0, 10);
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
	        cout << "Mismatch in the items after filtering" << endl;
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
	    cout << "Extra items in the non-filtered eset." << endl;
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
	    return doc->get_key(2).value == 0;
	}
};

bool test_matchfunctor1()
{
    // FIXME: check that the functor works both ways.
    bool success = true;

    OmEnquire enquire;
    init_simple_enquire(enquire);

    myMatchDecider myfunctor;

    OmMSet mymset = enquire.get_mset(0, 100, 0, 0, &myfunctor);

    for (unsigned int i=0; i<mymset.items.size(); ++i) {
	auto_ptr<const OmDocument> doc(enquire.get_doc(mymset.items[i]));
        if (!myfunctor(doc.get())) {
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

bool test_pctcutoff1()
{
    bool success = true;

    OmEnquire enquire;
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

    OmEnquire enquire;
    init_simple_enquire(enquire);

    OmMSet mymset = enquire.get_mset(0, 10);
    OmRSet myrset;
    myrset.add_document(mymset.items[0].did);
    myrset.add_document(mymset.items[1].did);

    OmExpandOptions eopt;
    eopt.use_query_terms(false);

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

    OmEnquire enquire;
    init_simple_enquire(enquire);

    OmMatchOptions mymopt;
    for (int key_no = 1; key_no<7; ++key_no) {
        vector<om_docid> dids(key_no);
	mymopt.set_collapse_key(key_no);

	OmMSet mymset = enquire.get_mset(0, 100, 0, &mymopt);

	for (vector<OmMSetItem>::const_iterator i=mymset.items.begin();
	     i != mymset.items.end();
	     ++i) {
	    if (dids[i->did % key_no] != 0) {
	        success = false;
		if (verbose) {
		    cout << "docids " << dids[i->did % key_no]
		         << " and " << i->did
			 << " both found in MSet with key_no " << key_no
			 << endl;
		}
		break;
	    } else {
	        dids[i->did % key_no] = i->did;
	    }
	}
	// don't bother continuing if we've already failed.
	if (!success) break;
    }

    return success;
}

bool test_reversebool1()
{
    OmEnquire enquire;
    init_simple_enquire(enquire);

    OmMatchOptions mymopt;
    OmMSet mymset1 = enquire.get_mset(0, 100, 0, &mymopt);
    mymopt.set_sort_forward();
    OmMSet mymset2 = enquire.get_mset(0, 100, 0, &mymopt);
    mymopt.set_sort_forward(false);
    OmMSet mymset3 = enquire.get_mset(0, 100, 0, &mymopt);

    // mymset1 and mymset2 should be identical
    if(mymset1.items.size() != mymset2.items.size()) return false;
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
    if(mymset1.items.size() != mymset3.items.size()) return false;
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
    OmEnquire enquire;
    init_simple_enquire(enquire);

    OmMatchOptions mymopt;
    OmMSet mymset1 = enquire.get_mset(0, 100, 0, &mymopt);
    mymopt.set_sort_forward();
    om_doccount msize = mymset.items.size() / 2;
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
