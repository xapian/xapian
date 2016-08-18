/** @file pl2plusweight.cc
 * @brief Xapian::PL2PlusWeight class - the PL2+ weighting scheme of the DFR framework.
 */
/* Copyright (C) 2013 Aarsh Shah
 * Copyright (C) 2013,2014 Olly Betts
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
PL2PlusWeight::init(double)
{
    if (get_wdf_upper_bound() == 0) {
	// The "extra" weight object is cloned, init() called and then
	// get_maxextra() is called and we discover that we don't need it.
	// So we need to handle that case (which will give us 0 from
	// get_wdf_upper_bound() here).
	lower_bound = upper_bound = 0;
	return;
    }

    cl = param_c * get_average_length();

    double base_change(1.0 / log(2.0));
    mean = double(get_collection_freq()) / get_collection_size();
    P1 = mean * base_change + 0.5 * log2(2.0 * M_PI);
    P2 = log2(mean) + base_change;

    double wdfn_lower = log2(1 + cl / get_doclength_upper_bound());
    double wdfn_upper =
	get_wdf_upper_bound() * log2(1 + cl / get_doclength_lower_bound());

    double P_delta = P1 + (param_delta + 0.5) * log2(param_delta) - P2 * param_delta;
    dw = P_delta / (param_delta + 1.0);

    // Calculate the lower bound on the weight.
    double P_min =
	P1 + (wdfn_lower + 0.5) * log2(wdfn_lower) - P2 * wdfn_lower;
    lower_bound = get_wqf() * ((P_min / (wdfn_upper + 1.0)) + dw);

    // Calculate the upper bound on the weight.
    double P_max =
	P1 + (wdfn_upper + 0.5) * log2(wdfn_upper) - P2 * wdfn_upper;
    upper_bound = get_wqf() * ((P_max / (wdfn_lower + 1.0)) + dw);

    upper_bound -= lower_bound;
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

    return get_wqf() * ((P / (wdfn + 1.0)) + dw) - lower_bound;
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
