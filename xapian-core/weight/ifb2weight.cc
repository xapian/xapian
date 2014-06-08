/** @file ifb2weight.cc
 * @brief Xapian::IfB2Weight class - the IfB2 weighting scheme of the DFR framework.
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

IfB2Weight::IfB2Weight(double c)
   : param_c(c)
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
    need_stat(TERMFREQ);
}

IfB2Weight *
IfB2Weight::clone() const
{
    return new IfB2Weight(param_c);
}

void
IfB2Weight::init(double factor_)
{
    factor = factor_;

    double wdfn_upper(get_wdf_upper_bound());
    if (wdfn_upper == 0) {
	upper_bound = 0.0;
	return;
    }

    double F(get_collection_freq());
    double N(get_collection_size());

    wdfn_upper *= log2(1 + (param_c * get_average_length()) /
		    get_doclength_lower_bound());

    double max_wdfn_product_B = (F + 1.0) / (get_termfreq() * (1.0 + (1.0 / wdfn_upper)));

    double idf_max = log2((N + 1.0) / (F + 0.5));

    /* Calculate constant values to be used in get_sumpart() . */
    wqf_product_idf = get_wqf() * idf_max;
    c_product_avlen = param_c * get_average_length();

    upper_bound = wqf_product_idf * max_wdfn_product_B;
}

string
IfB2Weight::name() const
{
    return "Xapian::IfB2Weight";
}

string
IfB2Weight::serialise() const
{
    return serialise_double(param_c);
}

IfB2Weight *
IfB2Weight::unserialise(const string & s) const
{
    const char *ptr = s.data();
    const char *end = ptr + s.size();
    double c = unserialise_double(&ptr, end);
    if (rare(ptr != end))
	throw Xapian::SerialisationError("Extra data in IfB2Weight::unserialise()");
    return new IfB2Weight(c);
}

double
IfB2Weight::get_sumpart(Xapian::termcount wdf, Xapian::termcount len) const
{
    if (wdf == 0) return 0.0;
    double wdfn(wdf);
    wdfn *= log2(1 + c_product_avlen / len);

    double F(get_collection_freq());

    double wdfn_product_B = (F + 1.0) / (get_termfreq() * (1.0 + (1.0 / wdfn)));

    return (wqf_product_idf * wdfn_product_B * factor);
}

double
IfB2Weight::get_maxpart() const
{
    return upper_bound * factor;
}

double
IfB2Weight::get_sumextra(Xapian::termcount) const
{
    return 0;
}

double
IfB2Weight::get_maxextra() const
{
    return 0;
}

}
