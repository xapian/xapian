/** @file bb2weight.cc
 * @brief Xapian::BB2Weight class - the BB2 weighting scheme of the DFR framework.
 */
/* Copyright (C) 2013,2014 Aarsh Shah
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
    double wdfn_upper(get_wdf_upper_bound());

    if (wdfn_upper == 0) {
	upper_bound = 0.0;
	return;
    }

    double base_change = log(2.0);
    double wdfn_lower(1.0);

    double F(get_collection_freq());
    double N(get_collection_size());

    wdfn_lower *= log2(1 + (param_c * get_average_length()) /
		    get_doclength_upper_bound());

    wdfn_upper *= log2(1 + (param_c * get_average_length()) /
		    get_doclength_lower_bound());

    /* Calculate constant values to be used in get_sumpart(). */
    c_product_avlen = param_c * get_average_length();
    B_constant = get_wqf() * factor * (F + 1.0) / get_termfreq();
    // This weighting scheme causes an error if N = 1 as that will cause
    // N - 1.0 to be equal to zero which is used inside a logarithm.
    wt = - log2(N - 1.0) - (1 / base_change);
    stirling_constant_1 = log2(N + F - 1.0);
    stirling_constant_2 = log2(F);

    double B_max = B_constant / (wdfn_lower + 1.0);

    // Maximize the Stirling value to be used in the upper bound.
    // Calculate the individual terms keeping the maximization of Stirling value
    // in mind.
    double y_max = N + F - wdfn_lower - 2.0;
    double y_min = F - wdfn_upper;
    // If the terms result in a negative value, make them positive as negative
    // values can not be used in a logarithm.
    if (y_min < 0)
	y_min = F;
    if (y_max < 0)
	y_max = N + F;

    double stirling_max_term1 = ((y_max + 0.5) *
				(stirling_constant_1 - log2(y_max)) +
				((wdfn_upper + 1.0) * stirling_constant_1));

    double stirling_min_term2 = ((y_min + 0.5) *
				(stirling_constant_2 - log2(y_min)) +
				((wdfn_lower) * stirling_constant_2));

    double stirling_max = stirling_max_term1 - stirling_min_term2;

    double final_weight_max = B_max * (wt + stirling_max);

    upper_bound = get_wqf() * final_weight_max * factor;
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
BB2Weight::get_sumpart(Xapian::termcount wdf, Xapian::termcount len) const
{
    if (wdf == 0) return 0.0;

    double wdfn(wdf);
    wdfn *= log2(1 + c_product_avlen / len);

    double F(get_collection_freq());
    double N(get_collection_size());

    double B = B_constant / (wdfn + 1.0);

    // This weighting scheme will cause an error if for any value of wdfn,
    // N + F - wdfn - 2.0 is equal to or less than zero as it is used inside
    // a logarithm in the stirling_value function.
    double stirling = stirling_value(wdfn + 1.0, N + F - wdfn - 2.0, stirling_constant_1) -
		      stirling_value(wdfn, F - wdfn, stirling_constant_2);

    double final_weight = B * (wt + stirling);

    return final_weight;
}

double
BB2Weight::get_maxpart() const
{
    return upper_bound;
}

double
BB2Weight::get_sumextra(Xapian::termcount) const
{
    return 0;
}

double
BB2Weight::get_maxextra() const
{
    return 0;
}

}
