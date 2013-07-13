/** @file tfidfweight.cc
 * @brief Xapian::TfIdfWeight class - The TfIdf weighting scheme
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
#include <cmath>
#include <cstring>

#include "debuglog.h"
#include "omassert.h"

#include "xapian/error.h"

using namespace std;

namespace Xapian {

TfIdfWeight::TfIdfWeight(const std::string &normals)
    : normalizations(normals)
{
    if (normalizations.length() != 3 ||
	!strchr("nbsl", normalizations[0]) ||
	!strchr("ntp", normalizations[1]) ||
	!strchr("n", normalizations[2]))
	throw Xapian::InvalidArgumentError("Normalization string is invalid");
    if (normalizations[1] != 'n') {
	need_stat(TERMFREQ);
	need_stat(COLLECTION_SIZE);
    }
    need_stat(WDF);
    need_stat(WDF_MAX);
}

TfIdfWeight *
TfIdfWeight::clone() const
{
    return new TfIdfWeight(normalizations);
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
    return normalizations;
}

TfIdfWeight *
TfIdfWeight::unserialise(const string & s) const
{
    if (s.length() != 3)
	throw Xapian::SerialisationError("Extra data in TfIdfWeight::unserialise()");
    return new TfIdfWeight(s);
}

double
TfIdfWeight::get_sumpart(Xapian::termcount wdf, Xapian::termcount) const
{
    Xapian::doccount termfreq = 1;
    if (normalizations[1] != 'n') termfreq = get_termfreq();
    double wt = get_wdfn(wdf, normalizations[0]) *
		get_idfn(termfreq, normalizations[1]);
    return get_wtn(wt, normalizations[2]) * factor;
}

// An upper bound can be calculated simply on the basis of wdf_max as termfreq
// and N are constants.
double
TfIdfWeight::get_maxpart() const
{
    Xapian::doccount termfreq = 1;
    if (normalizations[1] != 'n') termfreq = get_termfreq();
    Xapian::termcount wdf_max = get_wdf_upper_bound();
    double wt = get_wdfn(wdf_max, normalizations[0]) *
		get_idfn(termfreq, normalizations[1]);
    return get_wtn(wt, normalizations[2]) * factor;
}

// There is no extra per document component in the TfIdfWeighting scheme.
double
TfIdfWeight::get_sumextra(Xapian::termcount) const
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
	    return (1 + log(wdf));
	default:
	    AssertEq(c, 'n');
	    return wdf;
    }
}

double
TfIdfWeight::get_idfn(Xapian::doccount termfreq, char c) const
{
    double N = 1.0;
    if (c != 'n') N = get_collection_size();
    switch (c) {
	case 'n':
	    return 1.0;
	case 'p':
	    // All documents are indexed by the term
	    if (N == termfreq) return 0;
	    return log((N - termfreq) / termfreq);
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
