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

#include <vector>
#include <pthread.h>

struct some_searches
{
    vector<OmQuery> queries;
    vector<OmMSet> expected_results;
    vector<OmMSet> inthread_results;
};

void
search_stuff(vector<OmQuery> & queries, vector<OmMSet> & results)
{

}

void *
search_thread(void * data)
{
    struct some_searches * searches =
	reinterpret_cast<struct some_searches *>(data);

    search_stuff(searches->queries, searches->inthread_results);
    return 0;
}

bool test_twothreads1()
{
    pthread_t thread1;
    pthread_t thread2;
    struct some_searches searches1;
    struct some_searches searches2;

    //TEST_NOT_EQUAL(searches1.queries.size(), 0);
    //TEST_NOT_EQUAL(searches2.queries.size(), 0);

    search_stuff(searches1.queries, searches1.expected_results);
    search_stuff(searches2.queries, searches2.expected_results);

    TEST_EQUAL(searches1.expected_results.size(),
               searches1.queries.size());
    TEST_EQUAL(searches2.expected_results.size(),
               searches2.queries.size());

    int err;
    err = pthread_create(&thread1, 0, search_thread, &searches1);
    TEST_EQUAL(err, 0);
    err = pthread_create(&thread2, 0, search_thread, &searches2);
    TEST_EQUAL(err, 0);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    TEST_EQUAL(searches1.expected_results.size(),
               searches1.inthread_results.size());
    TEST_EQUAL(searches2.expected_results.size(),
               searches2.inthread_results.size());

    for (vector<OmMSet>::size_type i = 0;
         i != searches1.expected_results.size(); i++) {
	TEST_EQUAL(searches1.expected_results[i],
		   searches1.inthread_results[i]);
    }

    for (vector<OmMSet>::size_type i = 0;
         i != searches2.expected_results.size(); i++) {
	TEST_EQUAL(searches2.expected_results[i],
		   searches2.inthread_results[i]);
    }

    return true;
}



// ##################################################################
// # End of actual tests                                            #
// ##################################################################

/// The lists of tests to perform
test_desc tests[] = {
    {"twothreads1",		test_twothreads1},
    {0, 0}
};

int main(int argc, char *argv[])
{
    return test_driver::main(argc, argv, tests);
}
