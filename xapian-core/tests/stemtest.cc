/* stemtest.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002 Olly Betts
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
#include <stdio.h>
#include <ctype.h>
#include <getopt.h>

#include <string>
#include <fstream>
#include <iostream>

#include "om/omstem.h"
#include "testsuite.h"

using namespace std;

static const int JUNKSIZE = 2 * 1048576;

static string language;

static OmStem stemmer("english"); // no default ctor

static string srcdir;

static int seed;

// run stemmers on random text
static bool
test_stemrandom()
{
    static const char wordchars[] =
	"abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz0123456789^\0";

    if (getenv("OM_STEMTEST_SKIP_RANDOM"))
	SKIP_TEST("OM_STEMTEST_SKIP_RANDOM set");

    tout << "Stemming random text... (seed " << seed << ")" << endl;
    srand(seed);

    string word;
    int stemmed_size = 0;
    for (int c = JUNKSIZE; c; --c) {
	char ch = wordchars[(rand() >> 8) % sizeof wordchars];
	if (ch) {
	    word += ch;
	    continue;
	}
	stemmed_size += stemmer.stem_word(word).length();
	word = "";
    }
    stemmed_size += stemmer.stem_word(word).length();
    tout << "Input size " << JUNKSIZE << ", stemmed size " << stemmed_size
	 << endl;

    if (stemmed_size > JUNKSIZE * 101 / 100) {
	FAIL_TEST("Stemmed data is significantly bigger than input: "
		  << stemmed_size << " vs. " << JUNKSIZE);
    }
    if (stemmed_size < JUNKSIZE / 2) {
	FAIL_TEST("Stemmed data is significantly smaller than input: "
		  << stemmed_size << " vs. " << JUNKSIZE);
    }
    return true;
}
	
// run stemmers on random junk
static bool
test_stemjunk()
{
    if (getenv("OM_STEMTEST_SKIP_RANDOM"))
	SKIP_TEST("OM_STEMTEST_SKIP_RANDOM set");

    tout << "Stemming random junk... (seed " << seed << ")" << endl;
    srand(seed);

    string word;
    int stemmed_size = 0;
    for (int c = JUNKSIZE; c; --c) {
	char ch = rand() >> 8;
	if (ch) {
	    word += ch;
	    continue;
	}
	stemmed_size += stemmer.stem_word(word).length();
	word = "";
    }
    stemmed_size += stemmer.stem_word(word).length();
    tout << "Input size " << JUNKSIZE << ", stemmed size " << stemmed_size
	 << endl;

    if (stemmed_size > JUNKSIZE * 101 / 100) {
	FAIL_TEST("Stemmed data is significantly bigger than input ("
		  << stemmed_size << " vs. " << JUNKSIZE);
    }
    if (stemmed_size < JUNKSIZE / 2) {
	FAIL_TEST("Stemmed data is significantly smaller than input ("
		  << stemmed_size << " vs. " << JUNKSIZE);
    }
    return true;
}

static bool
test_stemdict()
{
    string dir = srcdir + "/../../xapian-data/stemming/";

    ifstream txt((dir + language + ".voc").c_str());
    if (!txt.is_open()) {
	SKIP_TEST(language + ".voc not found");
    }

    ifstream st((dir + language + ".st").c_str());
    if (!st.is_open()) {
	txt.close();
	SKIP_TEST(language + ".st not found");
    }
 
    int wordcount = 0;

    tout << "Testing " << language << " with fixed dictionary..." << endl;

    string word, stem, expect;
    while (!txt.eof() && !st.eof()) {
	getline(txt, word);
	getline(st, expect);

	stem = stemmer.stem_word(word);

	TEST_EQUAL(stem, expect);
	++wordcount;
    }
    txt.close();
    st.close();

    return true;
}

// ##################################################################
// # End of actual tests                                            #
// ##################################################################

/// The lists of tests to perform
test_desc tests[] = {
    {"stemrandom",		test_stemrandom},
    {"stemjunk",		test_stemjunk},
    {"stemdict",		test_stemdict},
    {0, 0}
};

int main(int argc, char *argv[])
{
    srcdir = test_driver::get_srcdir(argv[0]);
    int result = 0;
    char *val;

    val = getenv("OM_STEMTEST_SEED");
    if (val && *val) {
	seed = atoi(val);
    } else {
	seed = 42; // FIXME hash hostname like stemtest.pl did???
	//$seed = unpack("%32L*", `hostname`);
    }
    cout << "The random seed is " << seed << endl;
    cout << "Please report the seed when reporting a test failure." << endl;

    string langs;
    val = getenv("OM_STEMTEST_LANGUAGES");
    if (val && *val)
	langs = val;
    else
	langs = OmStem::get_available_languages();

    string::size_type b = 0;
    while (b != langs.size()) {
	string::size_type a = b;
	while (b < langs.size() && langs[b] != ' ') ++b;
	language = langs.substr(a, b - a);
	while (b < langs.size() && langs[b] == ' ') ++b;
	cout << "Running tests with " << language << " stemmer..." << endl;
	stemmer = OmStem(language);
	result = max(result, test_driver::main(argc, argv, tests));
    }
    return result;
}
