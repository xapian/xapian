/** \file latlong_posting_source.cc
 * \brief LatLongPostingSource implementation.
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
#include "xapian/error.h"

#include "utils.h"

#include <math.h>

namespace Xapian {

static double
weight_from_distance(double dist, double k1, double k2)
{
    return k1 * pow(dist + k1, -k2);
}

void
LatLongDistancePostingSource::calc_distance()
{
    std::string val(*value_it);
    LatLongCoords coords = LatLongCoords::unserialise(val);
    dist = metric(centre, coords);
}

LatLongDistancePostingSource::LatLongDistancePostingSource(
	Xapian::valueno slot_,
	const LatLongCoords & centre_,
	const LatLongMetric & metric_,
	double max_range_,
	double k1_,
	double k2_)
	: Xapian::ValuePostingSource(slot_),
	  centre(centre_),
	  metric(metric_),
	  max_range(max_range_),
	  k1(k1_),
	  k2(k2_)
{
    if (k1 <= 0)
	throw InvalidArgumentError(
	    "k1 parameter to LatLongDistancePostingSource must be greater "
	    "than 0; was " + om_tostring(k1));
    if (k2 <= 0)
	throw InvalidArgumentError(
	    "k2 parameter to LatLongDistancePostingSource must be greater "
	    "than 0; was " + om_tostring(k2));
    max_weight = weight_from_distance(0, k1, k2);
}

void
LatLongDistancePostingSource::next(Xapian::weight min_wt)
{
    ValuePostingSource::next(min_wt);

    while (value_it != value_end) {
	calc_distance();
	if (max_range == 0 || dist <= max_range)
	    break;
	++value_it;
    }
}

void
LatLongDistancePostingSource::skip_to(Xapian::docid min_docid,
				      Xapian::weight min_wt)
{
    ValuePostingSource::skip_to(min_docid, min_wt);

    while (value_it != value_end) {
	calc_distance();
	if (max_range == 0 || dist <= max_range)
	    break;
	++value_it;
    }
}

bool
LatLongDistancePostingSource::check(Xapian::docid min_docid,
				    Xapian::weight min_wt)
{
    if (!ValuePostingSource::check(min_docid, min_wt)) {
	// check returned false, so we know the document is not in the source.
	return false;
    }
    if (value_it == value_end) {
	// return true, since we're definitely at the end of the list.
	return true;
    }

    calc_distance();
    if (max_range > 0 && dist > max_range) {
	return false;
    }
    return true;
}

Xapian::weight
LatLongDistancePostingSource::get_weight() const
{
    return weight_from_distance(dist, k1, k2);
}

void
LatLongDistancePostingSource::reset(const Database & db_)
{
    ValuePostingSource::reset(db_);
    if (max_range > 0.0) {
	// Possible that no documents are in range.
	termfreq_min = 0;
	// Note - would be good to improve termfreq_est here, too, but
	// I can't think of anything we can do with the information
	// available.
    }
}

std::string
LatLongDistancePostingSource::get_description() const
{
    return "Xapian::LatLongDistancePostingSource(slot=" + om_tostring(slot) + ")";
}

}
