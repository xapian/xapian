/** @file
 * @brief Xapian::DiceWeight class
 */
/* Copyright (C) 2018 Guruprasad Hegde
 * Copyright (C) 2024 Olly Betts
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
 * along with this program; if not, see
 * <https://www.gnu.org/licenses/>.
 */

#include <config.h>

#include "xapian/weight.h"

#include "xapian/error.h"

using namespace std;

namespace Xapian {

DiceWeight*
DiceWeight::clone() const
{
    return new DiceWeight();
}

void
DiceWeight::init(double factor)
{
    if (factor == 0.0) {
	// This object is for the term-independent contribution, and that's
	// always zero for this scheme.
	return;
    }

    numerator = get_wqf() * 2 * factor;

    // The Dice Coefficient formula is
    //
    //   dice_coeff(q, d) = 2.0 * (q ∩ d) / (|q| + |d|)
    //
    // where q is the set of query terms and d the set of document terms.
    //
    // The value of (q ∩ d) is the sum of wqf for query terms matching the
    // current document.  That summing is done by the matcher, and each term
    // needs to contribute:
    //
    //   2.0 * wqf / (query_length + unique_term_count)
    //
    // We multiply that by factor, which is 1.0 unless OP_SCALE_WEIGHT has
    // been applied.
    //
    //   factor * 2.0 * wqf / (query_length + unique_term_count)
    //
    // We need an upper bound on this for any document in a given database.
    // Note that wdf and query_length are determined by the query, and only
    // unique_term_count varies by document.  We want to minimise the
    // denominator and so minimise unique_term_count.
    auto denominator = get_query_length() + get_unique_terms_lower_bound();
    upper_bound = numerator / denominator;
}

string
DiceWeight::name() const
{
    return "dice";
}

string
DiceWeight::serialise() const
{
    return string();
}

DiceWeight*
DiceWeight::unserialise(const string& s) const
{
    if (rare(!s.empty())) {
	throw Xapian::SerialisationError("Extra data in "
					 "DiceWeight::unserialise()");
    }
    return new DiceWeight;
}

double
DiceWeight::get_sumpart(Xapian::termcount,
			Xapian::termcount,
			Xapian::termcount uniqterms,
			Xapian::termcount) const
{
    return numerator / (get_query_length() + uniqterms);
}

double
DiceWeight::get_maxpart() const
{
    return upper_bound;
}

DiceWeight*
DiceWeight::create_from_parameters(const char* p) const
{
    if (*p != '\0') {
	throw InvalidArgumentError("No parameters are required for DiceWeight");
    }
    return new Xapian::DiceWeight;
}

}
