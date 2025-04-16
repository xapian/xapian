/** @file
 * @brief Test stemming algorithms
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2007,2008,2009,2012,2015,2025 Olly Betts
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

#include <cstdlib>

#include <string>
#include <iostream>

#include <zlib.h>

#include <xapian.h>
#include "parseint.h"
#include "testsuite.h"

using namespace std;

static const int JUNKSIZE = 2 * 1048576;

static string language;

static Xapian::Stem stemmer;

static string srcdir;

static int seed;

// run stemmers on random text
static void
test_stemrandom()
{
    static const char wordchars[] =
	"abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz0123456789^\0";

    tout << "Stemming random text... (seed " << seed << ")\n";
    srand(seed);

    string word;
    int stemmed_size = 0;
    for (int c = JUNKSIZE; c; --c) {
	char ch = wordchars[(rand() >> 8) % sizeof wordchars];
	if (ch) {
	    word += ch;
	    continue;
	}
	stemmed_size += stemmer(word).length();
	word.resize(0);
    }
    stemmed_size += stemmer(word).length();
    tout << "Input size " << JUNKSIZE << ", stemmed size " << stemmed_size
	 << '\n';

    if (stemmed_size > JUNKSIZE * 101 / 100) {
	FAIL_TEST("Stemmed data is significantly bigger than input: "
		  << stemmed_size << " vs. " << JUNKSIZE);
    }
    if (stemmed_size < JUNKSIZE / 2) {
	FAIL_TEST("Stemmed data is significantly smaller than input: "
		  << stemmed_size << " vs. " << JUNKSIZE);
    }
}

// run stemmers on random junk
static void
test_stemjunk()
{
    tout << "Stemming random junk... (seed " << seed << ")\n";
    srand(seed);

    string word;
    int stemmed_size = 0;
    for (int c = JUNKSIZE; c; --c) {
	char ch = char(rand() >> 8);
	if (ch) {
	    word += ch;
	    continue;
	}
	stemmed_size += stemmer(word).length();
	word.resize(0);
    }
    stemmed_size += stemmer(word).length();
    tout << "Input size " << JUNKSIZE << ", stemmed size " << stemmed_size
	 << '\n';

    if (stemmed_size > JUNKSIZE * 101 / 100) {
	FAIL_TEST("Stemmed data is significantly bigger than input ("
		  << stemmed_size << " vs. " << JUNKSIZE);
    }
    if (stemmed_size < JUNKSIZE / 2) {
	FAIL_TEST("Stemmed data is significantly smaller than input ("
		  << stemmed_size << " vs. " << JUNKSIZE);
    }
}

static void
test_stemdict()
{
    string dir = srcdir + "/../../xapian-data/stemming/";

    gzFile voc = gzopen((dir + language + "/voc.txt").c_str(), "rb");
    if (!voc) {
	voc = gzopen((dir + language + "/voc.txt.gz").c_str(), "rb");
	if (!voc) {
	    SKIP_TEST(language << "/voc.txt not found");
	}
    }

    gzFile st = gzopen((dir + language + "/output.txt").c_str(), "rb");
    if (!st) {
	st = gzopen((dir + language + "/output.txt.gz").c_str(), "rb");
	if (!st) {
	    gzclose(voc);
	    FAIL_TEST(language << "/output.txt not found");
	}
    }

    tout << "Testing " << language << " with Snowball dictionary...\n";

    int pass = 1;
    string word, expect;
    while (true) {
	while (!gzeof(voc) && !gzeof(st)) {
	    word.clear();
	    while (true) {
		int ch = gzgetc(voc);
		if (ch == EOF || ch == '\n') break;
		word += ch;
	    }

	    expect.clear();
	    while (true) {
		int ch = gzgetc(st);
		if (ch == EOF || ch == '\n') break;
		expect += ch;
	    }

	    string stem = stemmer(word);

	    TEST_EQUAL(stem, expect);
	}
	gzclose(voc);
	gzclose(st);

	if (pass == 2) break;

	voc = gzopen((dir + language + "/voc2.txt").c_str(), "rb");
	if (!voc) break;

	st = gzopen((dir + language + "/output2.txt").c_str(), "rb");
	if (!st) {
	    gzclose(voc);
	    FAIL_TEST(language << "/output2.txt not found");
	}
	tout << "Testing " << language << " with supplemental dictionary...\n";
	++pass;
    }
}

// ##################################################################
// # End of actual tests                                            #
// ##################################################################

/// The lists of tests to perform
static const test_desc tests[] = {
    {"stemrandom",		test_stemrandom},
    {"stemjunk",		test_stemjunk},
    {"stemdict",		test_stemdict},
    {0, 0}
};

int main(int argc, char **argv)
try {
    string langs = Xapian::Stem::get_available_languages();
    test_driver::add_command_line_option("languages", 'l', &langs);

    seed = 42;
    string seed_str;
    test_driver::add_command_line_option("seed", 's', &seed_str);

    test_driver::parse_command_line(argc, argv);
    srcdir = test_driver::get_srcdir();
    int result = 0;

    if (!seed_str.empty()) {
	if (!parse_signed(seed_str.c_str(), seed)) {
	    throw "seed must be an integer";
	}
    }
    cout << "The random seed is " << seed << '\n';
    cout << "Please report the seed when reporting a test failure.\n";

    string::size_type b = 0;
    while (b != langs.size()) {
	string::size_type a = b;
	while (b < langs.size() && langs[b] != ' ') ++b;
	language.assign(langs, a, b - a);
	while (b < langs.size() && langs[b] == ' ') ++b;
	cout << "Running tests with " << language << " stemmer...\n";
	stemmer = Xapian::Stem(language);
	result = max(result, test_driver::run(tests));
    }
    return result;
} catch (const char * e) {
    cout << e << '\n';
    return 1;
}
