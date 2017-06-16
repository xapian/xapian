/* api_snippets.cc: tests snippets
 *
 * Copyright 2012 Mihai Bivol
 * Copyright 2015,2016,2017 Olly Betts
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
				    Xapian::Query("mention")));
    Xapian::MSet mset = enquire.get_mset(0, 0);

    static const snippet_testcase testcases[] = {
	// Test highlighting in full sample.
	{ "Rubbish and junk", 20, "<b>Rubbish</b> and junk" },
	{ "Project R.U.B.B.I.S.H. greenlit", 31, "Project <b>R.U.B.B.I.S.H.</b> greenlit" },
	{ "What a load of rubbish", 100, "What a load of <b>rubbish</b>" },
	{ "Mention rubbish", 100, "<b>Mention</b> <b>rubbish</b>" },
	{ "A mention of rubbish", 100, "A <b>mention</b> of <b>rubbish</b>" },
	{ "Rubbish mention of rubbish", 100, "<b>Rubbish</b> <b>mention</b> of <b>rubbish</b>" },

	// Test selection of snippet.
	{ "Rubbish and junk", 12, "<b>Rubbish</b> and..." },
	{ "Project R.U.B.B.I.S.H. greenlit", 14, "...<b>R.U.B.B.I.S.H.</b>..." },
	{ "What a load of rubbish", 12, "...of <b>rubbish</b>" },
	{ "What a load of rubbish", 8, "...<b>rubbish</b>" },
	{ "Rubbish mention where the start is better than the rubbish ending", 18, "<b>Rubbish</b> <b>mention</b>..." },

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

    // Term Zexampl isn't in the database, but the highlighter should still
    // handle it.
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
		    Xapian::Query("mention"));
    // Regression test - a phrase with a following sibling query would crash in
    // the highlighting code.
    enquire.set_query(q &~ Xapian::Query("banana"));
    Xapian::MSet mset = enquire.get_mset(0, 0);

    static const snippet_testcase testcases[] = {
	{ "A mention of rubbish", 18, "...mention of rubbish" },
	{ "This is a rubbish mention", 20, "...is a <b>rubbish mention</b>" },
	{ "Mention of a rubbish mention of rubbish", 45, "Mention of a <b>rubbish mention</b> of rubbish" },
	{ "Mention of a rubbish mention of rubbish", 18, "...<b>rubbish mention</b> of..." },
	{ "rubbish rubbish mention mention", 45, "rubbish <b>rubbish mention</b> mention" },
	{ "rubbish mention rubbish mention", 45, "<b>rubbish mention</b> <b>rubbish mention</b>" },
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
		       "How much o'brien <b>do we have</b>?  Miles...");
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

/// Test snippet term diversity.
DEFINE_TESTCASE(snippet_termcover1, backend) {
    static const snippet_testcase testcases[] = {
	// "Zexample" isn't in the database, so should get termweight 0.  Once
	// max_tw is added on, "rubbish" should have just under twice the
	// relevance of "example" so clearly should win in a straight fight.
	{ "A rubbish, but a good example", 14, "...<b>rubbish</b>, but a..."},
	// But a second occurrence of "rubbish" has half the relevance, so
	// "example" should add slightly more relevance.
	{ "Rubbish and rubbish, and rubbish examples", 22, "...and <b>rubbish</b> <b>examples</b>"},
	// And again.
	{ "rubbish rubbish example rubbish rubbish", 16, "...<b>example</b> <b>rubbish</b>..." },
    };

    Xapian::Stem stem("en");
    // Disable SNIPPET_BACKGROUND_MODEL so we can test the relevance decay
    // for repeated terms.
    unsigned flags = Xapian::MSet::SNIPPET_EXHAUSTIVE;
    for (auto i : testcases) {
	Xapian::Enquire enquire(get_database("apitest_simpledata"));
	enquire.set_query(Xapian::Query(Xapian::Query::OP_OR,
		    Xapian::Query("rubbish"),
		    Xapian::Query("Zexampl")));

	Xapian::MSet mset = enquire.get_mset(0, 0);
	TEST_STRINGS_EQUAL(mset.snippet(i.input, i.len, stem, flags), i.expect);
    }

    return true;
}

/// Test snippet term diversity cases with BoolWeight.
DEFINE_TESTCASE(snippet_termcover2, backend) {
    // With BoolWeight, all terms have 0 termweight, and so relevance 1.0
    // (since max_tw is set to 1.0 if it is zero).
    static const snippet_testcase testcases[] = {
	// Diversity should pick two different terms in preference.
	{ "rubbish rubbish example rubbish rubbish", 16, "...<b>example</b> <b>rubbish</b>..." },
	// And again.
	{ "Rubbish and rubbish, and rubbish examples", 22, "...and <b>rubbish</b> <b>examples</b>"},
	// The last of two equal snippet should win.
	{ "A rubbish, but a good example", 14, "...a good <b>example</b>"},
    };

    Xapian::Stem stem("en");
    // Disable SNIPPET_BACKGROUND_MODEL so we can test the relevance decay
    // for repeated terms.
    unsigned flags = Xapian::MSet::SNIPPET_EXHAUSTIVE;
    for (auto i : testcases) {
	Xapian::Enquire enquire(get_database("apitest_simpledata"));
	enquire.set_query(Xapian::Query(Xapian::Query::OP_OR,
		    Xapian::Query("rubbish"),
		    Xapian::Query("Zexampl")));
	enquire.set_weighting_scheme(Xapian::BoolWeight());

	Xapian::MSet mset = enquire.get_mset(0, 0);
	TEST_STRINGS_EQUAL(mset.snippet(i.input, i.len, stem, flags), i.expect);
    }

    return true;
}

/// Test snippet EMPTY_WITHOUT_MATCH flag
DEFINE_TESTCASE(snippet_empty, backend) {

    Xapian::Stem stem("en");

    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query(Xapian::Query::OP_OR,
		      Xapian::Query("rubbish"),
		      Xapian::Query("Zexampl")));

    Xapian::MSet mset = enquire.get_mset(0, 0);

    // A non-matching text
    const char *input = "A string without a match.";
    size_t len = strlen(input);

    // By default, snippet() returns len bytes of input without markup
    unsigned flags = 0;
    TEST_STRINGS_EQUAL(mset.snippet(input, len, stem, 0), input);

    // force snippet() to return the empty string if no term got matched
    flags |= Xapian::MSet::SNIPPET_EMPTY_WITHOUT_MATCH;
    TEST_STRINGS_EQUAL(mset.snippet(input, len, stem, flags), "");

    // A text with a match
    input = "A rubbish example text";
    len = strlen(input);

    flags = 0;
    TEST_STRINGS_EQUAL(mset.snippet(input, len, stem, flags),
		       "A <b>rubbish</b> <b>example</b> text");

    flags |= Xapian::MSet::SNIPPET_EMPTY_WITHOUT_MATCH;
    TEST_STRINGS_EQUAL(mset.snippet(input, len, stem, flags),
		       "A <b>rubbish</b> <b>example</b> text");

    return true;
}
