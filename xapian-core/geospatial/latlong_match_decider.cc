/** \file latlong_match_decider.cc
 * \brief LatLongRangeMatchDecider implementation.
 */
/* Copyright 2008 Lemur Consulting Ltd
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

#include "xapian/geospatial.h"
#include "xapian/document.h"

using namespace Xapian;

bool
LatLongRangeMatchDecider::operator()(const Document &doc) const
{
    std::string val(doc.get_value(valno));
    if (val.empty())
	return false;
    LatLongCoords coords = LatLongCoords::unserialise(val);
    return (metric(centre, coords) <= range);
}
