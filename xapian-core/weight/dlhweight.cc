/** @file
 * @brief Xapian::DLHWeight class - The DLH weighting scheme of the DFR framework.
 */
/* Copyright (C) 2013, 2014 Aarsh Shah
 * Copyright (C) 2016,2017,2019 Olly Betts
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
    // Avoid warnings about unused private member.
    (void)lower_bound;

    if (factor == 0.0) {
	// This object is for the term-independent contribution, and that's
	// always zero for this scheme.
	return;
    }

    double wdf_upper = get_wdf_upper_bound();
    if (wdf_upper == 0) {
	upper_bound = 0.0;
	return;
    }

    const double wdf_lower = 1.0;
    double len_upper = get_doclength_upper_bound();
    double len_lower = get_doclength_lower_bound();

    double F = get_collection_freq();

    // Calculate constant values to be used in get_sumpart().
    log_constant = get_total_length() / F;
    wqf_product_factor = get_wqf() * factor;

    // Calculate values for the upper bound.

    // w <= l, so if the allowed ranges overlap, max w/l is 1.0.
    double max_wdf_over_l = wdf_upper < len_lower ? wdf_upper / len_lower : 1.0;

    // First term A: w/(w+.5)*log2(w/l*L) where L=total_len/coll_freq
    // Assume log >= 0:
    //   w/(w+.5) = 1-1/(2w+1) and w >= 1 so max A at w=w_max
    //   log2(w/l*L) maximised when w/l maximised
    //   so max A at w=w_max, l=max(l_min, w_max)
    // If log < 0 => then A <= 0, so max A <= 0 and we want to minimise the
    // factor outside the log.
    double logged_expr = max_wdf_over_l * log_constant;
    double w_for_A = logged_expr > 1.0 ? wdf_upper : wdf_lower;
    double A = w_for_A / (w_for_A + 0.5) * log2(logged_expr);

    // Second term B:
    //
    // (l-w)*log2(1-w/l)
    //
    // The log is negative, and w <= l so B <= 0 and its maximum is the value
    // as close to zero as possible.  So smaller (l-w) is better and smaller
    // w/l is better.
    //
    // This function is ill defined at w=l, but -> 0 as w -> l.
    //
    // If w=l is valid (i.e. len_lower > wdf_upper) then B = 0.
    double B = 0;
    if (len_lower > wdf_upper) {
	// If not, then minimising l-w gives us a candidate (i.e. w=wdf_upper
	// and l=len_lower).
	//
	// The function is also 0 at w = 0 (there must be a local mimina at
	// some value of w between 0 and l), so the other candidate is at
	// w=wdf_lower.
	//
	// We need to find the optimum value of l in this case, so
	// differentiate the formula by l:
	//
	// d/dl: log2(1-w/l) + (l-w)*(1-w/l)/(l*log(2))
	//     = (log(1-w/l) + (1-w/l)²)/log(2)
	//     = (log(x) + x²)/log(2)     [x=1-w/l]
	//
	// which is 0 at approx x=0.65291864
	//
	// x=1-w/l <=> l*(1-x)=w <=> l=w/(1-x) <=> l ~= 0.34708136*w
	//
	// but l >= w so we can't attain that (and the log isn't valid there).
	//
	// Gradient at (without loss of generality) l=2*w is:
	//       (log(0.5) + 0.25)/log(2)
	// which is < 0 so want to minimise l, i.e. l=len_lower, so the other
	// candidate is w=wdf_lower and l=len_lower.
	//
	// So evaluate both candidates and pick the larger:
	double B1 = (len_lower - wdf_lower) * log2(1.0 - wdf_lower / len_lower);
	double B2 = (len_lower - wdf_upper) * log2(1.0 - wdf_upper / len_lower);
	B = max(B1, B2);
    }

    /* An upper bound of the term used in the third log can be obtained by:
     *
     * 0.5 * log2(2.0 * M_PI * wdf * (1 - wdf / len))
     *
     * An upper bound on wdf * (1 - wdf / len) (and hence on the log, since
     * log is a monotonically increasing function on positive numbers) can
     * be obtained by plugging in the upper bound of the length and
     * differentiating the term w.r.t wdf which gives the value of wdf at which
     * the function attains maximum value - at wdf = len_upper / 2 (or if the
     * wdf can't be that large, at wdf = wdf_upper): */
    double wdf_var = min(wdf_upper, len_upper / 2.0);
    double max_product = wdf_var * (1.0 - wdf_var / len_upper);
#if 0
    /* An upper bound can also be obtained by taking the minimum and maximum
     * wdf value in the formula as shown (which isn't useful now as it's always
     * >= the bound above, but could be useful if we tracked bounds on wdf/len):
     */
    double min_wdf_to_len = wdf_lower / len_upper;
    double max_product_2 = wdf_upper * (1.0 - min_wdf_to_len);
    /* Take the minimum of the two upper bounds. */
    max_product = min(max_product, max_product_2);
#endif
    double C = 0.5 * log2(2.0 * M_PI * max_product) / (wdf_lower + 0.5);
    upper_bound = A + B + C;

    if (rare(upper_bound < 0.0))
	upper_bound = 0.0;
    else
	upper_bound *= wqf_product_factor;
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
DLHWeight::unserialise(const string& s) const
{
    if (rare(!s.empty()))
	throw Xapian::SerialisationError("Extra data in DLHWeight::unserialise()");
    return new DLHWeight();
}

double
DLHWeight::get_sumpart(Xapian::termcount wdf, Xapian::termcount len,
		       Xapian::termcount) const
{
    if (wdf == 0 || wdf == len) return 0.0;

    double wdf_to_len = double(wdf) / len;
    double one_minus_wdf_to_len = 1.0 - wdf_to_len;

    double wt = wdf * log2(wdf_to_len * log_constant) +
		(len - wdf) * log2(one_minus_wdf_to_len) +
		0.5 * log2(2.0 * M_PI * wdf * one_minus_wdf_to_len);
    if (rare(wt <= 0.0)) return 0.0;

    return wqf_product_factor * wt / (wdf + 0.5);
}

double
DLHWeight::get_maxpart() const
{
    return upper_bound;
}

double
DLHWeight::get_sumextra(Xapian::termcount, Xapian::termcount) const
{
    return 0;
}

double
DLHWeight::get_maxextra() const
{
    return 0;
}

}
