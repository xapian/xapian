/** @file
 * @brief tests snippets
 */
/* Copyright 2012 Mihai Bivol
 * Copyright 2015,2016,2017,2019,2020 Olly Betts
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
#include "testsuite.h"
#include "testutils.h"

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
DEFINE_TESTCASE(snippetmisc1, backend) {
    Xapian::Database db = get_database("snippet", make_tg_db, "snippet");
    Xapian::Enquire enquire(db);
    enquire.set_weighting_scheme(Xapian::BoolWeight());
    Xapian::Stem stem("en");

    static const char * const words[] = { "do", "we", "have" };
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
		       "\"<b>Welcome</b> to <b>Mike's</b>...");
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
}

/// Check snippets include certain preceding punctuation.
DEFINE_TESTCASE(snippet_start_nonspace, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query("foo") | Xapian::Query("10"));

    Xapian::MSet mset = enquire.get_mset(0, 0);

    Xapian::Stem stem;

    const char *input = "[xapian-devel] Re: foo";
    TEST_STRINGS_EQUAL(mset.snippet(input, strlen(input), stem),
		       "[xapian-devel] Re: <b>foo</b>");

    input = "bar [xapian-devel] Re: foo";
    TEST_STRINGS_EQUAL(mset.snippet(input, 24, stem),
		       "...[xapian-devel] Re: <b>foo</b>");

    input = "there is a $1000 prize for foo";
    TEST_STRINGS_EQUAL(mset.snippet(input, 20, stem),
		       "...$1000 prize for <b>foo</b>");

    input = "-1 is less than foo";
    TEST_STRINGS_EQUAL(mset.snippet(input, strlen(input), stem),
		       "-1 is less than <b>foo</b>");

    input = "+1 is less than foo";
    TEST_STRINGS_EQUAL(mset.snippet(input, strlen(input), stem),
		       "+1 is less than <b>foo</b>");

    input = "/bin/sh is a foo";
    TEST_STRINGS_EQUAL(mset.snippet(input, strlen(input), stem),
		       "/bin/sh is a <b>foo</b>");

    input = "'tis pity foo is a bar";
    TEST_STRINGS_EQUAL(mset.snippet(input, strlen(input), stem),
		       "'tis pity <b>foo</b> is a bar");

    input = "\"foo bar\" he whispered";
    TEST_STRINGS_EQUAL(mset.snippet(input, 11, stem),
		       "\"<b>foo</b> bar\" he...");

    input = "\\\\server\\share\\foo is a UNC path";
    TEST_STRINGS_EQUAL(mset.snippet(input, strlen(input), stem),
		       "\\\\server\\share\\<b>foo</b> is a UNC path");

    input = "«foo» is a placeholder";
    TEST_STRINGS_EQUAL(mset.snippet(input, 9, stem),
		       "«<b>foo</b>» is...");

    input = "#include <foo.h> to use libfoo";
    TEST_STRINGS_EQUAL(mset.snippet(input, 12, stem),
		       "...&lt;<b>foo</b>.h&gt; to...");

    input = "¡foo!";
    TEST_STRINGS_EQUAL(mset.snippet(input, strlen(input), stem),
		       "¡<b>foo</b>!");

    input = "¿foo?";
    TEST_STRINGS_EQUAL(mset.snippet(input, strlen(input), stem),
		       "¿<b>foo</b>?");

    input = "(foo) test";
    TEST_STRINGS_EQUAL(mset.snippet(input, strlen(input), stem),
		       "(<b>foo</b>) test");

    input = "{foo} test";
    TEST_STRINGS_EQUAL(mset.snippet(input, strlen(input), stem),
		       "{<b>foo</b>} test");

    input = "`foo` test";
    TEST_STRINGS_EQUAL(mset.snippet(input, strlen(input), stem),
		       "`<b>foo</b>` test");

    input = "@foo@ is replaced";
    TEST_STRINGS_EQUAL(mset.snippet(input, strlen(input), stem),
		       "@<b>foo</b>@ is replaced");

    input = "%foo is a perl hash";
    TEST_STRINGS_EQUAL(mset.snippet(input, strlen(input), stem),
		       "%<b>foo</b> is a perl hash");

    input = "&foo takes the address of foo";
    TEST_STRINGS_EQUAL(mset.snippet(input, strlen(input), stem),
		       "&amp;<b>foo</b> takes the address of <b>foo</b>");

    input = "§3.1.4 foo";
    TEST_STRINGS_EQUAL(mset.snippet(input, strlen(input), stem),
		       "§3.1.4 <b>foo</b>");

    input = "#foo";
    TEST_STRINGS_EQUAL(mset.snippet(input, strlen(input), stem),
		       "#<b>foo</b>");

    input = "~foo~ test";
    TEST_STRINGS_EQUAL(mset.snippet(input, strlen(input), stem),
		       "~<b>foo</b>~ test");

    input = "( foo )";
    TEST_STRINGS_EQUAL(mset.snippet(input, strlen(input), stem),
		       "<b>foo</b>...");

    input = "(=foo=)";
    TEST_STRINGS_EQUAL(mset.snippet(input, strlen(input), stem),
		       "<b>foo</b>...");

    // Check that excessive non-word characters aren't included.
    input = "((((((foo";
    TEST_STRINGS_EQUAL(mset.snippet(input, strlen(input), stem),
		       "<b>foo</b>");

    // Check we don't include characters that aren't useful.
    input = "bar,foo!";
    TEST_STRINGS_EQUAL(mset.snippet(input, 5, stem),
		       "...<b>foo</b>!");

    // Check trailing characters are included when useful.
    input = "/opt/foo/bin/";
    TEST_STRINGS_EQUAL(mset.snippet(input, strlen(input), stem),
		       "/opt/<b>foo</b>/bin/");

    input = "\"foo bar\"";
    TEST_STRINGS_EQUAL(mset.snippet(input, strlen(input), stem),
		       "\"<b>foo</b> bar\"");

    input = "\\\\server\\share\\foo\\";
    TEST_STRINGS_EQUAL(mset.snippet(input, strlen(input), stem),
		       "\\\\server\\share\\<b>foo</b>\\");

    input = "«foo»";
    TEST_STRINGS_EQUAL(mset.snippet(input, strlen(input), stem),
		       "«<b>foo</b>»");

    input = "#include <foo>";
    TEST_STRINGS_EQUAL(mset.snippet(input, strlen(input), stem),
		       "#include &lt;<b>foo</b>&gt;");

    input = "(foo)";
    TEST_STRINGS_EQUAL(mset.snippet(input, strlen(input), stem),
		       "(<b>foo</b>)");

    input = "{foo}";
    TEST_STRINGS_EQUAL(mset.snippet(input, strlen(input), stem),
		       "{<b>foo</b>}");

    input = "[foo]";
    TEST_STRINGS_EQUAL(mset.snippet(input, strlen(input), stem),
		       "[<b>foo</b>]");

    input = "`foo`";
    TEST_STRINGS_EQUAL(mset.snippet(input, strlen(input), stem),
		       "`<b>foo</b>`");

    input = "@foo@";
    TEST_STRINGS_EQUAL(mset.snippet(input, strlen(input), stem),
		       "@<b>foo</b>@");

    input = "foo for 10¢";
    TEST_STRINGS_EQUAL(mset.snippet(input, strlen(input), stem),
		       "<b>foo</b> for <b>10</b>¢");
}

/// Test snippets with small and zero length.
DEFINE_TESTCASE(snippet_small_zerolength, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query(Xapian::Query::OP_OR,
				    Xapian::Query("rubbish"),
				    Xapian::Query("mention")));
    Xapian::MSet mset = enquire.get_mset(0, 0);

    static const snippet_testcase testcases[] = {
	// Test with small length
	{ "mention junk rubbish", 3, "" },
	{ "Project R.U.B.B.I.S.H. greenlit", 5, "" },
	{ "What load rubbish", 3, "" },
	{ "Mention rubbish", 4, "" },

	// Test with zero length.
	{ "Rubbish and junk", 0, "" },
	{ "Project R.U.B.B.I.S.H. greenlit", 0, "" },
	{ "What a load of rubbish", 0, "" },
	{ "rubbish mention rubbish mention", 0, "" },
    };

    for (auto i : testcases) {
	TEST_STRINGS_EQUAL(mset.snippet(i.input, i.len), i.expect);
    }
}

/// Test ngrams.
DEFINE_TESTCASE(snippet_ngrams, backend) {
    Xapian::Database db = get_database("snippet_ngrams",
	[](Xapian::WritableDatabase& wdb,
	   const string&)
	{
	    Xapian::Document doc;
	    Xapian::TermGenerator tg;
	    tg.set_flags(Xapian::TermGenerator::FLAG_NGRAMS);
	    tg.set_document(doc);
	    tg.index_text("明末時已經有香港地方的概念");
	    wdb.add_document(doc);
	});
    Xapian::Enquire enquire(db);
    Xapian::QueryParser qp;
    auto q = qp.parse_query("已經完成", qp.FLAG_DEFAULT | qp.FLAG_NGRAMS);
    enquire.set_query(q);

    Xapian::MSet mset = enquire.get_mset(0, 0);

    Xapian::Stem stem;
    const char *input = "明末時已經有香港地方的概念";
    size_t len = strlen(input);

    unsigned flags = Xapian::MSet::SNIPPET_NGRAMS;
    string s;
    s = mset.snippet(input, len, stem, flags, "<b>", "</b>", "...");
    TEST_STRINGS_EQUAL(s, "明末時<b>已</b><b>經</b>有香港地方的概念");

    s = mset.snippet(input, len / 2, stem, flags, "<b>", "</b>", "...");
    TEST_STRINGS_EQUAL(s, "...<b>已</b><b>經</b>有香港地...");
}

DEFINE_TESTCASE(snippet_empty_mset, backend) {
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query());
    Xapian::MSet mset = enquire.get_mset(0, 0);
    TEST_STRINGS_EQUAL(mset.snippet("foo", 3), "foo");
}

DEFINE_TESTCASE(snippet_empty_mset2, !backend) {
    Xapian::MSet mset;
    TEST_STRINGS_EQUAL(mset.snippet("foo", 3), "foo");
}
