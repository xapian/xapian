/** @file
 * @brief QueryParser tests which need a backend
 */
/* Copyright (c) 2009,2015 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>

#include "api_qpbackend.h"

#include <xapian.h>

#include "backendmanager.h"
#include "testsuite.h"
#include "testutils.h"

#include "apitest.h"

using namespace std;

namespace {
struct test {
    const char *query;
    const char *expect;
};
}

/// Regression test for bug#407 fixed in 1.0.17 and 1.1.3.
DEFINE_TESTCASE(qpsynonympartial1, synonyms) {
    static const test test_queries[] = {
	{ "hello", "(WILDCARD SYNONYM hello OR hello@1)" },
	{ "~hello", "(hello@1 SYNONYM hi@1 SYNONYM howdy@1)" },
	{ "hello world", "(hello@1 OR (WILDCARD SYNONYM world OR world@2))" },
	{ "~hello world", "((hello@1 SYNONYM hi@1 SYNONYM howdy@1) OR (WILDCARD SYNONYM world OR world@2))" },
	{ "world ~hello", "(world@1 OR (hello@2 SYNONYM hi@2 SYNONYM howdy@2))" },
	{ NULL, NULL }
    };
    static const test test_queries_auto[] = {
	{ "hello", "(hello@1 SYNONYM hi@1 SYNONYM howdy@1)" },
	{ "~hello", "(hello@1 SYNONYM hi@1 SYNONYM howdy@1)" },
	{ "hello world", "((hello@1 SYNONYM hi@1 SYNONYM howdy@1) OR world@2)" },
	{ "~hello world", "((hello@1 SYNONYM hi@1 SYNONYM howdy@1) OR world@2)" },
	{ "world ~hello", "(world@1 OR (hello@2 SYNONYM hi@2 SYNONYM howdy@2))" },
	{ NULL, NULL }
    };
    static const test test_queries_partial_auto[] = {
	{ "hello", "(WILDCARD SYNONYM hello OR hello@1)" },
	{ "~hello", "(WILDCARD SYNONYM hello OR hello@1)" },
	{ "hello world", "((hello@1 SYNONYM hi@1 SYNONYM howdy@1) OR (WILDCARD SYNONYM world OR world@2))" },
	{ "~hello world", "((hello@1 SYNONYM hi@1 SYNONYM howdy@1) OR (WILDCARD SYNONYM world OR world@2))" },
	{ "world ~hello", "(world@1 OR (WILDCARD SYNONYM hello OR hello@2))" },
	{ NULL, NULL }
    };

    Xapian::Database db = get_database("qpsynonympartial1",
				       [](Xapian::WritableDatabase& wdb,
					  const string&) {
					   wdb.add_synonym("hello", "hi");
					   wdb.add_synonym("hello", "howdy");
				       });
    Xapian::Enquire enquire(db);
    Xapian::QueryParser qp;
    Xapian::Stem stemmmer("english");
    qp.set_database(db);
    qp.add_prefix("foo", "XFOO");

    const test *p;
    for (p = test_queries; p->query; ++p) {
	string expect, parsed;
	if (p->expect)
	    expect = p->expect;
	else
	    expect = "parse error";
	try {
	    unsigned f = qp.FLAG_SYNONYM | qp.FLAG_PARTIAL | qp.FLAG_DEFAULT;
	    Xapian::Query qobj = qp.parse_query(p->query, f);
	    parsed = qobj.get_description();
	    expect = string("Query(") + expect + ')';
	} catch (const Xapian::QueryParserError &e) {
	    parsed = e.get_msg();
	} catch (const Xapian::Error &e) {
	    parsed = e.get_description();
	} catch (...) {
	    parsed = "Unknown exception!";
	}
	tout << "Query: " << p->query << '\n';
	TEST_STRINGS_EQUAL(parsed, expect);
    }

    for (p = test_queries_auto; p->query; ++p) {
	string expect, parsed;
	if (p->expect)
	    expect = p->expect;
	else
	    expect = "parse error";
	try {
	    unsigned f = qp.FLAG_AUTO_SYNONYMS | qp.FLAG_DEFAULT;
	    Xapian::Query qobj = qp.parse_query(p->query, f);
	    parsed = qobj.get_description();
	    expect = string("Query(") + expect + ')';
	} catch (const Xapian::QueryParserError &e) {
	    parsed = e.get_msg();
	} catch (const Xapian::Error &e) {
	    parsed = e.get_description();
	} catch (...) {
	    parsed = "Unknown exception!";
	}
	tout << "Query: " << p->query << '\n';
	TEST_STRINGS_EQUAL(parsed, expect);
    }

    for (p = test_queries_partial_auto; p->query; ++p) {
	string expect, parsed;
	if (p->expect)
	    expect = p->expect;
	else
	    expect = "parse error";
	try {
	    unsigned f = qp.FLAG_AUTO_SYNONYMS | qp.FLAG_PARTIAL;
	    f |= qp.FLAG_DEFAULT;
	    Xapian::Query qobj = qp.parse_query(p->query, f);
	    parsed = qobj.get_description();
	    expect = string("Query(") + expect + ')';
	} catch (const Xapian::QueryParserError &e) {
	    parsed = e.get_msg();
	} catch (const Xapian::Error &e) {
	    parsed = e.get_description();
	} catch (...) {
	    parsed = "Unknown exception!";
	}
	tout << "Query: " << p->query << '\n';
	TEST_STRINGS_EQUAL(parsed, expect);
    }
}
