/** @file api_stem.cc
 * @brief Test the stemming API
 */
/* Copyright (C) 2010,2012 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include "api_stem.h"

#include <xapian.h>

#include "apitest.h"
#include "testsuite.h"
#include "testutils.h"

using namespace std;

class MyStemImpl : public Xapian::StemImplementation {
    string operator()(const string & word) {
	if (word == "vanish")
	    return string();
	return word.substr(0, 3);
    }

    string get_description() const {
	return "MyStem()";
    }
};

/// Test user stemming algorithms.
DEFINE_TESTCASE(stem1, !backend) {
    Xapian::Stem st(new MyStemImpl);

    TEST_EQUAL(st.get_description(), "Xapian::Stem(MyStem())");
    TEST_EQUAL(st("a"), "a");
    TEST_EQUAL(st("foo"), "foo");
    TEST_EQUAL(st("food"), "foo");

    return true;
}

/// New feature in 1.0.21/1.2.1 - "nb" and "nn" select the Norwegian stemmer.
DEFINE_TESTCASE(stem2, !backend) {
    Xapian::Stem st_norwegian("norwegian");
    TEST_EQUAL(st_norwegian.get_description(),
	       Xapian::Stem("nb").get_description());
    TEST_EQUAL(st_norwegian.get_description(),
	       Xapian::Stem("nn").get_description());
    TEST_EQUAL(st_norwegian.get_description(),
	       Xapian::Stem("no").get_description());
    TEST_NOT_EQUAL(st_norwegian.get_description(),
		   Xapian::Stem("en").get_description());
    TEST_NOT_EQUAL(st_norwegian.get_description(),
		   Xapian::Stem("none").get_description());
    return true;
}

/// Test add a stemmer test
DEFINE_TESTCASE(stem3, !backend) {
    Xapian::Stem earlyenglish("earlyenglish");
    TEST_EQUAL(earlyenglish("loved"), "love");
    TEST_EQUAL(earlyenglish("loving"), "love");
    TEST_EQUAL(earlyenglish("loveth"), "love");
    TEST_EQUAL(earlyenglish("givest"), "give");
    return true;
}

/// Test handling of a stemmer returning an empty string.
// Regression test for https://trac.xapian.org/ticket/741 fixed in 1.4.2.
DEFINE_TESTCASE(stemempty1, !backend) {
    Xapian::Stem st(new MyStemImpl);

    Xapian::TermGenerator tg;
    Xapian::Document doc;
    tg.set_document(doc);
    tg.set_stemmer(st);
    tg.set_stemming_strategy(tg.STEM_ALL);

    tg.index_text("watch me vanish now");
    auto i = doc.termlist_begin();
    TEST(i != doc.termlist_end());
    TEST_EQUAL(*i, "me");
    TEST(++i != doc.termlist_end());
    TEST_EQUAL(*i, "now");
    TEST(++i != doc.termlist_end());
    TEST_EQUAL(*i, "wat");
    TEST(++i == doc.termlist_end());

    return true;
}

/// Test invalid language names with various characters in.
DEFINE_TESTCASE(stemlangs2, !backend) {
    string lang("xdummy");
    for (unsigned ch = 0; ch <= 255; ++ch) {
	lang[0] = ch;
	TEST_EXCEPTION(Xapian::InvalidArgumentError, Xapian::Stem stem(lang));
    }
    return true;
}
