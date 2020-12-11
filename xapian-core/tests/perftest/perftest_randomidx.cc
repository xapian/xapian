/** @file
 * @brief performance tests involving a randomly generated index
 */
/* Copyright 2008 Lemur Consulting Ltd
 * Copyright 2009,2015 Olly Betts
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

#include "perftest/perftest_randomidx.h"

#include <cstdlib>
#include <string>
#include <xapian.h>

#include "backendmanager.h"
#include "perftest.h"
#include "str.h"
#include "testrunner.h"
#include "testsuite.h"
#include "testutils.h"

using namespace std;

/** Generate a random integer from 0 to "range" - 1.
 */
static unsigned int
rand_int(unsigned int range)
{
    return unsigned(range * (rand() / (RAND_MAX + 1.0)));
}

/** Generate a random integer from min to max.
 */
static unsigned int
rand_int(unsigned int min, unsigned int max)
{
    return min + unsigned((max + 1 - min) * (rand() / (RAND_MAX + 1.0)));
}

/** Generate a random double in range 0.0 <= v < 1.0
 */
static double
rand_01()
{
    return rand() / (RAND_MAX + 1.0);
}

/** Generate a "word", of the specified length.
 *
 *  @param length     The length of the word to generate.
 *  @param char_range The range of characters to use in the word.
 */
static string
gen_word(unsigned int length, unsigned int char_range)
{
    string result;
    result.reserve(length);
    for (unsigned int i = 0; i != length; ++i) {
	char ch = char('a' + rand_int(char_range));
	result.append(1, ch);
    }
    return result;
}

// Test the performance using randomly generated data.
DEFINE_TESTCASE(randomidx1, writable && !inmemory) {
    logger.testcase_begin("randomidx1");

    std::string dbname("randomidx1");
    Xapian::WritableDatabase dbw = backendmanager->get_writable_database(dbname, "");

    unsigned int runsize = 100000;
    unsigned int seed = 42;

    // Some parameters used to control generation of documents.
    unsigned int slots_used = 10;
    double slot_probability = 0.7;
    unsigned int slotval_minlen = 1;
    unsigned int slotval_maxlen = 6;

    unsigned int minterms = 100;
    unsigned int maxterms = 1000;
    unsigned int mintermlen = 1;
    unsigned int maxtermlen = 10;
    unsigned int termcharrange = 10;

    srand(seed);

    std::map<std::string, std::string> params;
    params["runsize"] = str(runsize);
    params["seed"] = str(seed);
    params["slots_used"] = str(slots_used);
    params["slot_probability"] = str(slot_probability);
    params["slotval_minlen"] = str(slotval_minlen);
    params["slotval_maxlen"] = str(slotval_maxlen);
    params["minterms"] = str(minterms);
    params["maxterms"] = str(maxterms);
    params["mintermlen"] = str(mintermlen);
    params["maxtermlen"] = str(maxtermlen);
    params["termcharrange"] = str(termcharrange);
    logger.indexing_begin(dbname, params);

    unsigned int i;
    for (i = 0; i < runsize; ++i) {
	Xapian::Document doc;
	doc.set_data("random document " + str(i));

	unsigned int terms = rand_int(minterms, maxterms);
	for (unsigned int j = 0; j < terms; ++j) {
	    unsigned int termlen = rand_int(mintermlen, maxtermlen);
	    doc.add_term(gen_word(termlen, termcharrange));
	}

	// Add values to slots - all values are between 1 and 6 characters, but
	// later slots have a greater range of characters, so more unique
	// values.
	for (unsigned int slot = 0; slot < slots_used; ++slot) {
	    if (rand_01() < slot_probability) {
		unsigned int len = rand_int(slotval_minlen, slotval_maxlen);
		doc.add_value(slot, gen_word(len, slot + 2));
	    }
	}

	dbw.add_document(doc);
	logger.indexing_add();
    }
    dbw.commit();
    logger.indexing_end();

    logger.testcase_end();
}
