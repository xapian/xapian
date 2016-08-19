/** @file pl2plusweight.cc
 * @brief Xapian::PL2PlusWeight class - the PL2+ weighting scheme of the DFR framework.
 */
/* Copyright (C) 2013 Aarsh Shah
 * Copyright (C) 2013,2014,2016 Olly Betts
 * Copyright (C) 2016 Vivek Pal
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

PL2PlusWeight::PL2PlusWeight(double c, double delta)
	: param_c(c), param_delta(delta)
{
    if (param_c <= 0)
	throw Xapian::InvalidArgumentError("Parameter c is invalid.");
    if (param_delta <= 0)
	throw Xapian::InvalidArgumentError("Parameter delta is invalid.");
    need_stat(AVERAGE_LENGTH);
    need_stat(DOC_LENGTH);
    need_stat(DOC_LENGTH_MIN);
    need_stat(DOC_LENGTH_MAX);
    need_stat(COLLECTION_SIZE);
    need_stat(COLLECTION_FREQ);
    need_stat(WDF);
    need_stat(WDF_MAX);
    need_stat(WQF);
}

PL2PlusWeight *
PL2PlusWeight::clone() const
{
    return new PL2PlusWeight(param_c, param_delta);
}

void
PL2PlusWeight::init(double factor_)
{
    factor = factor_;

    if (get_wdf_upper_bound() == 0) {
	// The "extra" weight object is cloned, init() called and then
	// get_maxextra() is called and we discover that we don't need it.
	// So we need to handle that case (which will give us 0 from
	// get_wdf_upper_bound() here).
	upper_bound = 0;
	return;
    }

    factor *= get_wqf();

    cl = param_c * get_average_length();

    double base_change(1.0 / log(2.0));
    mean = double(get_collection_freq()) / get_collection_size();
    P1 = mean * base_change + 0.5 * log2(2.0 * M_PI);
    P2 = log2(mean) + base_change;

    double wdfn_lower = log2(1 + cl / get_doclength_upper_bound());
    double divisior = max(get_wdf_upper_bound(), get_doclength_lower_bound());
    double wdfn_upper = get_wdf_upper_bound() * log2(1 + cl / divisior);

    double P_delta = P1 + (param_delta + 0.5) * log2(param_delta) - P2 * param_delta;
    dw = P_delta / (param_delta + 1.0);

    // Calculate an upper bound on the weights which get_sumpart() can return.
    //
    // We consider the equation for P as the sum of two parts which we
    // maximise individually:
    //
    // (a) (wdfn + 0.5) / (wdfn + 1) * log2(wdfn)
    // (b) (P1 - P2 * wdfn) / (wdfn + 1)
    //
    // To maximise (a), the fractional part is always positive (since wdfn>0)
    // and is maximised by maximising wdfn - clearer when rewritten as:
    // (1 - 0.5 / (wdfn + 1))
    //
    // The log part of (a) is clearly also maximised by maximising wdfn,
    // so we want to evaluate (a) at wdfn=wdfn_upper.
    double P_max2a = (wdfn_upper + 0.5) * log2(wdfn_upper) / (wdfn_upper + 1.0);
    // To maximise (b) substitute x=wdfn+1 (so x>1) and we get:
    //
    // (P1 + P2)/x - P2
    //
    // Differentiating wrt x gives:
    //
    // -(P1 + P2)/x²
    //
    // So there are no local minima or maxima, and the function is continuous
    // in the range of interest, so the sign of this differential tells us
    // whether we want to maximise or minimise wdfn, and since x>1, we can
    // just consider the sign of: (P1 + P2)
    //
    // Commonly P1 + P2 > 0, in which case we evaluate P at wdfn=wdfn_upper
    // giving us a bound that can't be bettered if wdfn_upper is tight.
    double wdfn_optb = P1 + P2 > 0 ? wdfn_upper : wdfn_lower;
    double P_max2b = (P1 - P2 * wdfn_optb) / (wdfn_optb + 1.0);
    upper_bound = factor * (P_max2a + P_max2b + dw);

    if (rare(upper_bound <= 0)) upper_bound = 0;
}

string
PL2PlusWeight::name() const
{
    return "Xapian::PL2PlusWeight";
}

string
PL2PlusWeight::serialise() const
{
    string result = serialise_double(param_c);
    result += serialise_double(param_delta);
    return result;
}

PL2PlusWeight *
PL2PlusWeight::unserialise(const string & s) const
{
    const char *ptr = s.data();
    const char *end = ptr + s.size();
    double c = unserialise_double(&ptr, end);
    double delta = unserialise_double(&ptr, end);
    if (rare(ptr != end))
	throw Xapian::SerialisationError("Extra data in PL2PlusWeight::unserialise()");
    return new PL2PlusWeight(c, delta);
}

double
PL2PlusWeight::get_sumpart(Xapian::termcount wdf, Xapian::termcount len,
			   Xapian::termcount) const
{
    if (wdf == 0 || mean < 1) return 0.0;

    double wdfn = wdf * log2(1 + cl / len);

    double P = P1 + (wdfn + 0.5) * log2(wdfn) - P2 * wdfn;

    double wt = (P / (wdfn + 1.0)) + dw;
    // FIXME: Is a negative wt possible here?  It is with vanilla PL2, but for
    // PL2+ we've added on dw, and bailed out early if mean < 1.
    if (rare(wt <= 0)) return 0.0;

    return factor * wt;
}

double
PL2PlusWeight::get_maxpart() const
{
    return upper_bound;
}

double
PL2PlusWeight::get_sumextra(Xapian::termcount, Xapian::termcount) const
{
    return 0;
}

double
PL2PlusWeight::get_maxextra() const
{
    return 0;
}

}
