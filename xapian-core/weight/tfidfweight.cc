/** @file tfidfweight.cc
 * @brief Xapian::TfIdfWeight class - The TfIdf weighting scheme
 */
/* Copyright (C) 2013 Aarsh Shah
 * Copyright (C) 2016 Vivek Pal
 * Copyright (C) 2016 Olly Betts
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
#include <cmath>
#include <cstring>

#include "debuglog.h"
#include "omassert.h"
#include "serialise-double.h"

#include "xapian/error.h"

using namespace std;

namespace Xapian {

TfIdfWeight::TfIdfWeight(const std::string &normals)
    : normalizations(normals), param_slope(0.2), param_delta(1.0)
{
    if (normalizations.length() != 3 ||
	!strchr("nbslP", normalizations[0]) ||
	!strchr("ntpfsP", normalizations[1]) ||
	!strchr("n", normalizations[2]))
	throw Xapian::InvalidArgumentError("Normalization string is invalid");
    if (normalizations[1] != 'n') {
	need_stat(TERMFREQ);
	need_stat(COLLECTION_SIZE);
    }
    need_stat(WDF);
    need_stat(WDF_MAX);
    need_stat(WQF);
}

TfIdfWeight::TfIdfWeight(const std::string &normals, double slope, double delta)
    : normalizations(normals), param_slope(slope), param_delta(delta)
{
    if (normalizations.length() != 3 ||
	!strchr("nbslP", normalizations[0]) ||
	!strchr("ntpfsP", normalizations[1]) ||
	!strchr("n", normalizations[2]))
	throw Xapian::InvalidArgumentError("Normalization string is invalid");
    if (param_slope <= 0)
	throw Xapian::InvalidArgumentError("Parameter slope is invalid.");
    if (param_delta <= 0)
	throw Xapian::InvalidArgumentError("Parameter delta is invalid.");
    if (normalizations[1] != 'n') {
	need_stat(TERMFREQ);
	need_stat(COLLECTION_SIZE);
    }
    need_stat(WDF);
    need_stat(WDF_MAX);
    need_stat(WQF);
    if (normalizations[0] == 'P' || normalizations[1] == 'P') {
	need_stat(AVERAGE_LENGTH);
	need_stat(DOC_LENGTH);
	need_stat(DOC_LENGTH_MIN);
    }
}

TfIdfWeight *
TfIdfWeight::clone() const
{
    return new TfIdfWeight(normalizations, param_slope, param_delta);
}

void
TfIdfWeight::init(double factor_)
{
    wqf_factor = get_wqf() * factor_;
    idfn = get_idfn(normalizations[1]);
}

string
TfIdfWeight::name() const
{
    return "Xapian::TfIdfWeight";
}

string
TfIdfWeight::serialise() const
{
    string result = serialise_double(param_slope);
    result += serialise_double(param_delta);
    result += normalizations;
    return result;
}

TfIdfWeight *
TfIdfWeight::unserialise(const string & s) const
{
    const char *ptr = s.data();
    const char *end = ptr + s.size();
    double slope = unserialise_double(&ptr, end);
    double delta = unserialise_double(&ptr, end);
    string normals(ptr, end);
    ptr += 3;
    if (rare(ptr != end))
	throw Xapian::SerialisationError("Extra data in TfIdfWeight::unserialise()");
    return new TfIdfWeight(normals, slope, delta);
}

double
TfIdfWeight::get_sumpart(Xapian::termcount wdf, Xapian::termcount doclen,
			 Xapian::termcount) const
{
    double wdfn = get_wdfn(wdf, doclen, normalizations[0]);
    return get_wtn(wdfn * idfn, normalizations[2]) * wqf_factor;
}

// An upper bound can be calculated simply on the basis of wdf_max as termfreq
// and N are constants.
double
TfIdfWeight::get_maxpart() const
{
    Xapian::termcount wdf_max = get_wdf_upper_bound();
    Xapian::termcount len_min = get_doclength_lower_bound();
    double wdfn = get_wdfn(wdf_max, len_min, normalizations[0]);
    return get_wtn(wdfn * idfn, normalizations[2]) * wqf_factor;
}

// There is no extra per document component in the TfIdfWeighting scheme.
double
TfIdfWeight::get_sumextra(Xapian::termcount, Xapian::termcount) const
{
    return 0;
}

double
TfIdfWeight::get_maxextra() const
{
    return 0;
}

// Return normalized wdf, idf and weight depending on the normalization string.
double
TfIdfWeight::get_wdfn(Xapian::termcount wdf, Xapian::termcount doclen, char c) const
{
    switch (c) {
	case 'b':
	    if (wdf == 0) return 0;
	    return 1.0;
	case 's':
	    return (wdf * wdf);
	case 'l':
	    if (wdf == 0) return 0;
	    return (1 + log(double(wdf)));
	case 'P': {
	    if (wdf == 0) return 0;
	    double normlen = doclen / get_average_length();
	    double norm_factor =  1 / (1 - param_slope + (param_slope * normlen));
	    return ((1 + log(1 + log(double(wdf)))) * norm_factor + param_delta);
	}
	default:
	    AssertEq(c, 'n');
	    return wdf;
    }
}

double
TfIdfWeight::get_idfn(char c) const
{
    Xapian::doccount termfreq = 1;
    if (c != 'n') termfreq = get_termfreq();
    double N = 1.0;
    if (c != 'n' && c != 'f') N = get_collection_size();
    switch (c) {
	case 'n':
	    return 1.0;
	case 'p':
	    // All documents are indexed by the term
	    if (N == termfreq) return 0;
	    return log((N - termfreq) / termfreq);
	case 'f':
	    return (1.0 / termfreq);
	case 's':
	    return pow(log(N / termfreq), 2.0);
	case 'P':
	    return log((N + 1) / termfreq);
	default:
	    AssertEq(c, 't');
	    return (log(N / termfreq));
    }
}

double
TfIdfWeight::get_wtn(double wt, char c) const
{
    (void)c;
    AssertEq(c, 'n');
    return wt;
}

}
