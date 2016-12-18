/** @file bb2weight.cc
 * @brief Xapian::BB2Weight class - the BB2 weighting scheme of the DFR framework.
 */
/* Copyright (C) 2013,2014 Aarsh Shah
 * Copyright (C) 2014,2015,2016 Olly Betts
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
#include "common/log2.h"

#include "serialise-double.h"

#include "xapian/error.h"

using namespace std;

namespace Xapian {

static double stirling_value(double difference, double y, double stirling_constant)
{
    return ((y + 0.5) * (stirling_constant - log2(y)) + (difference * stirling_constant));
}

BB2Weight::BB2Weight(double c) : param_c(c)
{
    if (param_c <= 0)
	throw Xapian::InvalidArgumentError("Parameter c is invalid.");
    need_stat(AVERAGE_LENGTH);
    need_stat(DOC_LENGTH);
    need_stat(DOC_LENGTH_MIN);
    need_stat(DOC_LENGTH_MAX);
    need_stat(COLLECTION_SIZE);
    need_stat(COLLECTION_FREQ);
    need_stat(WDF);
    need_stat(WDF_MAX);
    need_stat(WQF);
    need_stat(TERMFREQ);
}

BB2Weight *
BB2Weight::clone() const
{
    return new BB2Weight(param_c);
}

void
BB2Weight::init(double factor)
{
    double wdfn_upper = get_wdf_upper_bound();

    if (wdfn_upper == 0) {
	upper_bound = 0.0;
	return;
    }

    c_product_avlen = param_c * get_average_length();
    double wdfn_lower(1.0);
    wdfn_lower *= log2(1 + c_product_avlen / get_doclength_upper_bound());
    wdfn_upper *= log2(1 + c_product_avlen / get_doclength_lower_bound());

    double F = get_collection_freq();

    // Clamp wdfn to at most (F - 1) to avoid ill-defined log calculations in
    // stirling_value().
    if (rare(wdfn_lower >= F - 1))
	wdfn_upper = F - 1;
    if (rare(wdfn_upper >= F - 1))
	wdfn_upper = F - 1;

    B_constant = get_wqf() * factor * (F + 1.0) / get_termfreq();

    // Clamp N to at least 2 to avoid ill-defined log calculations in
    // stirling_value().
    double N = rare(get_collection_size() <= 2) ? 2.0 : double(get_collection_size());

    wt = -1.0 / log(2.0) - log2(N - 1.0);
    stirling_constant_1 = log2(N + F - 1.0);
    stirling_constant_2 = log2(F);

    // Maximize the Stirling value to be used in the upper bound.
    // Calculate the individual terms keeping the maximization of Stirling value
    // in mind.
    double y_min = F - wdfn_upper;
    double y_max = N + F - wdfn_lower - 2.0;

    double stirling_max = stirling_value(wdfn_upper + 1.0, y_max,
					 stirling_constant_1) -
			  stirling_value(wdfn_lower, y_min,
					 stirling_constant_2);

    double B_max = B_constant / (wdfn_lower + 1.0);
    upper_bound = B_max * (wt + stirling_max);
    if (rare(upper_bound < 0.0))
	upper_bound = 0.0;
}

string
BB2Weight::name() const
{
    return "Xapian::BB2Weight";
}

string
BB2Weight::serialise() const
{
    return serialise_double(param_c);
}

BB2Weight *
BB2Weight::unserialise(const string & s) const
{
    const char *ptr = s.data();
    const char *end = ptr + s.size();
    double c = unserialise_double(&ptr, end);
    if (rare(ptr != end))
	throw Xapian::SerialisationError("Extra data in BB2Weight::unserialise()");
    return new BB2Weight(c);
}

double
BB2Weight::get_sumpart(Xapian::termcount wdf, Xapian::termcount len,
		       Xapian::termcount) const
{
    if (wdf == 0) return 0.0;

    double wdfn = wdf * log2(1 + c_product_avlen / len);

    double F = get_collection_freq();

    // Clamp wdfn to at most (F - 1) to avoid ill-defined log calculations in
    // stirling_value().
    if (rare(wdfn >= F - 1))
	wdfn = F - 1;

    // Clamp N to at least 2 to avoid ill-defined log calculations in
    // stirling_value().
    Xapian::doccount N = get_collection_size();
    Xapian::doccount N_less_2 = rare(N <= 2) ? 0 : N - 2;

    double y2 = F - wdfn;
    double y1 = N_less_2 + y2;
    double stirling = stirling_value(wdfn + 1.0, y1, stirling_constant_1) -
		      stirling_value(wdfn, y2, stirling_constant_2);

    double B = B_constant / (wdfn + 1.0);
    double final_weight = B * (wt + stirling);
    if (rare(final_weight < 0.0))
	final_weight = 0.0;
    return final_weight;
}

double
BB2Weight::get_maxpart() const
{
    return upper_bound;
}

double
BB2Weight::get_sumextra(Xapian::termcount, Xapian::termcount) const
{
    return 0;
}

double
BB2Weight::get_maxextra() const
{
    return 0;
}

}
