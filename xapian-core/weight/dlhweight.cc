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
#include "common/log2.h"

using namespace std;

namespace Xapian {

DLHWeight *
DLHWeight::clone() const
{
    return new DLHWeight();
}

void
DLHWeight::init(double factor)
{
    factor_ = factor;

    double wdf_lower(1.0);
    double wdf_upper(get_wdf_upper_bound());
    double len_lower(get_doclength_lower_bound());
    double len_upper(get_doclength_upper_bound());

    double min_wdf_to_len = wdf_lower / len_upper;

    double N(get_collection_size());
    double F(get_collection_freq());

    // Calculate the lower bound.
    double min_weight = (wdf_lower * log2((wdf_lower * get_average_length() /
			len_upper) * (N / F)) -
			(1.5 * log2(len_upper)) +
			0.5 * log2(2.0 * M_PI * wdf_lower)) /
			(wdf_upper + 0.5);

    lower_bound = get_wqf() * min_weight;

    // Calculate the upper bound.
    if (wdf_upper == 0) {
	upper_bound = 0.0;
	return;
    }

    double max_weight = (wdf_upper * log2((wdf_upper * get_average_length() /
			len_lower) * (N / F)) + (len_upper - wdf_lower) *
			log2(1.0 - min_wdf_to_len) +
			0.5 * log2(2.0 * M_PI * wdf_upper *
			(1.0 - min_wdf_to_len))) / (wdf_lower + 0.5);

    upper_bound = (get_wqf() * max_weight) - lower_bound;
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

    double wt = (wdf * log2((wdf * get_average_length() / len) * (N / F)) +
		(len - wdf) * log2(1.0 - wdf_to_len) +
		0.5 * log2(2.0 * M_PI * wdf * (1.0 - wdf_to_len))) /
		(wdf + 0.5);

    return ((get_wqf() * wt) - lower_bound) * factor_;
}

double
DLHWeight::get_maxpart() const
{
    return upper_bound * factor_;
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
