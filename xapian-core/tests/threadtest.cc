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
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

using std::cout;
using std::cerr;
using std::endl;

#include "omlocks.h"
#include <memory>
std::auto_ptr<OmLock> outputmutex;

void
OutputMessage(std::string msg) {
    OmLockSentry sentry(*outputmutex);
    fprintf(stdout, "%s\n", msg.c_str());
}

static std::string database_path;
static std::string queryfile;
static int num_threads;

struct some_searches
{
    std::string database_type;
    std::string database_path;
    OmDatabaseGroup dbgrp;
    std::vector<OmQuery> queries;
    std::vector<OmMSet> expected_results;
    std::vector<OmMSet> inthread_results;
};

OmDatabaseGroup
open_db_group(std::string database_type, std::string dlist_path)
{
    OmDatabaseGroup dbgrp;

    FILE * fp = fopen (dlist_path.c_str(), "r");
    TEST_AND_EXPLAIN(fp != 0, "Can't open file `" << dlist_path << "' - " <<
		     strerror(errno));
    while(!feof(fp) && !ferror(fp)) {
	std::string database_path;
	while(1) {
	    char c;
	    fread(&c, sizeof(char), 1, fp);
	    if (feof (fp) || ferror (fp) || c == '\n') {
		break;
	    }
	    database_path += std::string(&c, 1);
	}
	if(database_path.size() != 0) {
	    OutputMessage("Adding `" + database_path + "' to dlist");
	    OmSettings params;
	    params.set_value("backend", database_type);
	    params.set_value("auto_dir", database_path); // FIXME:
	    OmDatabase db(params);
	    dbgrp.add_database(db);
	}
    }

    TEST_AND_EXPLAIN(!ferror(fp), "Error reading dlist file `" <<
		     dlist_path << "' - " << strerror(errno));

    return dbgrp;
}

void
search_stuff(OmEnquire & enq,
	     std::vector<OmQuery> & queries,
	     std::vector<OmMSet> & results)
{
    std::vector<OmQuery>::const_iterator i;
    for (i = queries.begin(); i != queries.end(); i++) {
	enq.set_query(*i);
	results.push_back(enq.get_mset(0, 10));
	if(results[results.size() - 1].items.size() > 0) {
	    enq.get_matching_terms(results[results.size() - 1].items[0]);
	}
    }
}

void
search_stuff(OmDatabaseGroup & dbgrp,
	     std::vector<OmQuery> & queries,
	     std::vector<OmMSet> & results)
{
    OmEnquire enq(dbgrp);
    search_stuff(enq, queries, results);
}

void
search_stuff(std::string database_type,
	     std::string database_path,
	     std::vector<OmQuery> & queries,
	     std::vector<OmMSet> & results)
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
read_queries(std::string filename, std::vector<OmQuery> & queries)
{
    FILE * fp = fopen (filename.c_str(), "r");
    TEST_AND_EXPLAIN(fp != 0, "Can't open file `" << filename << "' - " <<
		     strerror(errno))
    while(!feof(fp)) {
	std::vector<std::string> terms;
	std::string thisterm;
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
		thisterm += std::string(&c, 1);
	    }
	}

	if (terms.size() != 0) {
	    OmQuery new_query(OM_MOP_OR, terms.begin(), terms.end());
	    OutputMessage(new_query.get_description());
	    queries.push_back(new_query);
	}
    }
    fclose (fp);
}

