/* api_nodb.cc: tests which don't use any of the backends
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003 Olly Betts
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

#include <config.h>
#include <iostream>
#include <string>
#include <vector>
#include "autoptr.h"

#include "om/om.h"
#include "testsuite.h"
#include "testutils.h"

#include <list>

using namespace std;

typedef list<string> om_termname_list;

// always succeeds
static bool test_trivial1()
{
    return true;
}

// tests that get_query_terms() returns the terms in the right order
static bool test_getqterms1()
{
    om_termname_list answers_list;
    answers_list.push_back("one");
    answers_list.push_back("two");
    answers_list.push_back("three");
    answers_list.push_back("four");

    OmQuery myquery(OmQuery::OP_OR,
	    OmQuery(OmQuery::OP_AND,
		    OmQuery("one", 1, 1),
		    OmQuery("three", 1, 3)),
	    OmQuery(OmQuery::OP_OR,
		    OmQuery("four", 1, 4),
		    OmQuery("two", 1, 2)));

#ifdef __SUNPRO_CC
    om_termname_list list;
    {
        OmTermIterator t;
        for (t = myquery.get_terms_begin(); t != myquery.get_terms_end(); ++t) 
            list.push_back(*t);
    }
#else
    om_termname_list list(myquery.get_terms_begin(), myquery.get_terms_end());
#endif
    TEST(list == answers_list);
    return true;
}

// tests that empty queries work correctly
static bool test_emptyquery1()
{
    // test that an empty query is_empty
    TEST(OmQuery().is_empty());
    // test that an empty query has length 0
    TEST(OmQuery().get_length() == 0);
    vector<OmQuery> v;
    TEST(OmQuery(OmQuery::OP_OR, v.begin(), v.end()).is_empty());
    TEST(OmQuery(OmQuery::OP_OR, v.begin(), v.end()).get_length() == 0);
    return true;
}

// tests that query lengths are calculated correctly
static bool test_querylen1()
{
    // test that a simple query has the right length
    OmQuery myquery;
    myquery = OmQuery(OmQuery::OP_OR,
		      OmQuery("foo"),
		      OmQuery("bar"));
    myquery = OmQuery(OmQuery::OP_AND,
		      myquery,
		      OmQuery(OmQuery::OP_OR,
			      OmQuery("wibble"),
			      OmQuery("spoon")));

    TEST_EQUAL(myquery.get_length(), 4);
    return true;
}

// tests that query lengths are calculated correctly
static bool test_querylen2()
{
    // test with an even bigger and strange query
    string terms[3] = {
	"foo",
	"bar",
	"baz"
    };
    OmQuery queries[3] = {
	OmQuery("wibble"),
	OmQuery("wobble"),
	OmQuery(OmQuery::OP_OR, string("jelly"), string("belly"))
    };

    OmQuery myquery;
    vector<string> v1(terms, terms + 3);
    vector<OmQuery> v2(queries, queries + 3);
    vector<OmQuery *> v3;
    AutoPtr<OmQuery> dynquery1(new OmQuery(OmQuery::OP_AND,
					   string("ball"),
					   string("club")));
    AutoPtr<OmQuery> dynquery2(new OmQuery("ring"));
    v3.push_back(dynquery1.get());
    v3.push_back(dynquery2.get());

    OmQuery myq1 = OmQuery(OmQuery::OP_AND, v1.begin(), v1.end());
    tout << "myq1=" << myq1 << "\n";
    TEST_EQUAL(myq1.get_length(), 3);

    OmQuery myq2_1 = OmQuery(OmQuery::OP_OR, v2.begin(), v2.end());
    tout << "myq2_1=" << myq2_1 << "\n";
    TEST_EQUAL(myq2_1.get_length(), 4);

    OmQuery myq2_2 = OmQuery(OmQuery::OP_AND, v3.begin(), v3.end());
    tout << "myq2_2=" << myq2_2 << "\n";
    TEST_EQUAL(myq2_2.get_length(), 3);

    OmQuery myq2 = OmQuery(OmQuery::OP_OR, myq2_1, myq2_2);
    tout << "myq2=" << myq2 << "\n";
    TEST_EQUAL(myq2.get_length(), 7);

    myquery = OmQuery(OmQuery::OP_OR, myq1, myq2);
    tout << "myquery=" << myquery << "\n";
    TEST_EQUAL(myquery.get_length(), 10);

    return true;
}

// tests that queries validate correctly
static bool test_queryvalid1()
{
    vector<OmQuery> v1;
    // Need two arguments
    TEST_EXCEPTION(Xapian::InvalidArgumentError,
		   OmQuery(OmQuery::OP_AND_NOT, v1.begin(), v1.end()));
    tout << "ANDNOT () checked" << endl;
    v1.push_back(OmQuery("bad"));
    TEST_EXCEPTION(Xapian::InvalidArgumentError,
		   OmQuery(OmQuery::OP_AND_NOT, v1.begin(), v1.end()));
    tout << "ANDNOT (\"bad\") checked" << endl;
    v1.clear();
    v1.push_back(OmQuery());
    TEST_EXCEPTION(Xapian::InvalidArgumentError,
		   OmQuery(OmQuery::OP_AND_NOT, v1.begin(), v1.end()));
    tout << "ANDNOT (OmQuery()) checked" << endl;

    OmQuery q2(OmQuery::OP_XOR, OmQuery("foo"), OmQuery("bar"));
    tout << "XOR (\"foo\", \"bar\") checked" << endl;
    return true;
}

// tests that collapsing of queries includes subqueries
static bool test_subqcollapse1()
{
    OmQuery queries1[3] = {
	OmQuery("wibble"),
	OmQuery("wobble"),
	OmQuery(OmQuery::OP_OR, string("jelly"), string("belly"))
    };

    OmQuery queries2[3] = {
	OmQuery(OmQuery::OP_AND, string("jelly"), string("belly")),
	OmQuery("wibble"),
	OmQuery("wobble")
    };

    vector<OmQuery> vec1(queries1, queries1 + 3);
    OmQuery myquery1(OmQuery::OP_OR, vec1.begin(), vec1.end());
    TEST_EQUAL(myquery1.get_description(),
	       "OmQuery((wibble OR wobble OR jelly OR belly))");

    vector<OmQuery> vec2(queries2, queries2 + 3);
    OmQuery myquery2(OmQuery::OP_AND, vec2.begin(), vec2.end());
    TEST_EQUAL(myquery2.get_description(),
	       "OmQuery((jelly AND belly AND wibble AND wobble))");

    return true;
}

// test behaviour when creating a query from an empty vector
static bool test_emptyquerypart1()
{
    vector<string> emptyterms;
    OmQuery query(OmQuery::OP_OR, emptyterms.begin(), emptyterms.end());
    return true;
}

static bool test_singlesubq1()
{
    vector<string> oneterm;
    oneterm.push_back("solo");
    OmQuery q_eliteset(OmQuery::OP_ELITE_SET, oneterm.begin(), oneterm.end());
    q_eliteset.set_elite_set_size(1);
    OmQuery q_near(OmQuery::OP_NEAR, oneterm.begin(), oneterm.end());
    q_near.set_window(1);
    OmQuery q_phrase(OmQuery::OP_PHRASE, oneterm.begin(), oneterm.end());
    q_phrase.set_window(1);
    return true;
}

static bool test_stemlangs1()
{
    vector<string> langv;
    string langs = Xapian::Stem::get_available_languages();

    string::size_type next = langs.find_first_of(" ");

    while (langs.length() > 0) {
	langv.push_back(langs.substr(0, next));
    	if (next == langs.npos) {
	    langs = "";
	} else {
	    langs = langs.substr(next);
	}
	if (langs.length() > 0) {
	    // skip leading space too
	    langs = langs.substr(1);
	}
	next = langs.find_first_of(" ");
    }

    TEST(langv.size() != 0);

    vector<string>::const_iterator i;
    for (i = langv.begin(); i != langv.end(); i++) {
	// try making a stemmer with the given language -
	// it should successfully create, and not throw an exception.
	Xapian::Stem stemmer(*i);
    }

    return true;
}

// #######################################################################
// # End of test cases: now we list the tests to run.

test_desc nodb_tests[] = {
    {"trivial1",           test_trivial1},
    {"getqterms1",	   test_getqterms1},
    {"emptyquery1",	   test_emptyquery1},
    {"querylen1",	   test_querylen1},
    {"querylen2",	   test_querylen2},
    {"queryvalid1",	   test_queryvalid1},
    {"subqcollapse1",	   test_subqcollapse1},
    {"emptyquerypart1",    test_emptyquerypart1},
    {"singlesubq1",	   test_singlesubq1},
    {"stemlangs1",	   test_stemlangs1},
    {0, 0}
};
