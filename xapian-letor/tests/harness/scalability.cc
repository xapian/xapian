/** @file scalability.cc
 * @brief Test how an operation scales.
 */
/* Copyright (C) 2009 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include "scalability.h"

#include "cputimer.h"
#include "testsuite.h"

void
test_scalability(double (*func)(unsigned), unsigned n, double threshold)
{
    double time1;
    // Increase the number of tests until we take a reliably measurable amount
    // of time.
    do {
	time1 = func(n);
	tout << "Test with " << n << " repetitions took " << time1 << " secs\n";
	unsigned n_new = n * 10;
	if (n_new < n)
	    SKIP_TEST("Can't count enough repetitions to be able to time test");
	n = n_new;
    } while (time1 <= 0.001);

    double time10 = func(n);
    tout << "Test with " << n << " repetitions took " << time10 << " secs\n";

    TEST_REL(time10,<,time1 * threshold);
}
