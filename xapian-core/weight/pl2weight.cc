/** @file pl2weight.cc
 * @brief Xapian::PL2Weight class - the PL2 weighting scheme of the DFR framework.
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

PL2Weight::PL2Weight(double c) : param_c(c)
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
}

PL2Weight *
PL2Weight::clone() const
{
    return new PL2Weight(param_c);
}

void
PL2Weight::init(double)
{
    double wdfn_lower(1.0);
    double wdfn_upper(get_wdf_upper_bound());
    double base_change(log(2));
    double F(get_collection_freq());
    double N(get_collection_size());
    double mean(F / N);

    wdfn_lower *= log2(1 + (param_c * get_average_length()) /
		    get_doclength_upper_bound());

    wdfn_upper *= log2(1 + (param_c * get_average_length()) /
		    get_doclength_lower_bound());

    double L_min = 1 / (wdfn_upper + 1.0);
    double L_max = 1 / (wdfn_lower + 1.0);

    // Calculate the lower bound on the weight.
    double P_min = wdfn_lower * log2(1.0 / mean) +
		   mean / base_change +
		   0.5 * log2(2.0 * M_PI * wdfn_lower) +
		   wdfn_lower * (log2(wdfn_lower) - (1 / base_change));

    lower_bound = get_wqf() * L_min * P_min;

    // Calculate the upper bound on the weight.
    if (wdfn_upper == 0) {
	upper_bound = 0.0;
	return;
    }

    double P_max = wdfn_upper * log2(1.0 / mean) +
		   mean / base_change +
		   0.5 * log2(2.0 * M_PI * wdfn_upper) +
		   wdfn_upper * (log2(wdfn_upper) - (1 / base_change));

    upper_bound = (get_wqf() * L_max * P_max) - lower_bound;
}

string
PL2Weight::name() const
{
    return "Xapian::PL2Weight";
}

string
PL2Weight::serialise() const
{
    return serialise_double(param_c);
}

PL2Weight *
PL2Weight::unserialise(const string & s) const
{
    const char *ptr = s.data();
    const char *end = ptr + s.size();
    double c = unserialise_double(&ptr, end);
    if (rare(ptr != end))
	throw Xapian::SerialisationError("Extra data in PL2Weight::unserialise()");
    return new PL2Weight(c);
}

double
PL2Weight::get_sumpart(Xapian::termcount wdf, Xapian::termcount len) const
{
    if (wdf == 0) return 0.0;
    double wdfn(wdf);
    double base_change(log(2));

    wdfn *= log2(1 + (param_c * get_average_length()) / len);

    double L = 1.0 / (wdfn + 1);

    double F(get_collection_freq());
    double N(get_collection_size());

    double mean = F / N;

    if (mean == 0) return 0.0;

    double P = wdfn * log2(1.0 / mean) +
	       mean / base_change +
	       0.5 * log2(2.0 * M_PI * wdfn) +
	       wdfn * (log2(wdfn) - (1 / base_change));

    return (get_wqf() * L * P) - lower_bound;
}

double
PL2Weight::get_maxpart() const
{
    return upper_bound;
}

double
PL2Weight::get_sumextra(Xapian::termcount) const
{
    return 0;
}

double
PL2Weight::get_maxextra() const
{
    return 0;
}

}
