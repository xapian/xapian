/** @file
 * @brief LatLongDistancePostingSource implementation.
 */
/* Copyright 2008 Lemur Consulting Ltd
 * Copyright 2010,2011 Richard Boulton
 * Copyright 2012,2015 Olly Betts
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

#include "xapian/error.h"
#include "xapian/registry.h"

#include "net/length.h"
#include "serialise-double.h"
#include "str.h"

#include <cmath>

using namespace Xapian;
using namespace std;

static double
weight_from_distance(double dist, double k1, double k2)
{
    // k2 defaults to 1.0, so handle that case with a fast path which avoids
    // calling pow().
    if (k2 == 1.0)
	return k1 / (dist + k1);
    return k1 * pow(dist + k1, -k2);
}

void
LatLongDistancePostingSource::calc_distance()
{
    dist = (*metric)(centre, get_value());
}

/// Validate the parameters supplied to LatLongDistancePostingSource.
static void
validate_postingsource_params(double k1, double k2) {
    if (k1 <= 0) {
	string msg("k1 parameter to LatLongDistancePostingSource must be "
		   "greater than 0; was ");
	msg += str(k1);
	throw InvalidArgumentError(msg);
    }
    if (k2 <= 0) {
	string msg("k2 parameter to LatLongDistancePostingSource must be "
		   "greater than 0; was ");
	msg += str(k2);
	throw InvalidArgumentError(msg);
    }
}

LatLongDistancePostingSource::LatLongDistancePostingSource(
	valueno slot_,
	const LatLongCoords & centre_,
	const LatLongMetric * metric_,
	double max_range_,
	double k1_,
	double k2_)
	: ValuePostingSource(slot_),
	  centre(centre_),
	  metric(metric_),
	  max_range(max_range_),
	  k1(k1_),
	  k2(k2_)
{
    validate_postingsource_params(k1, k2);
    set_maxweight(weight_from_distance(0, k1, k2));
}

LatLongDistancePostingSource::LatLongDistancePostingSource(
	valueno slot_,
	const LatLongCoords & centre_,
	const LatLongMetric & metric_,
	double max_range_,
	double k1_,
	double k2_)
	: ValuePostingSource(slot_),
	  centre(centre_),
	  metric(metric_.clone()),
	  max_range(max_range_),
	  k1(k1_),
	  k2(k2_)
{
    validate_postingsource_params(k1, k2);
    set_maxweight(weight_from_distance(0, k1, k2));
}

LatLongDistancePostingSource::LatLongDistancePostingSource(
	valueno slot_,
	const LatLongCoords & centre_,
	double max_range_,
	double k1_,
	double k2_)
	: ValuePostingSource(slot_),
	  centre(centre_),
	  metric(new Xapian::GreatCircleMetric()),
	  max_range(max_range_),
	  k1(k1_),
	  k2(k2_)
{
    validate_postingsource_params(k1, k2);
    set_maxweight(weight_from_distance(0, k1, k2));
}

LatLongDistancePostingSource::~LatLongDistancePostingSource()
{
    delete metric;
}

void
LatLongDistancePostingSource::next(double min_wt)
{
    ValuePostingSource::next(min_wt);

    while (!ValuePostingSource::at_end()) {
	calc_distance();
	if (max_range == 0 || dist <= max_range)
	    break;
	ValuePostingSource::next(min_wt);
    }
}

void
LatLongDistancePostingSource::skip_to(docid min_docid,
				      double min_wt)
{
    ValuePostingSource::skip_to(min_docid, min_wt);

    while (!ValuePostingSource::at_end()) {
	calc_distance();
	if (max_range == 0 || dist <= max_range)
	    break;
	ValuePostingSource::next(min_wt);
    }
}

bool
LatLongDistancePostingSource::check(docid min_docid,
				    double min_wt)
{
    if (!ValuePostingSource::check(min_docid, min_wt)) {
	// check returned false, so we know the document is not in the source.
	return false;
    }
    if (ValuePostingSource::at_end()) {
	// return true, since we're definitely at the end of the list.
	return true;
    }

    calc_distance();
    if (max_range > 0 && dist > max_range) {
	return false;
    }
    return true;
}

double
LatLongDistancePostingSource::get_weight() const
{
    return weight_from_distance(dist, k1, k2);
}

LatLongDistancePostingSource *
LatLongDistancePostingSource::clone() const
{
    return new LatLongDistancePostingSource(get_slot(), centre,
					    metric->clone(),
					    max_range, k1, k2);
}

string
LatLongDistancePostingSource::name() const
{
    return "Xapian::LatLongDistancePostingSource";
}

string
LatLongDistancePostingSource::serialise() const
{
    string serialised_centre = centre.serialise();
    string metric_name = metric->name();
    string serialised_metric = metric->serialise();

    string result = encode_length(get_slot());
    result += encode_length(serialised_centre.size());
    result += serialised_centre;
    result += encode_length(metric_name.size());
    result += metric_name;
    result += encode_length(serialised_metric.size());
    result += serialised_metric;
    result += serialise_double(max_range);
    result += serialise_double(k1);
    result += serialise_double(k2);
    return result;
}

LatLongDistancePostingSource *
LatLongDistancePostingSource::unserialise_with_registry(const string &s,
					     const Registry & registry) const
{
    const char * p = s.data();
    const char * end = p + s.size();

    valueno new_slot;
    decode_length(&p, end, new_slot);
    size_t len;
    decode_length_and_check(&p, end, len);
    string new_serialised_centre(p, len);
    p += len;
    decode_length_and_check(&p, end, len);
    string new_metric_name(p, len);
    p += len;
    decode_length_and_check(&p, end, len);
    string new_serialised_metric(p, len);
    p += len;
    double new_max_range = unserialise_double(&p, end);
    double new_k1 = unserialise_double(&p, end);
    double new_k2 = unserialise_double(&p, end);
    if (p != end) {
	throw NetworkError("Bad serialised LatLongDistancePostingSource - junk at end");
    }

    LatLongCoords new_centre;
    new_centre.unserialise(new_serialised_centre);

    const Xapian::LatLongMetric * metric_type =
	    registry.get_lat_long_metric(new_metric_name);
    if (metric_type == NULL) {
	string msg("LatLongMetric ");
	msg += new_metric_name;
	msg += " not registered";
	throw InvalidArgumentError(msg);
    }
    LatLongMetric * new_metric =
	    metric_type->unserialise(new_serialised_metric);

    return new LatLongDistancePostingSource(new_slot, new_centre,
					    new_metric,
					    new_max_range, new_k1, new_k2);
}

void
LatLongDistancePostingSource::init(const Database & db_)
{
    ValuePostingSource::init(db_);
    if (max_range > 0.0) {
	// Possible that no documents are in range.
	set_termfreq_min(0);
	// Note - would be good to improve termfreq_est here, too, but
	// I can't think of anything we can do with the information
	// available.
    }
}

string
LatLongDistancePostingSource::get_description() const
{
    string result("Xapian::LatLongDistancePostingSource(slot=");
    result += str(get_slot());
    result += ")";
    return result;
}
