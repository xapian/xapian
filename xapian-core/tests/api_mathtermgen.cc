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
	"	</mrow> </mfrac"
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
	" 	  <mi> i </mi> </msub>",
	{ "V!a"} },
    {	// Expression with subscript.
	" <math>"
	" <msub> <mi> a </mi>"
	" 	  <mi> i </mi> </msub>",
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
	" 	  <mi> a </mi> </under>",
	{ "O-"} },
    {	// Expression with overscript.
	" <math>"
	" <mover> <mo> - </mo>"
	" 	  <mi> i </mi> </mover>",
	{ "O-"} },
    {	// Expression with underscript & overscript.
	" <math>"
	" <munderover>"
	"	<mo> &int; </mo>"
	"	<mn> 0 </mn>"
	"	<mn> 1 </mn>"
	"	</munderover> </math>",
	{ "O&int;" } },
    {	// Expression with namespace prefix.
	" <m:math>"
	" <m:mi> a </m:mi>"
	" <m:mo> / </m:mo>"
	" <m:mn> 5 </m:mn>"
	"	</m:math>",
	{ "V!a", "O/", "N!5" } },
    {	// Expression with namespace prefix.
	" <mn:math>"
	" <mn:mi> a </mn:mi>"
	" <mn:mo> / </mn:mo>"
	" <mn:mn> 5 </mn:mn>"
	"	</mn:math>",
	{ "V!a", "O/", "N!5" } },
    {	// Expression from wikidata.
	R"(<m:math alttext="Z(G)(f(z),f(z^{2}),\ldots,f(z^{n})).\," display="inline">
          <m:semantics>
            <m:mrow>
              <m:mrow>
                <m:mi>Z</m:mi>
                <m:mo>⁢</m:mo>
                <m:mrow>
                  <m:mo>(</m:mo>
                  <m:mi>G</m:mi>
                  <m:mo>)</m:mo>
                </m:mrow>
                <m:mo>⁢</m:mo>
                <m:mrow>
                  <m:mo>(</m:mo>
                  <m:mrow>
                    <m:mrow>
                      <m:mi>f</m:mi>
                      <m:mo>⁢</m:mo>
                      <m:mrow>
                        <m:mo>(</m:mo>
                        <m:mi>z</m:mi>
                        <m:mo>)</m:mo>
                      </m:mrow>
                    </m:mrow>
                    <m:mo>,</m:mo>
                    <m:mrow>
                      <m:mi>f</m:mi>
                      <m:mo>⁢</m:mo>
                      <m:mrow>
                        <m:mo>(</m:mo>
                        <m:msup>
                          <m:mi>z</m:mi>
                          <m:mn>2</m:mn>
                        </m:msup>
                        <m:mo>)</m:mo>
                      </m:mrow>
                    </m:mrow>
                    <m:mo>,</m:mo>
                    <m:mi mathvariant="normal">…</m:mi>
                    <m:mo>,</m:mo>
                    <m:mrow>
                      <m:mi>f</m:mi>
                      <m:mo>⁢</m:mo>
                      <m:mrow>
                        <m:mo>(</m:mo>
                        <m:msup>
                          <m:mi>z</m:mi>
                          <m:mi>n</m:mi>
                        </m:msup>
                        <m:mo>)</m:mo>
                      </m:mrow>
                    </m:mrow>
                  </m:mrow>
                  <m:mo>)</m:mo>
                </m:mrow>
              </m:mrow>
              <m:mo>.</m:mo>
            </m:mrow>
            <m:annotation encoding="application/x-tex">Z(G)(f(z),f(z^{2}),\ldots,f(z^{n})).\,</m:annotation>
          </m:semantics>
        </m:math>)",
    { "V!Z", invisible_times, "O(", "V!G", "O)", invisible_times, "O(", "V!f", invisible_times, "O(", "V!z", "O)", "O,", "V!f", invisible_times, "O(", "V!z",
      "O)", "O,", "V!…", "O,", "V!f", invisible_times, "O(", "V!z", "O)", "O)", "O." } },
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
