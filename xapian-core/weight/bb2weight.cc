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
#include <cmath>

#include "serialise-double.h"

#include "xapian/error.h"

using namespace std;

namespace Xapian {

BB2Weight *
BB2Weight::clone() const
{
    return new BB2Weight(param_c);
}

void
BB2Weight::init(double)
{
     // None Required
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

double BB2Weight::stirlingValue(double x, double y) const
{
    double base_change = log(2);
    double difference = x - y;
    double answer = (y + 0.5) * (log(x / y) / base_change) + (difference * (log(x) / base_change));
    return answer;
}

double
BB2Weight::get_sumpart(Xapian::termcount wdf, Xapian::termcount len) const
{
    if (wdf == 0) return 0.0;
    double wdfn(wdf);
    double base_change(log(2));
    wdfn = wdfn * ((log(1 + (param_c * get_average_length() / len))) / (base_change));

    double F(get_collection_freq());
    double N(get_collection_size());

    double B = (F + 1.0) / (get_termfreq() * (wdfn + 1.0));

    double weight = B * ( - (log(N - 1.0) / base_change) - (1 / base_change) +
    stirlingValue(N + F - 1.0, N + F - wdfn -2.0) - stirlingValue(F, F - wdfn));

    return (get_wqf() * weight);
}

double
BB2Weight::get_maxpart() const
{
    if (get_wdf_upper_bound() == 0) return 0.0;
    double wdfn_lower(1.0);
    double base_change(log(2));
    double wdfn_upper(get_wdf_upper_bound());
    double F(get_collection_freq());
    double N(get_collection_size());

    wdfn_lower = wdfn_lower * ((log(1 + param_c * get_average_length() / get_doclength_upper_bound())) / (base_change));

    wdfn_upper = wdfn_upper * ((log(1 + param_c * get_average_length() / get_doclength_lower_bound())) / (base_change));

    double B_max = (F + 1.0) / (get_termfreq() * (wdfn_lower + 1.0));

    double weight_max = B_max * ( - (log(N - 1.0) / base_change) - (1 / base_change) +
    stirlingValue(N + F - 1.0, N + F - wdfn_lower -2.0) - stirlingValue(F, F - wdfn_upper));

    return (get_wqf() * weight_max);
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
