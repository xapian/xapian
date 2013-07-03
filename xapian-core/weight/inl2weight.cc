/** @file inl2weight.cc
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
#include "common/log2.h"

#include "serialise-double.h"

#include "xapian/error.h"

using namespace std;

namespace Xapian {

InL2Weight::InL2Weight(double c)
    : param_c(c)
{
    if (param_c <= 0)
        throw Xapian::InvalidArgumentError("Parameter c is invalid.");
    need_stat(AVERAGE_LENGTH);
    need_stat(DOC_LENGTH);
    need_stat(DOC_LENGTH_MIN);
    need_stat(DOC_LENGTH_MAX);
    need_stat(COLLECTION_SIZE);
    need_stat(WDF);
    need_stat(WDF_MAX);
    need_stat(WQF);
    need_stat(TERMFREQ);
}

InL2Weight *
InL2Weight::clone() const
{
    return new InL2Weight(param_c);
}

void
InL2Weight::init(double)
{
    double wdfn_upper(get_wdf_upper_bound());
    if (wdfn_upper == 0) {
        upper_bound = 0.0;
        return;
    }

    double wdfn_lower(1.0);
    double termfrequency(get_termfreq());
    double N(get_collection_size());

    wdfn_lower *= log2(1 + (param_c * get_average_length()) /
                    get_doclength_upper_bound());

    wdfn_upper *= log2(1 + (param_c * get_average_length()) /
                    get_doclength_lower_bound());

    double L_max = 1 / (wdfn_lower + 1);

    double idf_max = log2((N + 1) / (termfrequency + 0.5));

    upper_bound = get_wqf() * wdfn_upper * L_max * idf_max;
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
    double N(get_collection_size());
    double termfrequency(get_termfreq());

    wdfn *= log2(1 + (param_c * get_average_length()) / len);

    double L = 1 / (wdfn + 1);

    double idf = log2((N + 1) / (termfrequency + 0.5));

    return (get_wqf() * wdfn * L * idf);
}

double
InL2Weight::get_maxpart() const
{
    return upper_bound;
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