bool check_query_threads(void * (* search_thread)(void *))
{
    std::vector<pthread_t> threads;
    std::vector<struct some_searches> searches;

    OutputMessage("Performing test with " + om_tostring(num_threads) +
		  " threads.");

    struct some_searches mainsearch;

    mainsearch.database_type = "da_flimsy"; // FIXME: "da" and set "m36_heavyduty" to false
    mainsearch.database_path = database_path;

    mainsearch.dbgrp = open_db_group(mainsearch.database_type,
				     mainsearch.database_path);

    for (int i = 0; i < num_threads; i++) {
	struct some_searches newsearch;
	newsearch.database_type = mainsearch.database_type;
	newsearch.database_path = mainsearch.database_path;
	newsearch.dbgrp = mainsearch.dbgrp;

	read_queries(queryfile + om_tostring((i % 2) + 1),
		     newsearch.queries);
	OutputMessage("search " + om_tostring(i + 1) + " has " +
		om_tostring(newsearch.queries.size()) + " items");
	TEST_NOT_EQUAL(newsearch.queries.size(), 0);
	searches.push_back(newsearch);
    }

    for (int i = 0; i < num_threads; i++) {
	OutputMessage("Performing single threaded search for search " +
		om_tostring(i + 1));
	search_stuff(searches[i].database_type,
		     searches[i].database_path,
		     searches[i].queries,
		     searches[i].expected_results);
	TEST_EQUAL(searches[i].expected_results.size(),
		   searches[i].queries.size());
	OutputMessage("done.");
    }

    for (int i = 0; i < num_threads; i++) {
	pthread_t newthread;
	threads.push_back(newthread);
    }

    for (int i = 0; i < num_threads; i++) {
	int err;
	OutputMessage("starting thread search " + om_tostring(i + 1));
	err = pthread_create(&threads[i],
			     0,
			     search_thread,
			     &searches[i]);
	TEST_EQUAL(err, 0);
    }

    for (int i = 0; i < num_threads; i++) {
	OutputMessage("waiting for end of thread search " + om_tostring(i + 1));
	pthread_join(threads[i], NULL);
    }
    OutputMessage("all threads finished");

    for (int i = 0; i < num_threads; i++) {
	TEST_EQUAL(searches[i].expected_results.size(),
		   searches[i].inthread_results.size());

	for (std::vector<OmMSet>::size_type j = 0;
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

void
mysleep(int secs)
{
    struct timeval timeout;
    timeout.tv_sec = secs;
    timeout.tv_usec = 0;

    int err = select(0, 0, 0, 0, &timeout);
    TEST_EQUAL(err, 0);
}
    
void *
sleep_thread(void * data)
{
    OmLock * lock = (OmLock *)data;

    if(lock) lock->lock();
    OutputMessage("Sleeping");
    mysleep(2);
    OutputMessage("Slept");
    if(lock) lock->unlock();

    return 0;
}

long time_sleep(OmLock * lock_ptr)
{
    pthread_t thread1;
    pthread_t thread2;

    struct timeval tv1;
    struct timeval tv2;

    int err;

    err = gettimeofday(&tv1, 0);
    TEST_EQUAL(err, 0);

    OutputMessage("Creating thread 1");
    err = pthread_create(&thread1,
			 0,
			 sleep_thread,
			 lock_ptr);
    TEST_EQUAL(err, 0);

    OutputMessage("Creating thread 2");
    err = pthread_create(&thread2,
			 0,
			 sleep_thread,
			 lock_ptr);
    TEST_EQUAL(err, 0);

    OutputMessage("waiting for end of thread 1 ");
    pthread_join(thread1, NULL);
    OutputMessage("waiting for end of thread 2 ");
    pthread_join(thread2, NULL);

    OutputMessage("all threads finished");

    err = gettimeofday(&tv2, 0);
    TEST_EQUAL(err, 0);

    long time = (tv2.tv_sec - tv1.tv_sec) * 1000;
    time += (tv2.tv_usec - tv1.tv_usec) / 1000;

    OutputMessage("Time taken : " + om_tostring(time) + "ms");

    return time;
}

/// Check that mutexes work.
bool test_mutextest()
{
    OmLock lock;

    TEST(time_sleep(&lock) > 3500);
    TEST(time_sleep(0) < 3500);

    return true;
}



// ##################################################################
// # End of actual tests                                            #
// ##################################################################

/// The lists of tests to perform
test_desc tests[] = {
    {"mutextest",               test_mutextest},
    {"separatedbs",		test_separatedbs},
    {"samedb",			test_samedb},
    {0, 0}
};

int main(int argc, char *argv[])
{
    {
	std::auto_ptr<OmLock> temp(new OmLock());
	outputmutex = temp;
    }
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
