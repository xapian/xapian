/** @file soaktest.cc
 * @brief Long-running "soak" tests for Xapian.
 */
/* Copyright (C) 2010 Richard Boulton
 * Copyright (C) 2011,2013 Olly Betts
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

#include "soaktest/soaktest.h"

#include "soaktest/soaktest_all.h"
#include "testrunner.h"
#include "testsuite.h"

// random() and srandom() aren't in <cstdlib> with Sun's compiler.
#include <stdlib.h>

using namespace std;

unsigned int g_random_seed;

extern unsigned int initrand()
{
    tout << "Setting random seed to " << g_random_seed << "\n";
#if defined HAVE_SRANDOM && defined HAVE_RANDOM
    srandom(g_random_seed);
#else
    srand(g_random_seed);
#endif
    return g_random_seed;
}

extern unsigned int randint(unsigned int s)
{
#if defined HAVE_SRANDOM && defined HAVE_RANDOM
    unsigned long int r = static_cast<unsigned long int>(random());
    r = r % static_cast<unsigned long int>(s);
    return static_cast<unsigned int>(r);
#else
    unsigned int r = static_cast<unsigned long int>(rand());
    // The low order bits have lousy randomness on some platforms.
    r = r / (RAND_MAX / s);
    return r;
#endif
}

class SoakTestRunner : public TestRunner
{
    string seed_str;
  public:
    SoakTestRunner() : seed_str("42") {
	test_driver::add_command_line_option("seed", 's', &seed_str);
    }

    int run() const {
	int result = 0;
	g_random_seed = atoi(seed_str.c_str());
#include "soaktest/soaktest_collated.h"
	return result;
    }
};

int main(int argc, char **argv)
{
    SoakTestRunner runner;
    return runner.run_tests(argc, argv);
}
