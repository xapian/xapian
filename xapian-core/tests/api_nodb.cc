/* api_nodb.cc: tests which don't use any of the backends
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
#include <vector>

using std::cout;
using std::endl;
using std::vector;
using std::string;

#include "om/om.h"
#include "testsuite.h"
#include "testutils.h"
#include "textfile_indexer.h"
#include "../indexer/index_utils.h"
#include "backendmanager.h"
#include "utils.h"

// always succeeds
static bool test_trivial()
{
    return true;
}

// tests that get_query_terms() returns the terms in the right order
static bool test_getqterms1()
{
    bool success;

    static const char *answers[4] = {
	"one",
	"two",
	"three",
	"four"
    };

    OmQuery myquery(OmQuery::OP_OR,
	    OmQuery(OmQuery::OP_AND,
		    OmQuery("one", 1, 1),
		    OmQuery("three", 1, 3)),
	    OmQuery(OmQuery::OP_OR,
		    OmQuery("four", 1, 4),
		    OmQuery("two", 1, 2)));

    om_termname_list terms = myquery.get_terms();

    om_termname_list answers_list;
    for (unsigned int i=0; i<(sizeof(answers) / sizeof(answers[0])); ++i) {
        answers_list.push_back(answers[i]);
    }
    success = (terms == answers_list);
    if (verbose && !success) {
	cout << "Terms returned in incorrect order: ";
	copy(terms.begin(),
	     terms.end(),
	     ostream_iterator<om_termname>(cout, " "));
	cout << endl << "Expected: one two three four" << endl;
    }

    return success;
}

// tests that building a query with boolean sub-queries throws an exception.
static bool test_boolsubq1()
{
    OmQuery mybool("foo");
    mybool.set_bool(true);

    TEST_EXCEPTION(OmInvalidArgumentError,
		   OmQuery query(OmQuery::OP_OR, OmQuery("bar"), mybool));
    return true;
}

// tests that query lengths are calculated correctly
static bool test_querylen1()
{
    // test that a null query has length 0
    bool success = (OmQuery().get_length()) == 0;

    return success;
}

// tests that query lengths are calculated correctly
static bool test_querylen2()
{
    // test that a simple query has the right length
    bool success = true;

    OmQuery myquery;

    myquery = OmQuery(OmQuery::OP_OR,
		      OmQuery("foo"),
		      OmQuery("bar"));
    myquery = OmQuery(OmQuery::OP_AND,
		      myquery,
		      OmQuery(OmQuery::OP_OR,
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

// tests that query lengths are calculated correctly
static bool test_querylen3()
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
	OmQuery(OmQuery::OP_OR, std::string("jelly"), std::string("belly"))
    };

    OmQuery myquery;
    vector<om_termname> v1(terms, terms+3);
    vector<OmQuery> v2(queries, queries+3);
    vector<OmQuery *> v3;

    auto_ptr<OmQuery> dynquery1(new OmQuery(OmQuery::OP_AND,
					    std::string("ball"),
					    std::string("club")));
    auto_ptr<OmQuery> dynquery2(new OmQuery("ring"));
    v3.push_back(dynquery1.get());
    v3.push_back(dynquery2.get());

    OmQuery myq1 = OmQuery(OmQuery::OP_AND, v1.begin(), v1.end());
    if (myq1.get_length() != 3) {
	success = false;
	if (verbose) {
	    cout << "Query myq1 length is "
		    << myq1.get_length()
		    << ", expected 3.  Description: "
		    << myq1.get_description() << endl;
	}
    }

    OmQuery myq2_1 = OmQuery(OmQuery::OP_OR, v2.begin(), v2.end());
    if (myq2_1.get_length() != 4) {
	success = false;
	if (verbose) {
	    cout << "Query myq2_1 length is "
		    << myq2_1.get_length()
		    << ", expected 4.  Description: "
		    << myq2_1.get_description() << endl;
	}
    }

    OmQuery myq2_2 = OmQuery(OmQuery::OP_AND, v3.begin(), v3.end());
    if (myq2_2.get_length() != 3) {
	success = false;
	if (verbose) {
	    cout << "Query myq2_2 length is "
		    << myq2_2.get_length()
		    << ", expected 3.  Description: "
		    << myq2_2.get_description() << endl;
	}
    }

    OmQuery myq2 = OmQuery(OmQuery::OP_OR, myq2_1, myq2_2);
    if (myq2.get_length() != 7) {
	success = false;
	if (verbose) {
	    cout << "Query myq2 length is "
		    << myq2.get_length()
		    << ", expected 7.  Description: "
		    << myq2.get_description() << endl;
	}
    }

    myquery = OmQuery(OmQuery::OP_OR, myq1, myq2);
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

// tests that collapsing of queries includes subqueries
static bool test_subqcollapse1()
{
    bool success = true;

    OmQuery queries1[3] = {
	OmQuery("wibble"),
	OmQuery("wobble"),
	OmQuery(OmQuery::OP_OR, std::string("jelly"), std::string("belly"))
    };

    OmQuery queries2[3] = {
	OmQuery(OmQuery::OP_AND, std::string("jelly"), std::string("belly")),
	OmQuery("wibble"),
	OmQuery("wobble")
    };

    vector<OmQuery> vec1(queries1, queries1+3);
    OmQuery myquery1(OmQuery::OP_OR, vec1.begin(), vec1.end());
    string desc1 = myquery1.get_description();

    vector<OmQuery> vec2(queries2, queries2+3);
    OmQuery myquery2(OmQuery::OP_AND, vec2.begin(), vec2.end());
    string desc2 = myquery2.get_description();

    if(desc1 != "OmQuery((wibble OR wobble OR jelly OR belly))") {
	success = false;
	if(verbose) {
	    cout << "Failed to correctly collapse query: got `" <<
		    desc1 << "'" << endl;
	}
    }

    if(desc2 != "OmQuery((jelly AND belly AND wibble AND wobble))") {
	success = false;
	if(verbose) {
	    cout << "Failed to correctly collapse query: got `" <<
		    desc2 << "'" << endl;
	}
    }

    return success;
}

// test behaviour when creating a query from an empty vector
static bool test_emptyquerypart1()
{
    vector<om_termname> emptyterms;
    OmQuery query(OmQuery::OP_OR, emptyterms.begin(), emptyterms.end());

    return true;
}

static bool test_stemlangs()
{
    vector<string> langs;
    langs = OmStem::get_available_languages();

    TEST(langs.size() != 0);

    vector<string>::const_iterator i;
    for (i = langs.begin(); i != langs.end(); i++) {
	// try making a stemmer with the given language -
	// it should successfully create, and not throw an exception.
	OmStem stemmer(*i);
    }

    return true;
}

// Check that backend BACKEND doesn't exist
#define CHECK_BACKEND_UNKNOWN(BACKEND) do {\
    OmSettings p;\
    p.set("backend", (BACKEND));\
    TEST_EXCEPTION(OmInvalidArgumentError, OmDatabase db(p))\
    catch (const OmInvalidArgumentError &e) { }\
    } while (0)

// test that DatabaseBuilder throws correct error for a completely unknown
// database backend, or if an empty string is passed for the backend
static bool test_badbackend1()
{
    CHECK_BACKEND_UNKNOWN("shorterofbreathanotherdayclosertodeath");
    CHECK_BACKEND_UNKNOWN("");
    return true;
}

// Check that backend BACKEND isn't compiled in
#define CHECK_BACKEND_UNAVAILABLE(BACKEND) do {\
    OmSettings p;\
    p.set("backend", (BACKEND));\
    TEST_EXCEPTION(OmFeatureUnavailableError, OmDatabase db(p))\
    } while (0)

// test that DatabaseBuilder throws correct error for any unavailable
// database backends
static bool test_badbackend2()
{
#ifndef MUS_BUILD_BACKEND_INMEMORY
    CHECK_BACKEND_UNAVAILABLE("inmemory");
#endif
#ifndef MUS_BUILD_BACKEND_QUARTZ
    CHECK_BACKEND_UNAVAILABLE("quartz");
#endif
#ifndef MUS_BUILD_BACKEND_SLEEPYCAT
    CHECK_BACKEND_UNAVAILABLE("sleepycat");
#endif
#ifndef MUS_BUILD_BACKEND_REMOTE
    CHECK_BACKEND_UNAVAILABLE("remote");
#endif
#ifndef MUS_BUILD_BACKEND_MUSCAT36
    CHECK_BACKEND_UNAVAILABLE("da");
    CHECK_BACKEND_UNAVAILABLE("db");
#endif
    return true;
}

// #######################################################################
// # End of test cases: now we list the tests to run.

test_desc nodb_tests[] = {
    {"trivial",            test_trivial},
    {"getqterms1",	   test_getqterms1},
    {"boolsubq1",	   test_boolsubq1},
    {"querylen1",	   test_querylen1},
    {"querylen2",	   test_querylen2},
    {"querylen3",	   test_querylen3},
    {"subqcollapse1",	   test_subqcollapse1},
    {"emptyquerypart1",    test_emptyquerypart1},
    {"stemlangs",	   test_stemlangs},
    {"badbackend1",	   test_badbackend1},
    {"badbackend2",	   test_badbackend2},
    {0, 0}
};
