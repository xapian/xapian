/** @file dlhweight.cc
 * @brief Xapian::DLHWeight class - The DLH weighting scheme of the DFR framework.
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

using namespace std;

namespace Xapian {

DLHWeight *
DLHWeight::clone() const
{
    return new DLHWeight();
}

void
DLHWeight::init(double)
{
     // None Required
}

string
DLHWeight::name() const
{
    return "Xapian::DLHWeight";
}

string
DLHWeight::serialise() const
{
    return string();
}

DLHWeight *
DLHWeight::unserialise(const string & ) const
{
    return new DLHWeight();
}

double
DLHWeight::get_sumpart(Xapian::termcount wdf, Xapian::termcount len) const
{
    if (wdf == 0) return 0.0;

    double wdf_to_len = double(wdf) / len;

    double N(get_collection_size());
    double F(get_collection_freq());

    double weight = (wdf * log2((wdf * get_average_length() / len) * (N / F)) +
    (len - wdf) * log2(1.0 - wdf_to_len) + 0.5 * log2(2.0 * 3.14 * wdf * (1.0 - wdf_to_len))) /
    (wdf + 0.5);

    return (get_wqf() * weight);
}

double
DLHWeight::get_maxpart() const
{
    if (get_wdf_upper_bound() == 0) return 0.0;

    double wdf_lower(1.0);
    double wdf_upper(get_wdf_upper_bound());

    double min_wdf_to_len = (wdf_lower / get_doclength_upper_bound());

    double N(get_collection_size());
    double F(get_collection_freq());

    double max_weight = (wdf_upper * log2((wdf_upper * get_average_length() /
    get_doclength_lower_bound()) * (N / F)) + (get_doclength_upper_bound() -
    wdf_lower) * log2(1.0 - min_wdf_to_len) + 0.5 * log2(2.0 * 3.14 * wdf_upper *
    (1.0 - min_wdf_to_len))) / (wdf_upper + 0.5);

    return (get_wqf() * max_weight);
}

double
DLHWeight::get_sumextra(Xapian::termcount) const
{
    return 0;
}

double
DLHWeight::get_maxextra() const
{
    return 0;
}

}
