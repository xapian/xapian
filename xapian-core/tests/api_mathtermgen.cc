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

DEFINE_TESTCASE(mathtermgen1, !backend) {
    Xapian::MathTermGenerator termgen;

    std::string formula {
    " <math>"
    "	<mtext> such that </mtext>"
    "	<mi> x </mi>"
    "   <mo> + </mo>"
    " 	<mi> a </mi>"
    "   <mo> / </mo>"
    "   <mi> b </mi>"
    "	<mfrac> <mi> p </mi>"
    "		<mi> q </mi>"
    "	</mfrac>"
    " </math>"
    " Some text entry."
    " Some more text entry"
    " <nonmath> should not be parsed </nonmath>"
    };

    formula.push_back('\0');

    termgen.index_math(formula.data());

    // Check symbol count on main line.
    TEST_EQUAL(termgen.symbol_count(), 7);

    return true;
}
