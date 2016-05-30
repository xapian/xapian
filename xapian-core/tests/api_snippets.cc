/* api_snippets.cc: tests snippets
 *
 * Copyright 2012 Mihai Bivol
 * Copyright 2015,2016 Olly Betts
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

#include "api_snippets.h"

#include <fstream>
#include <string>

#include <xapian.h>

#include "apitest.h"
#include "backendmanager_local.h"
#include "testsuite.h"
#include "testutils.h"

#include <iostream>

using namespace std;

struct snippet_testcase {
    const char * input;
    size_t len;
    const char * expect;
};

/// Test snippets without stemming.
DEFINE_TESTCASE(snippet1, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query(Xapian::Query::OP_OR,
				    Xapian::Query("rubbish"),
				    Xapian::Query("example")));
    Xapian::MSet mset = enquire.get_mset(0, 0);

    static const snippet_testcase testcases[] = {
	// Test highlighting in full sample.
	{ "Rubbish and junk", 20, "<b>Rubbish</b> and junk" },
	{ "Project R.U.B.B.I.S.H. greenlit", 31, "Project <b>R.U.B.B.I.S.H.</b> greenlit" },
	{ "What a load of rubbish", 100, "What a load of <b>rubbish</b>" },
	{ "Example rubbish", 100, "<b>Example</b> <b>rubbish</b>" },
	{ "An example of rubbish", 100, "An <b>example</b> of <b>rubbish</b>" },
	{ "Rubbish example of rubbish", 100, "<b>Rubbish</b> <b>example</b> of <b>rubbish</b>" },

	// Test selection of snippet.
	{ "Rubbish and junk", 12, "<b>Rubbish</b> and..." },
	{ "Project R.U.B.B.I.S.H. greenlit", 14, "...<b>R.U.B.B.I.S.H.</b>..." },
	{ "What a load of rubbish", 12, "...of <b>rubbish</b>" },
	{ "What a load of rubbish", 8, "...<b>rubbish</b>" },
	{ "Rubbish example where the start is better than the rubbish ending", 18, "<b>Rubbish</b> <b>example</b>..." },

	// Should prefer "interesting" words for context.
	{ "And of the rubbish document to this", 18, "...<b>rubbish</b> document..." },
	{ "And if they document rubbish to be this", 18, "...document <b>rubbish</b>..." },
    };

    for (auto i : testcases) {
	TEST_STRINGS_EQUAL(mset.snippet(i.input, i.len), i.expect);
    }

    return true;
}

/// Test snippets with stemming.
DEFINE_TESTCASE(snippetstem1, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query(Xapian::Query::OP_OR,
				    Xapian::Query("rubbish"),
				    Xapian::Query("Zexampl")));
    Xapian::MSet mset = enquire.get_mset(0, 0);

    static const snippet_testcase testcases[] = {
	// "rubbish" isn't stemmed, example is.
	{ "You rubbished my ideas", 24, "You rubbished my ideas" },
	{ "Rubbished all my examples", 20, "...all my <b>examples</b>" },
	{ "Examples of text", 20, "<b>Examples</b> of text" },
    };

    Xapian::Stem stem("en");
    for (auto i : testcases) {
	TEST_STRINGS_EQUAL(mset.snippet(i.input, i.len, stem), i.expect);
    }

    return true;
}

/// Test snippets with phrases.
DEFINE_TESTCASE(snippetphrase1, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    Xapian::Query q(Xapian::Query::OP_PHRASE,
		    Xapian::Query("rubbish"),
		    Xapian::Query("example"));
    // Regression test - a phrase with a follow sibling query would crash in
    // the highlighting code.
    enquire.set_query(q &~ Xapian::Query("banana"));
    Xapian::MSet mset = enquire.get_mset(0, 0);

    static const snippet_testcase testcases[] = {
	{ "An example of rubbish", 18, "...example of rubbish" },
	{ "This is a rubbish example", 20, "...is a <b>rubbish example</b>" },
	{ "Example of a rubbish example of rubbish", 45, "Example of a <b>rubbish example</b> of rubbish" },
	{ "Example of a rubbish example of rubbish", 18, "...<b>rubbish example</b> of..." },
	{ "rubbish rubbish example example", 45, "rubbish <b>rubbish example</b> example" },
	{ "rubbish example rubbish example", 45, "<b>rubbish example</b> <b>rubbish example</b>" },
    };

    Xapian::Stem stem("en");
    for (auto i : testcases) {
	TEST_STRINGS_EQUAL(mset.snippet(i.input, i.len, stem), i.expect);
    }

    return true;
}

/// Index file to a DB with TermGenerator.
static void
make_tg_db(Xapian::WritableDatabase &db, const string & source)
{
    string file = test_driver::get_srcdir();
    file += "/testdata/";
    file += source;
    file += ".txt";
    ifstream input;
    input.open(file.c_str());
    if (!input.is_open()) {
	FAIL_TEST("Couldn't open input: " << file);
    }

    Xapian::TermGenerator tg;
    tg.set_stemmer(Xapian::Stem("en"));
    while (!input.eof()) {
	Xapian::Document doc;
	tg.set_document(doc);
	string line, data;
	while (true) {
	    getline(input, line);
	    if (find_if(line.begin(), line.end(), C_isnotspace) == line.end())
		break;
	    tg.index_text(line);
	    if (!data.empty()) data += ' ';
	    data += line;
	}
	doc.set_data(data);
	db.add_document(doc);
    }
}

/// Test snippets in various ways.
DEFINE_TESTCASE(snippetmisc1, generated) {
    Xapian::Database db = get_database("snippet", make_tg_db, "snippet");
    Xapian::Enquire enquire(db);
    enquire.set_weighting_scheme(Xapian::BoolWeight());
    Xapian::Stem stem("en");

    static const char * words[] = { "do", "we", "have" };
    Xapian::Query q(Xapian::Query::OP_PHRASE, words, words + 3);
    enquire.set_query(q);
    Xapian::MSet mset = enquire.get_mset(0, 6);
    TEST_EQUAL(mset.size(), 3);
    TEST_STRINGS_EQUAL(mset.snippet(mset[0].get_document().get_data(), 40, stem),
		       "...much o'brien <b>do we have</b>?  Miles O'Brien...");
    TEST_STRINGS_EQUAL(mset.snippet(mset[1].get_document().get_data(), 40, stem),
		       "...Unicode: How much o’brien <b>do we have</b>?");
    TEST_STRINGS_EQUAL(mset.snippet(mset[2].get_document().get_data(), 32, stem),
		       "We do have we <b>do we have</b> do we.");

    enquire.set_query(Xapian::Query("Zwelcom") | Xapian::Query("Zmike"));
    mset = enquire.get_mset(0, 6);
    TEST_EQUAL(mset.size(), 3);
    TEST_STRINGS_EQUAL(mset.snippet(mset[0].get_document().get_data(), 25, stem),
		       "<b>Welcome</b> to <b>Mike's</b>...");
    TEST_STRINGS_EQUAL(mset.snippet(mset[1].get_document().get_data(), 5, stem),
		       "<b>Mike</b>...");
    TEST_STRINGS_EQUAL(mset.snippet(mset[2].get_document().get_data(), 10, stem),
		       "...<b>Mike</b> can...");

    enquire.set_query(Xapian::Query(q.OP_WILDCARD, "m"));
    mset = enquire.get_mset(0, 6);
    TEST_EQUAL(mset.size(), 5);
    TEST_STRINGS_EQUAL(mset.snippet(mset[0].get_document().get_data(), 18, stem),
		       "...<b>Mike's</b> <b>Mechanical</b>...");
    TEST_STRINGS_EQUAL(mset.snippet(mset[1].get_document().get_data(), 80, stem),
		       "<b>Mike</b> <b>McDonald</b> is a <b>mechanic</b> who enjoys repairing things of a <b>mechanical</b> sort.");
    TEST_STRINGS_EQUAL(mset.snippet(mset[2].get_document().get_data(), 102, stem),
		       "From autos to zip-lines, from tea-lights to x-rays, from sea ships to u-boats - <b>Mike</b> can fix them all.");
    TEST_STRINGS_EQUAL(mset.snippet(mset[3].get_document().get_data(), 64, stem),
		       "How <b>much</b> o'brien do we have?  <b>Miles</b> O'Brien, that's how <b>much</b>.");
    // The requested length is in bytes, so the "fancy" apostrophe results in
    // fewer Unicode characters in this sample than the previous one.
    TEST_STRINGS_EQUAL(mset.snippet(mset[4].get_document().get_data(), 64, stem),
		       "...<b>much</b> o’brien do we have?  <b>Miles</b> O’Brien, that’s how <b>much</b>.");

    return true;
}
