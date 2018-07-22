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

#include <iostream>
#include <string>

using namespace std;

const string invisible_times = "O\342\201\242";

struct test {
    const char * expr;
    vector<string> expected_mrow_labels;
};

static const test test_parse[] = {
    {	// Expression with matrix.
	"<math>"
	"<mtable>"
	"<mtr>"
	"	<mtd> <mrow> <mn> 1 </mn> </mrow> </mtd>"
	"	<mtd> <mn> 0 </mn> </mtd>"
	"	<mtd> <mn> 0 </mn> </mtd>"
	"</mtr>"
	"<mtr>"
	"	<mtd> <mn> 0 </mn> </mtd>"
	"	<mtd> <mn> 1 </mn> </mtd>"
	"	<mtd> <mn> 0 </mn> </mtd>"
	"</mtr>"
	"</mtable>"
	"</math>",
	{ "M!2" } },
    {	// Expression with fraction.
	" <math>"
	" <mfrac> <mi> p </mi>"
	"	  <mi> q </mi> </mfrac>"
	" <mo> = </mo>"
	" <mfrac> <mi> c </mi>"
	"	  <mi> d </mi> </mfrac> </math>",
	{ "F", "O=", "F" } },
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
	"	  <mi> q </mi> </mfrac>"
	" <mo> = </mo>"
	" <mfrac> <mi> c </mi>"
	"	  <mi> d </mi> </mfrac> </math>",
	{ "F", "O=", "F" } },
    {	// Expression with fraction having sub-expression.
	" <math>"
	" <mfrac> <mrow> "
	"		<mi> p </mi>"
	"		<mo> + </mo>"
	"		<mi> q </mi>"
	"	</mrow>"
	"	<mrow>"
	"		<mi> a </mi>"
	"		<mo> - </mo>"
	"		<mi> b </mi>"
	"	</mrow> </mfrac>"
	"	</math>",
	{ "F"} },
    {	// Expression with root.
	" <math>"
	" <mroot> <mi> a </mi>"
	" 	  <mn> 4 </mn> </mroot>",
	{ "R", "V!a" } },
    {	// Expression with square root.
	" <math>"
	" <msqrt> <mi> a </mi>"
	"		</msqrt> </math>",
	{ "R" } },
    {   // Expression with square root having subexpression.
	" <math>"
	" <msqrt> <mrow>"
	"	  <mi> a </mi>"
	"	  <mo> + </mo>"
	"	  <mn> 2 </mn>"
	"		 </mrow>"
	"			</msqrt>"
	"				</math>",
	{ "R" } },
    {	// Expression with superscript.
	" <math>"
	" <msup> <mi> a </mi>"
	" 	  <mi> i </mi> </msup> </math>",
	{ "V!a"} },
    {	// Expression with subscript.
	" <math>"
	" <msub> <mi> a </mi>"
	" 	  <mi> i </mi> </msub> </math>",
	{ "V!a"} },
    {	// Expression with subscript & superscript.
	" <math>"
	" <msubsup>"
	"	<mo> &int; </mo>"
	"	<mn> 0 </mn>"
	"	<mn> 1 </mn>"
	"	</msubsup> </math>",
	{ "O&int;" } },
    {	// Expression with underscript.
	" <math>"
	" <munder> <mo> - </mo>"
	" 	  <mi> a </mi> </munder>",
	{ "O-"} },
    {	// Expression with overscript.
	" <math>"
	" <mover> <mo> - </mo>"
	" 	  <mi> i </mi> </mover> </math>",
	{ "O-"} },
    {	// Expression with underscript & overscript.
	" <math>"
	" <munderover>"
	"	<mo> &int; </mo>"
	"	<mn> 0 </mn>"
	"	<mn> 1 </mn>"
	"	</munderover> </math>",
	{ "O&int;" } },
    // Testcases with namespace prefix,
    {	// Expression with token elements.
	" <m:math>"
	" <m:mi> a </m:mi>"
	" <m:mo> / </m:mo>"
	" <m:mn> 5 </m:mn>"
	"	</m:math>",
	{ "V!a", "O/", "N!5" } },
    {	// Expression with different namespace prefix.
	" <mn:math>"
	" <mn:mi> a </mn:mi>"
	" <mn:mo> / </mn:mo>"
	" <mn:mn> 5 </mn:mn>"
	"	</mn:math>",
	{ "V!a", "O/", "N!5" } },
    {	// Expression with superscript.
	" <m:math>"
	" <m:msup> <m:mi> a </m:mi>"
	" 	  <m:mi> i </m:mi> </m:msup> </m:math>",
	{ "V!a"} },
    {	// Expression with subscript.
	" <m:math>"
	" <m:msub> <m:mi> a </m:mi>"
	" 	  <m:mi> i </m:mi> </m:msub> </m:math>",
	{ "V!a"} },
    {	// Expression with subscript & superscript.
	" <m:math>"
	" <m:msubsup>"
	"	<m:mo> &int; </m:mo>"
	"	<m:mn> 0 </m:mn>"
	"	<m:mn> 1 </m:mn>"
	"	</m:msubsup> </m:math>",
	{ "O&int;" } },
    {	// Expression with underscript.
	" <m:math>"
	" <m:munder> <m:mo> - </m:mo>"
	" 	  <m:mi> a </m:mi> </m:munder>",
	{ "O-"} },
    {	// Expression with overscript.
	" <m:math>"
	" <m:mover> <m:mo> - </m:mo>"
	" 	  <m:mi> i </m:mi> </m:mover> </m:math>",
	{ "O-"} },
    {	// Expression with underscript & overscript.
	" <m:math>"
	" <m:munderover>"
	"	<m:mo> &int; </m:mo>"
	"	<m:mn> 0 </m:mn>"
	"	<m:mn> 1 </m:mn>"
	"	</m:munderover> </m:math>",
	{ "O&int;" } },
	{ NULL, { } }
};

DEFINE_TESTCASE(mathtermgen1, !backend) {
    Xapian::MathTermGenerator termgen;

    for (const test * t = test_parse; t->expr; ++t) {
	termgen.index_math(t->expr);
	vector<string> labels = termgen.get_labels_list();
	TEST(labels == t->expected_mrow_labels);
    }

    return true;
}

DEFINE_TESTCASE(mathparsefail1, !backend) {
    Xapian::MathTermGenerator termgen;

    // Expression doesn't contain close tag </mo>.
    const char * invalid_formula = {
	"<math> <mi> a  <mo> + </mo> <mi> b </mi> </math>"
    };

    termgen.index_math(invalid_formula);
    TEST_EQUAL(termgen.parse_error(), true);

    return true;
}
