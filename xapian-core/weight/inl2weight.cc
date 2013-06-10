/** @file InL2weight.cc
 * @brief Xapian::InL2Weight class - the InL2 weighting scheme of the DFR framework.
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

InL2Weight *
InL2Weight::clone() const
{
    return new InL2Weight(param_c);
}

void
InL2Weight::init(double)
{
     // None Required
}

string
InL2Weight::name() const
{
    return "Xapian::InL2Weight";
}

string
InL2Weight::serialise() const
{
    return serialise_double(param_c);
}

InL2Weight *
InL2Weight::unserialise(const string & s) const
{
    const char *ptr = s.data();
    const char *end = ptr + s.size();
    double c = unserialise_double(&ptr, end);
    if (rare(ptr != end))
	throw Xapian::SerialisationError("Extra data in InL2Weight::unserialise()");
    return new InL2Weight(c);
}

double
InL2Weight::get_sumpart(Xapian::termcount wdf, Xapian::termcount len) const
{
    if (wdf == 0) return 0.0;
    double wdfn(wdf);
    double base_change(log(2));
    double N(get_collection_size());
    double TermFrequency(get_termfreq());

    wdfn = wdfn * ((log(1 + (param_c * get_average_length() / len))) / (base_change));
    double L = 1 / (wdfn + 1);

    double Idf_value = log((N + 1) / (TermFrequency + 0.5)) / (base_change);

    return (get_wqf() * wdfn * L * Idf_value);
}

double
InL2Weight::get_maxpart() const
{
    if (get_wdf_upper_bound() == 0) return 0.0;
    double wdfn_lower(1.0);
    double wdfn_upper(get_wdf_upper_bound());
    double base_change(log(2));
    double TermFrequency(get_termfreq());
    double N(get_collection_size());

    wdfn_lower = wdfn_lower * ((log(1 + param_c * get_average_length() / get_doclength_upper_bound())) / (base_change));
    wdfn_upper = wdfn_upper * ((log(1 + param_c * get_average_length() / get_doclength_lower_bound())) / (base_change));

    double L_max = 1 / (wdfn_lower + 1);

    double Idf_value_max = log((N + 1) / (TermFrequency + 0.5)) / (base_change);

    return (get_wqf() * wdfn_upper * L_max * Idf_value_max);
}

double
InL2Weight::get_sumextra(Xapian::termcount) const
{
    return 0;
}

double
InL2Weight::get_maxextra() const
{
    return 0;
}

}
