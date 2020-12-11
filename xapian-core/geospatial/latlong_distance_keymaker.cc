/** @file
 * @brief LatLongDistanceKeyMaker implementation.
 */
/* Copyright 2008 Lemur Consulting Ltd
 * Copyright 2011 Richard Boulton
 * Copyright 2012 Olly Betts
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
#include "xapian/queryparser.h" // For sortable_serialise.

using namespace Xapian;
using namespace std;

string
LatLongDistanceKeyMaker::operator()(const Document &doc) const
{
    string val(doc.get_value(slot));
    if (val.empty()) {
	return defkey;
    }
    LatLongCoords doccoords;
    doccoords.unserialise(val);
    double distance = (*metric)(centre, doccoords);
    return sortable_serialise(distance);
}

LatLongDistanceKeyMaker::~LatLongDistanceKeyMaker()
{
    delete metric;
}
