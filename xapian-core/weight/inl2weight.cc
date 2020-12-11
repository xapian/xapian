/** @file
 * @brief Xapian::InL2Weight class - the InL2 weighting scheme of the DFR framework.
 */
/* Copyright (C) 2013,2014 Aarsh Shah
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
	throw Xapian::InvalidArgumentError("Parameter c is invalid");
    need_stat(AVERAGE_LENGTH);
    need_stat(DOC_LENGTH);
    need_stat(DOC_LENGTH_MIN);
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
InL2Weight::init(double factor)
{
    if (factor == 0.0) {
	// This object is for the term-independent contribution, and that's
	// always zero for this scheme.
	return;
    }

    double wdfn_upper = get_wdf_upper_bound();
    if (wdfn_upper == 0) {
	upper_bound = 0.0;
	return;
    }

    double termfreq = get_termfreq();
    double N = get_collection_size();

    wdfn_upper *= log2(1 + (param_c * get_average_length()) /
		       get_doclength_lower_bound());

    // wdfn * L = wdfn / (wdfn + 1) = 1 / (1 + 1 / wdfn).
    // To maximize the product, we need to minimize the denominator and so we use wdfn_upper in (1 / wdfn).
    double maximum_wdfn_product_L = wdfn_upper / (wdfn_upper + 1.0);

    // This term is constant for all documents.
    double idf_max = log2((N + 1) / (termfreq + 0.5));

    /* Calculate constant values to be used in get_sumpart() upfront. */
    wqf_product_idf = get_wqf() * idf_max * factor;
    c_product_avlen = param_c * get_average_length();

    upper_bound = wqf_product_idf * maximum_wdfn_product_L * factor;
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
InL2Weight::get_sumpart(Xapian::termcount wdf, Xapian::termcount len,
			Xapian::termcount) const
{
    if (wdf == 0) return 0.0;
    double wdfn = wdf;

    wdfn *= log2(1 + c_product_avlen / len);

    double wdfn_product_L = wdfn / (wdfn + 1.0);

    return (wqf_product_idf * wdfn_product_L);
}

double
InL2Weight::get_maxpart() const
{
    return upper_bound;
}

double
InL2Weight::get_sumextra(Xapian::termcount, Xapian::termcount) const
{
    return 0;
}

double
InL2Weight::get_maxextra() const
{
    return 0;
}

}
