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
    return pow(dist + k1, -k2);
}

void
LatLongDistancePostingSource::calc_distance()
{
    std::string val(*it);
    LatLongCoords coords = LatLongCoords::unserialise(val);
    dist = metric(centre, coords);
}

LatLongDistancePostingSource::LatLongDistancePostingSource(
	Xapian::Database db_,
	Xapian::valueno valno_,
	const LatLongCoords & centre_,
	const LatLongMetric & metric_,
	double max_range_,
	double k1_,
	double k2_)
	: db(db_),
	  valno(valno_),
	  // "it" is not initialised until we start
	  // "end" is not initialised until we start
	  started(false),
	  // "dist" is not initialised until we start
	  // "max_weight" is initialised in body of constructor
	  // "termfreq_min" is initialised in body of constructor
	  // "termfreq_est" is initialised in body of constructor
	  // "termfreq_max" is initialised in body of constructor
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

    try { 
	termfreq_max = db.get_value_freq(valno);
	if (max_range == 0) {
	    termfreq_min = termfreq_max;
	    termfreq_est = termfreq_max;
	} else {
	    termfreq_min = 0;
	    termfreq_est = termfreq_max / 2;
	}
    } catch (const Xapian::UnimplementedError &) {
	termfreq_max = db.get_doccount();
	termfreq_est = termfreq_max / 2;
	termfreq_min = 0;
    }
}

Xapian::doccount
LatLongDistancePostingSource::get_termfreq_min() const
{
    return termfreq_min;
}

Xapian::doccount
LatLongDistancePostingSource::get_termfreq_est() const
{
    return termfreq_est;
}

Xapian::doccount
LatLongDistancePostingSource::get_termfreq_max() const
{
    return termfreq_max;
}

Xapian::weight
LatLongDistancePostingSource::get_maxweight() const
{
    return max_weight;
}

Xapian::weight
LatLongDistancePostingSource::get_weight() const
{
    return weight_from_distance(dist, k1, k2);
}

void
LatLongDistancePostingSource::next(Xapian::weight)
{
    do {
	if (!started) {
	    started = true;
	    it = db.valuestream_begin(valno);
	    end = db.valuestream_end(valno);
	} else {
	    ++it;
	}
	if (it == end) break;

	calc_distance();
	if (max_range > 0 && dist > max_range)
	    continue;
    } while(false);
}

void
LatLongDistancePostingSource::skip_to(Xapian::docid min_docid, Xapian::weight)
{
    if (!started) {
	started = true;
	it = db.valuestream_begin(valno);
	end = db.valuestream_end(valno);
    }

    it.skip_to(min_docid);

    while (it != end) {
	calc_distance();
	if (max_range == 0 || dist <= max_range)
	    break;
	++it;
    }
}

bool
LatLongDistancePostingSource::check(Xapian::docid min_docid, Xapian::weight)
{
    if (!started) {
	started = true;
	it = db.valuestream_begin(valno);
	end = db.valuestream_end(valno);
    }

    if (!it.check(min_docid)) {
	// check returned false, so we know the document is not in the source.
	return false;
    }
    if (it == end) {
	// return true, since we're definitely at the end of the list.
	return true;
    }

    calc_distance();
    if (max_range > 0 && dist > max_range) {
	return false;
    }
    return true;
}

bool
LatLongDistancePostingSource::at_end() const
{
    return it == end;
}

Xapian::docid
LatLongDistancePostingSource::get_docid() const
{
    return it.get_docid();
}

void
LatLongDistancePostingSource::reset()
{
    started = false;
}

std::string
LatLongDistancePostingSource::get_description() const
{
    return "Xapian::LatLongDistancePostingSource(slot=" + om_tostring(valno) + ")";
}

}
