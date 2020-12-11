/** @file
 * @brief Xapian::CoordWeight class - coordinate matching
 */
/* Copyright (C) 2004,2009,2011,2016 Olly Betts
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

CoordWeight *
CoordWeight::clone() const
{
    return new CoordWeight;
}

void
CoordWeight::init(double factor_)
{
    factor = factor_;
}

string
CoordWeight::name() const
{
    return "Xapian::CoordWeight";
}

string
CoordWeight::serialise() const
{
    // No parameters to serialise.
    return string();
}

CoordWeight *
CoordWeight::unserialise(const string& s) const
{
    if (rare(!s.empty()))
	throw Xapian::SerialisationError("Extra data in CoordWeight::unserialise()");
    return new CoordWeight;
}

double
CoordWeight::get_sumpart(Xapian::termcount, Xapian::termcount,
			 Xapian::termcount) const
{
    return factor;
}

double
CoordWeight::get_maxpart() const
{
    return factor;
}

double
CoordWeight::get_sumextra(Xapian::termcount, Xapian::termcount) const
{
    return 0;
}

double
CoordWeight::get_maxextra() const
{
    return 0;
}

}
