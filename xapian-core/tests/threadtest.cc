/* threadtest.cc - test of the OpenMuscat in the multithreaded case
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
#include <string>

#include "om/om.h"
#include "testsuite.h"
#include "testutils.h"
#include "utils.h"

#include <vector>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

static string database_path;
static string queryfile;
static int num_threads;

struct some_searches
{
    string database_type;
    string database_path;
    OmDatabaseGroup dbgrp;
    vector<OmQuery> queries;
    vector<OmMSet> expected_results;
    vector<OmMSet> inthread_results;
};

OmDatabaseGroup
open_db_group(string database_type, string dlist_path)
{
    OmDatabaseGroup dbgrp;

    FILE * fp = fopen (dlist_path.c_str(), "r");
    TEST_AND_EXPLAIN(fp != 0, "Can't open file `" << dlist_path << "' - " <<
		     strerror(errno));
    while(!feof(fp) && !ferror(fp)) {
	string database_path;
	while(1) {
	    char c;
	    fread(&c, sizeof(char), 1, fp);
	    if (feof (fp) || ferror (fp) || c == '\n') {
		break;
	    }
	    database_path += string(&c, 1);
	}
	if(database_path.size() != 0) {
	    cout << "Adding `" << database_path << "' to dlist" << endl;
	    vector<string> params;
	    params.push_back(database_path);
	    OmDatabase db(database_type, params);
	    dbgrp.add_database(db);
	}
    }

    TEST_AND_EXPLAIN(!ferror(fp), "Error reading dlist file `" <<
		     dlist_path << "' - " << strerror(errno));

    return dbgrp;
}

void
search_stuff(OmEnquire & enq,
	     vector<OmQuery> & queries,
	     vector<OmMSet> & results)
{
    vector<OmQuery>::const_iterator i;
    for (i = queries.begin(); i != queries.end(); i++) {
	enq.set_query(*i);
	results.push_back(enq.get_mset(0, 10));
    }
}

void
search_stuff(OmDatabaseGroup & dbgrp,
	     vector<OmQuery> & queries,
	     vector<OmMSet> & results)
{
    OmEnquire enq(dbgrp);
    search_stuff(enq, queries, results);
}

void
search_stuff(string database_type,
	     string database_path,
	     vector<OmQuery> & queries,
	     vector<OmMSet> & results)
{
    OmDatabaseGroup dbgrp = open_db_group(database_type, database_path);
    search_stuff(dbgrp, queries, results);
}

void *
search_thread_separatedbs(void * data)
{
    struct some_searches * searches =
	reinterpret_cast<struct some_searches *>(data);

    search_stuff(searches->database_type,
		 searches->database_path,
		 searches->queries,
		 searches->inthread_results);
    return 0;
}

void *
search_thread_separateenqs(void * data)
{
    struct some_searches * searches =
	reinterpret_cast<struct some_searches *>(data);

    search_stuff(searches->dbgrp,
		 searches->queries,
		 searches->inthread_results);
    return 0;
}

void
read_queries(string filename, vector<OmQuery> & queries)
{
    FILE * fp = fopen (filename.c_str(), "r");
    TEST_AND_EXPLAIN(fp != 0, "Can't open file `" << filename << "' - " <<
		     strerror(errno))
    while(!feof(fp)) {
	vector<string> terms;
	string thisterm;
	while(1) {
	    char c;
	    fread(&c, sizeof(char), 1, fp);
	    if (feof (fp) || c == '\n') {
		break;
	    }
	    if (c == ';') {
		if(thisterm.size() != 0) {
		    terms.push_back(thisterm);
		    thisterm = "";
		}
	    } else {
		thisterm += string(&c, 1);
	    }
	}

	if (terms.size() != 0) {
	    OmQuery new_query(OM_MOP_OR, terms.begin(), terms.end());
	    //cout << new_query.get_description() << endl;
	    queries.push_back(new_query);
	}
    }
    fclose (fp);
}

bool check_query_threads(void * (* search_thread)(void *))
{
    vector<pthread_t> threads;
    vector<struct some_searches> searches;

    cout << "Performing test with " << num_threads << " threads." << endl;

    struct some_searches mainsearch;

    mainsearch.database_type = "da_flimsy";
    mainsearch.database_path = database_path;

    mainsearch.dbgrp = open_db_group(mainsearch.database_type,
				     mainsearch.database_path);

    for (int i = 0; i < num_threads; i++) {
	struct some_searches newsearch;
	newsearch.database_type = mainsearch.database_type;
	newsearch.database_path = mainsearch.database_path;
	newsearch.dbgrp = mainsearch.dbgrp;

	read_queries(queryfile + om_inttostring((i % 2) + 1),
		     newsearch.queries);
	cout << "search " << (i + 1) << " has " <<
		newsearch.queries.size() << " items" << endl;
	TEST_NOT_EQUAL(newsearch.queries.size(), 0);
	searches.push_back(newsearch);
    }

    for (int i = 0; i < num_threads; i++) {
	cout << "Performing single threaded search for search " <<
		(i + 1) << endl;
	search_stuff(searches[i].database_type,
		     searches[i].database_path,
		     searches[i].queries,
		     searches[i].expected_results);
	TEST_EQUAL(searches[i].expected_results.size(),
		   searches[i].queries.size());
	cout << "done." << endl;
    }

    for (int i = 0; i < num_threads; i++) {
	int err;
	pthread_t newthread;
	threads.push_back(newthread);
	cout << "starting thread search " << (i + 1) << endl;
	err = pthread_create(&threads[threads.size() - 1],
			     0,
			     search_thread,
			     &searches[threads.size() - 1]);
	TEST_EQUAL(err, 0);
    }

    for (int i = 0; i < num_threads; i++) {
	cout << "waiting for end of thread search " << (i + 1) << endl;
	pthread_join(threads[i], NULL);
    }
    cout << "all threads finished" << endl;

    for (int i = 0; i < num_threads; i++) {
	TEST_EQUAL(searches[i].expected_results.size(),
		   searches[i].inthread_results.size());

	for (vector<OmMSet>::size_type j = 0;
	     j != searches[i].expected_results.size(); j++) {
	    TEST_EQUAL(searches[i].expected_results[j],
		       searches[i].inthread_results[j]);
	}
    }

    return true;
}

/** Test behaviour when multiple threads sharing a db, but each has its own
 *  enquire.
 */
bool test_samedb()
{
    return check_query_threads(search_thread_separateenqs);
}

/// Test behaviour when multiple threads but each has its own db and enquire.
bool test_separatedbs()
{
    return check_query_threads(search_thread_separatedbs);
}



// ##################################################################
// # End of actual tests                                            #
// ##################################################################

/// The lists of tests to perform
test_desc tests[] = {
    {"separatedbs",		test_separatedbs},
    {"samedb",			test_samedb},
    {0, 0}
};

int main(int argc, char *argv[])
{
    if (argc < 4) {
	cerr << "Usage: " << argv[0] <<
		" <dlist path> <queryfile> <threadcount> <options>" << endl;
	exit (77);
    }
    database_path = argv[1];
    queryfile = argv[2];
    num_threads = atoi(argv[3]);
    argc -= 3;
    argv += 3;
    return test_driver::main(argc, argv, tests);
}
