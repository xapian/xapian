/** @file tfidfweight.cc
 * @brief Xapian::TfIdfWeight class - The TfIdf weighting scheme
 */
/* Copyright (C) 2013 Aarsh Shah
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
}

TfIdfWeight::TfIdfWeight(const std::string &normals, double slope, double delta)
    : normalizations(normals), param_slope(slope), param_delta(delta)
{
    if (normalizations.length() != 3 ||
	!strchr("nbslP", normalizations[0]) ||
	!strchr("ntpfsP", normalizations[1]) ||
	!strchr("nP", normalizations[2]))
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
    if (normalizations[2] == 'P') {
	need_stat(AVERAGE_LENGTH);
	need_stat(DOC_LENGTH);
	need_stat(WQF);
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
    factor = factor_;
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
    Xapian::doccount termfreq = 1;
    if (normalizations[1] != 'n') termfreq = get_termfreq();
    double wt = 1.0;
    if (normalizations[2] == 'P') {
	double wqf = get_wqf();
	wt = (get_wdfn(wdf, normalizations[0]) *
		get_wtn(doclen, wt, normalizations[2]) + param_delta) *
		    get_idfn(termfreq, normalizations[1]);
	return wqf * wt * factor;
    } else {
	wt = get_wdfn(wdf, normalizations[0]) *
		get_idfn(termfreq, normalizations[1]);
	return get_wtn(doclen, wt, normalizations[2]) * factor;
    }
}

// An upper bound can be calculated simply on the basis of wdf_max as termfreq
// and N are constants.
double
TfIdfWeight::get_maxpart() const
{
    Xapian::doccount termfreq = 1;
    if (normalizations[1] != 'n') termfreq = get_termfreq();
    Xapian::termcount wdf_max = get_wdf_upper_bound();
    double wt = 1.0;
    if (normalizations[2] =='P') {
	double wqf = get_wqf();
	wt = (get_wdfn(wdf_max, normalizations[0]) *
		get_wtn(get_doclength_lower_bound(), wt, normalizations[2]) + param_delta) *
		    get_idfn(termfreq, normalizations[1]);
	return wqf * wt * factor;
    } else {
	wt = get_wdfn(wdf_max, normalizations[0]) *
		get_idfn(termfreq, normalizations[1]);
	return get_wtn(1.0, wt, normalizations[2]) * factor;
    }
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
TfIdfWeight::get_wdfn(Xapian::termcount wdf, char c) const
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
	case 'P':
	    if (wdf == 0) return 0;
	    return (1 + log(1 + log(double(wdf))));
	default:
	    AssertEq(c, 'n');
	    return wdf;
    }
}

double
TfIdfWeight::get_idfn(Xapian::doccount termfreq, char c) const
{
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
	    return (log((N+1) / termfreq));
	default:
	    AssertEq(c, 't');
	    return (log(N / termfreq));
    }
}

double
TfIdfWeight::get_wtn(Xapian::termcount doclen, double wt, char c) const
{
    switch (c) {
	case 'P': {
	    double normlen = doclen / get_average_length();
	    double norm_factor =  1 / (1 - param_slope + (param_slope * normlen));
	    return wt * norm_factor;
	}
	default:
	    AssertEq(c, 'n');
	    return wt;
    }
}

}
