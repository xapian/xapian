/** @file dphweight.cc
 * @brief Xapian::DPHWeight class - The DPH weighting scheme of the DFR framework.
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

DPHWeight *
DPHWeight::clone() const
{
    return new DPHWeight();
}

void
DPHWeight::init(double factor_)
{
    factor = factor_;

    double F(get_collection_freq());
    double N(get_collection_size());
    double wdf_lower(1.0);
    double wdf_upper(get_wdf_upper_bound());

    double len_upper(get_doclength_upper_bound());
    double len_lower(get_doclength_lower_bound());

    double min_wdf_to_len = wdf_lower / len_upper;
    double max_normalization = pow((1.0 - min_wdf_to_len), 2) / (wdf_lower + 1.0);
    double min_normalization = pow(1.0 / len_upper, 2) / (wdf_upper + 1.0);

    /* Cacluate lower bound on the weight in order to deal with negative
     * weights. */
    double min_weight = min_normalization *
                        (wdf_lower *
                        log2((wdf_lower * get_average_length() / len_upper) *
                        (N / F)) +
                        (0.5 * log2(2.0 * M_PI * wdf_lower / len_upper)));

    lower_bound = get_wqf() * min_weight;

    // Calcuate the upper bound on the weight.
    if (wdf_upper == 0) {
        upper_bound = 0.0;
        return;
    }

    double max_weight = max_normalization *
                        (wdf_upper *
                        log2((wdf_upper * get_average_length() / len_lower) *
                        (N / F)) +
                        (0.5 * log2(2 * M_PI * wdf_upper * (1 - min_wdf_to_len))));

    upper_bound = (get_wqf() * max_weight) - lower_bound;
}

string
DPHWeight::name() const
{
    return "Xapian::DPHWeight";
}

string
DPHWeight::serialise() const
{
    return string();
}

DPHWeight *
DPHWeight::unserialise(const string &) const
{
    return new DPHWeight();
}

double
DPHWeight::get_sumpart(Xapian::termcount wdf, Xapian::termcount len) const
{
    if (wdf == 0) return 0.0;

    double F(get_collection_freq());
    double N(get_collection_size());
    double wdf_to_len = double(wdf) / len;

    double normalization = pow((1 - wdf_to_len), 2) / (wdf + 1);

    double wt = normalization *
		(wdf *
		log2((wdf * get_average_length() / len) *
		(N / F)) +
		(0.5 * log2(2 * M_PI * wdf * (1 - wdf_to_len))));

    // Subtract the lower bound from the actual weight to avoid negative weights.
    return ((get_wqf() * wt) - lower_bound) * factor;
}

double
DPHWeight::get_maxpart() const
{
    return upper_bound * factor;
}

double
DPHWeight::get_sumextra(Xapian::termcount) const
{
    return 0;
}

double
DPHWeight::get_maxextra() const
{
    return 0;
}

}
