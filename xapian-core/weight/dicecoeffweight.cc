/** @file dicecoeffweight.cc
 * @brief Xapian::DiceCoeffWeight class
 */
/* Copyright (C) 2018 Guruprasad Hegde
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <config.h>

#include "xapian/weight.h"

#include "xapian/error.h"

using namespace std;

namespace Xapian {

DiceCoeffWeight *
DiceCoeffWeight::clone() const
{
    return new DiceCoeffWeight();
}

void
DiceCoeffWeight::init(double factor_)
{
    if (factor_ == 0.0) {
	// This object is for the term-independent contribution, and that's
	// always zero for this scheme.
	return;
    }

    factor = get_wqf() * factor_;

    // FIXME: Upper bound computation:
    // dice_coeff(q, d) = 2.0 * (q ∩ d) / (|q| + |d|)
    // To maximize the result minimize the denominator, hence
    // |q| = 1, |d| = length of smallest document.
    upper_bound = factor * (2.0 / (1 + get_doclength_lower_bound()));
}

string
DiceCoeffWeight::name() const
{
    return "Xapian::DiceCoeffWeight";
}

string
DiceCoeffWeight::short_name() const
{
    return "dicecoeff";
}

string
DiceCoeffWeight::serialise() const
{
    return string();
}

DiceCoeffWeight *
DiceCoeffWeight::unserialise(const string & s) const
{
    if (rare(!s.empty()))
	throw Xapian::SerialisationError("Extra data in\
		DiceCoeffWeight::unserialise()");
    return new DiceCoeffWeight;
}

double
DiceCoeffWeight::get_sumpart(Xapian::termcount wdf, Xapian::termcount,
			     Xapian::termcount uniqterms) const
{
    if (wdf == 0) return 0.0;
    return factor * 2.0 / (get_query_length() + uniqterms);
}

double
DiceCoeffWeight::get_maxpart() const
{
    return upper_bound;
}

double
DiceCoeffWeight::get_sumextra(Xapian::termcount, Xapian::termcount) const
{
    return 0;
}

double
DiceCoeffWeight::get_maxextra() const
{
    return 0;
}

DiceCoeffWeight *
DiceCoeffWeight::create_from_parameters(const char * p) const
{
    if (*p != '\0')
	throw InvalidArgumentError("No parameters are required for\
		DiceCoeffWeight");
    return new Xapian::DiceCoeffWeight;
}

}
