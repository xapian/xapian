/** @file api_mathtermgen.cc
 * @brief Tests of Xapian::MathTermGenerator
 */
/* Copyright (C) 2018 Guruprasad Hegde
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

#include "api_mathtermgen.h"

#include <xapian.h>

#include "apitest.h"
#include "str.h"
#include "testsuite.h"
#include "testutils.h"

#include <string>

using namespace std;

struct test {
    const char * expr;
    vector<string> expected_mrow_labels;
};

static const test test_parse[] = {
    {	// Text having mix of math and text.
	" <math>"
	" <mtext> such that </mtext>"
	" <mi> x </mi>"
	" <mo> + </mo>"
	" <mi> b </mi>"
	" <mfrac> <mi> p </mi>"
	"	<mi> q </mi>"
	" </mfrac>"
	" </math>"
	" Some more text entry"
	" <nonmath> should not be parsed </nonmath>",
	{ "T!such that", "V!x", "O+", "V!b", "F" } },
    {	// Leading and trailing whitespaces in element value.
	" <math>"
	" <mi>    a</mi>"
	" <mo>*     </mo>"
	" <mi> b </mi>"
	" <mo> = </mo>"
	" <mi>    x </mi>"
	" </math>",
	{ "V!a", "O*", "V!b", "O=", "V!x" } },
    {	// Expression with fraction.
	" <math>"
	" <mfrac> <mi> p </mi>"
	"	  <mi> q </mi> </mfrac"
	" <mo> = </mo>"
	" <mfrac> <mi> c </mi>"
	"	  <mi> d </mi> </mfrac>",
	{ "F", "O=", "F" } },
    {	// Expression with root.
	" <math>"
	" <mroot> <mi> a </mi>"
	" 	  <mn> 4 </mn> </mroot>",
	{ "R", "V!a" } },
    { NULL, { } }
};

DEFINE_TESTCASE(mathtermgen1, !backend) {
    Xapian::MathTermGenerator termgen;

    for (const test * t = test_parse; t->expr; ++t) {
	termgen.index_math(t->expr);
	vector<string> labels = termgen.get_labels_list();
	TEST( labels == t->expected_mrow_labels );
    }

    return true;
}
