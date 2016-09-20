/** @file boolweight.cc
 * @brief Xapian::BoolWeight class - boolean weighting
 */
/* Copyright (C) 2009,2011,2016 Olly Betts
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

BoolWeight *
BoolWeight::clone() const
{
    return new BoolWeight;
}

void
BoolWeight::init(double)
{
    // Nothing to do here.
}

string
BoolWeight::name() const
{
    return "Xapian::BoolWeight";
}

string
BoolWeight::serialise() const
{
    // No parameters to serialise.
    return string();
}

BoolWeight *
BoolWeight::unserialise(const string& s) const
{
    if (rare(!s.empty()))
	throw Xapian::SerialisationError("Extra data in BoolWeight::unserialise()");
    return new BoolWeight;
}

double
BoolWeight::get_sumpart(Xapian::termcount, Xapian::termcount,
			Xapian::termcount) const
{
    return 0;
}

double
BoolWeight::get_maxpart() const
{
    return 0;
}

double
BoolWeight::get_sumextra(Xapian::termcount, Xapian::termcount) const
{
    return 0;
}

double
BoolWeight::get_maxextra() const
{
    return 0;
}

}
