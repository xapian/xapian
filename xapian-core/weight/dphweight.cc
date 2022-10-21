/** @file
 * @brief Xapian::DPHWeight class - The DPH weighting scheme of the DFR framework.
 */
/* Copyright (C) 2013, 2014 Aarsh Shah
 * Copyright (C) 2016,2017 Olly Betts
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

#include "xapian/error.h"
#include "common/log2.h"
#include <algorithm>
#include <cmath>

using namespace std;

namespace Xapian {

DPHWeight *
DPHWeight::clone() const
{
    return new DPHWeight();
}

void
DPHWeight::init(double factor)
{
    // Avoid warnings about unused private member.
    (void)lower_bound;

    if (factor == 0.0) {
	// This object is for the term-independent contribution, and that's
	// always zero for this scheme.
	return;
    }

    double F = get_collection_freq();
    double wdf_lower = 1.0;
    double wdf_upper = get_wdf_upper_bound();

    double len_upper = get_doclength_upper_bound();

    if (wdf_upper == 0) {
	upper_bound = 0.0;
	return;
    }

    double min_wdf_to_len = wdf_lower / len_upper;

    /* Calculate constant value to be used in get_sumpart(). */
    log_constant = log2(get_total_length() / F);
    wqf_product_factor = get_wqf() * factor;

    // Calculate the upper bound on the weight.

    /* Calculations to decide the values to be used for calculating upper bound. */
    /* The upper bound of the term appearing in the second log is obtained
       by taking the minimum and maximum wdf value in the formula as shown. */
    double max_product_1 = wdf_upper * (1.0 - min_wdf_to_len);
    /* A second upper bound of the term can be obtained by plugging in the
       upper bound of the length and differentiating the term w.r.t wdf
       to find the value of wdf at which function attains maximum value. */
    double wdf_var = min(wdf_upper, len_upper / 2.0);
    double max_product_2 = wdf_var * (1.0 - wdf_var / len_upper);
    /* Take the minimum of the two upper bounds. */
    double max_product = min(max_product_1, max_product_2);

    // Maximization of the product of wdf and normalized wdf.
    /* The expression is (wdf * (1.0 - wdf / len) * (1.0 - wdf / len)) /
			 (wdf + 1.0). */
    /* Now, assuming len to be len_upper for the purpose of maximization,
       (d)/(dx) (x * (1 - x / c) * (1 - x / c)) / (x+1) =
       ((c - x) * (c - x * (2 * x + 3))) / (c² * (x + 1)²)
       Thus, if (c - x * (2 * x + 3)) is positive, the differentiation
       value will be positive and hence the function will be an
       increasing function. By finding the positive root of the equation
       2 * x² + 3 * x - c = 0, we get the value of x(wdf)
       at which the differentiation value turns to negative from positive,
       and hence, the function will have maximum value for that value of wdf. */
    double wdf_root = 0.25 * (sqrt(8.0 * len_upper + 9.0) - 3.0);

    // If wdf_root outside valid range, use nearest value in range.
    if (wdf_root > wdf_upper) {
	wdf_root = wdf_upper;
    } else if (wdf_root < wdf_lower) {
	wdf_root = wdf_lower;
    }

    double x = 1 - wdf_root / len_upper;
    double x_squared = x * x;
    auto max_wdf_product_normalization = wdf_root / (wdf_root + 1) * x_squared;

    double max_weight = max_wdf_product_normalization *
	(log_constant + (0.5 * log2(2 * M_PI * max_product)));

    upper_bound = wqf_product_factor * max_weight;
    if (rare(upper_bound < 0.0)) upper_bound = 0.0;
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
DPHWeight::unserialise(const string& s) const
{
    if (rare(!s.empty()))
	throw Xapian::SerialisationError("Extra data in DPHWeight::unserialise()");
    return new DPHWeight();
}

double
DPHWeight::get_sumpart(Xapian::termcount wdf, Xapian::termcount len,
		       Xapian::termcount) const
{
    if (wdf == 0 || wdf == len) return 0.0;

    double wdf_to_len = double(wdf) / len;

    double x = 1 - wdf_to_len;
    double normalization = x * x / (wdf + 1);

    double wt = normalization *
	(wdf * (log2(wdf_to_len) + log_constant) +
	 (0.5 * log2(2 * M_PI * wdf * (1 - wdf_to_len))));
    if (rare(wt <= 0.0)) return 0.0;

    return wqf_product_factor * wt;
}

double
DPHWeight::get_maxpart() const
{
    return upper_bound;
}

double
DPHWeight::get_sumextra(Xapian::termcount, Xapian::termcount) const
{
    return 0;
}

double
DPHWeight::get_maxextra() const
{
    return 0;
}

}
