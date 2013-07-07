/** @file bb2weight.cc
 * @brief Xapian::BB2Weight class - the BB2 weighting scheme of the DFR framework.
 */
/* Copyright (C) 2013 Aarsh Shah
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

double stirling_value(double, double);

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
BB2Weight::init(double)
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

    double B_max = (F + 1.0) / (get_termfreq() * (wdfn_lower + 1.0));

    double weight_max = - log2(N - 1.0) - (1 / base_change);

    double stirling_max = stirling_value(N + F - 1.0, N + F - wdfn_lower - 2.0) -
                          stirling_value(F, F - wdfn_upper);

    double final_weight_max = B_max * (weight_max + stirling_max);

    upper_bound = get_wqf() * final_weight_max;
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

    double base_change = log(2.0);
    double wdfn(wdf);
    wdfn *= log2(1 + (param_c * get_average_length()) / len);

    double F(get_collection_freq());
    double N(get_collection_size());

    double B = (F + 1.0) / (get_termfreq() * (wdfn + 1.0));

    double weight = - log2(N - 1.0) - (1 / base_change);

    double stirling = stirling_value(N + F - 1.0, N + F - wdfn - 2.0) -
                      stirling_value(F, F - wdfn);

    double final_weight = B * (weight + stirling);

    return (get_wqf() * final_weight);
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

double stirling_value(double x, double y)
{
    double difference = x - y;
    return ((y + 0.5) * log2(x / y)) + (difference * log2(x));
}

}
