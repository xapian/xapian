/** @file
 * @brief Xapian::TfIdfWeight class - The TfIdf weighting scheme
 */
/* Copyright (C) 2013 Aarsh Shah
 * Copyright (C) 2016 Vivek Pal
 * Copyright (C) 2016,2017 Olly Betts
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
#include "keyword.h"
#include "weight/idf-norm-dispatch.h"
#include "weight/wdf-norm-dispatch.h"
#include "weightinternal.h"
#include <cmath>
#include <cstring>

#include "debuglog.h"
#include "omassert.h"
#include "serialise-double.h"

#include "xapian/error.h"

using namespace std;

namespace Xapian {

static TfIdfWeight::wdf_norm
decode_wdf_norm(const string& normalizations)
{
    if (normalizations.length() != 3)
	throw Xapian::InvalidArgumentError("Normalization string is invalid");
    switch (normalizations[0]) {
	case 'b':
	    return TfIdfWeight::wdf_norm::BOOLEAN;
	case 's':
	    return TfIdfWeight::wdf_norm::SQUARE;
	case 'l':
	    return TfIdfWeight::wdf_norm::LOG;
	case 'P':
	    return TfIdfWeight::wdf_norm::PIVOTED;
	case 'L':
	    return TfIdfWeight::wdf_norm::LOG_AVERAGE;
	case 'n':
	    return TfIdfWeight::wdf_norm::NONE;
	case 'm':
	    return TfIdfWeight::wdf_norm::MAX;
	case 'a':
	    return TfIdfWeight::wdf_norm::AUG;
    }
    throw Xapian::InvalidArgumentError("Normalization string is invalid");
}

static TfIdfWeight::idf_norm
decode_idf_norm(const string& normalizations)
{
    if (normalizations.length() != 3)
	throw Xapian::InvalidArgumentError("Normalization string is invalid");
    switch (normalizations[1]) {
	case 'n':
	    return TfIdfWeight::idf_norm::NONE;
	case 's':
	    return TfIdfWeight::idf_norm::SQUARE;
	case 'f':
	    return TfIdfWeight::idf_norm::FREQ;
	case 'P':
	    return TfIdfWeight::idf_norm::PIVOTED;
	case 'p':
	    return TfIdfWeight::idf_norm::PROB;
	case 't':
	    return TfIdfWeight::idf_norm::TFIDF;
    }
    throw Xapian::InvalidArgumentError("Normalization string is invalid");
}

static TfIdfWeight::wt_norm
decode_wt_norm(const string& normalizations)
{
    if (normalizations.length() != 3)
	throw Xapian::InvalidArgumentError("Normalization string is invalid");
    switch (normalizations[2]) {
	case 'n':
	    return TfIdfWeight::wt_norm::NONE;
    }
    throw Xapian::InvalidArgumentError("Normalization string is invalid");
}

TfIdfWeight::TfIdfWeight(const std::string& normals,
			 double slope, double delta)
    : TfIdfWeight::TfIdfWeight(decode_wdf_norm(normals),
			       decode_idf_norm(normals),
			       decode_wt_norm(normals),
			       slope, delta) {}

TfIdfWeight::TfIdfWeight(wdf_norm wdf_normalization,
			 idf_norm idf_normalization,
			 wt_norm wt_normalization,
			 double slope, double delta)
    : wdf_norm_(wdf_normalization), idf_norm_(idf_normalization),
      wt_norm_(wt_normalization), param_slope(slope), param_delta(delta)
{
    if (param_slope <= 0)
	throw Xapian::InvalidArgumentError("Parameter slope is invalid");
    if (param_delta <= 0)
	throw Xapian::InvalidArgumentError("Parameter delta is invalid");
    if (idf_norm_ != idf_norm::NONE) {
	need_stat(TERMFREQ);
	need_stat(COLLECTION_SIZE);
    }
    need_stat(WDF);
    need_stat(WDF_MAX);
    need_stat(WQF);
    if (wdf_norm_ == wdf_norm::PIVOTED || idf_norm_ == idf_norm::PIVOTED) {
	need_stat(AVERAGE_LENGTH);
	need_stat(DOC_LENGTH);
	need_stat(DOC_LENGTH_MIN);
    }
    if (wdf_norm_ == wdf_norm::LOG_AVERAGE ||
	wdf_norm_ == wdf_norm::AUG_AVERAGE) {
	need_stat(DOC_LENGTH);
	need_stat(DOC_LENGTH_MIN);
	need_stat(DOC_LENGTH_MAX);
	need_stat(UNIQUE_TERMS);
    }
    if (wdf_norm_ == wdf_norm::MAX || wdf_norm_ == wdf_norm::AUG) {
	need_stat(WDF_DOC_MAX);
    }
    if (idf_norm_ == idf_norm::GLOBAL_FREQ ||
	idf_norm_ == idf_norm::LOG_GLOBAL_FREQ ||
	idf_norm_ == idf_norm::INCREMENTED_GLOBAL_FREQ ||
	idf_norm_ == idf_norm::SQRT_GLOBAL_FREQ) {
	need_stat(COLLECTION_FREQ);
    }
}

TfIdfWeight *
TfIdfWeight::clone() const
{
    return new TfIdfWeight(wdf_norm_, idf_norm_, wt_norm_,
			   param_slope, param_delta);
}

void
TfIdfWeight::init(double factor_)
{
    if (factor_ == 0.0) {
	// This object is for the term-independent contribution, and that's
	// always zero for this scheme.
	return;
    }

    wqf_factor = get_wqf() * factor_;
    idfn = get_idfn(idf_norm_);
}

string
TfIdfWeight::name() const
{
    return "Xapian::TfIdfWeight";
}

string
TfIdfWeight::short_name() const
{
    return "tfidf";
}

string
TfIdfWeight::serialise() const
{
    string result = serialise_double(param_slope);
    result += serialise_double(param_delta);
    result += static_cast<unsigned char>(wdf_norm_);
    result += static_cast<unsigned char>(idf_norm_);
    result += static_cast<unsigned char>(wt_norm_);
    return result;
}

TfIdfWeight *
TfIdfWeight::unserialise(const string & s) const
{
    const char *ptr = s.data();
    const char *end = ptr + s.size();
    double slope = unserialise_double(&ptr, end);
    double delta = unserialise_double(&ptr, end);
    if (rare(end - ptr != 3))
	throw Xapian::SerialisationError
	      ("Incorrect data in TfIdfWeight::unserialise()");
    wdf_norm wdf_normalization = static_cast<wdf_norm>(*(ptr)++);
    idf_norm idf_normalization = static_cast<idf_norm>(*(ptr)++);
    wt_norm wt_normalization = static_cast<wt_norm>(*(ptr)++);
    return new TfIdfWeight(wdf_normalization, idf_normalization,
			   wt_normalization, slope, delta);
}

double
TfIdfWeight::get_sumpart(Xapian::termcount wdf,
			 Xapian::termcount doclen,
			 Xapian::termcount uniqterms,
			 Xapian::termcount wdfdocmax) const
{
    double wdfn = get_wdfn(wdf, doclen, uniqterms, wdfdocmax, wdf_norm_);
    return get_wtn(wdfn * idfn, wt_norm_) * wqf_factor;
}

// An upper bound can be calculated simply on the basis of wdf_max as termfreq
// and N are constants.
double
TfIdfWeight::get_maxpart() const
{
    Xapian::termcount wdf_max = get_wdf_upper_bound();
    Xapian::termcount len_min = get_doclength_lower_bound();
    double wdfn = get_wdfn(wdf_max, len_min, len_min, wdf_max, wdf_norm_);
    return get_wtn(wdfn * idfn, wt_norm_) * wqf_factor;
}

// Return normalized wdf, idf and weight depending on the normalization string.
double
TfIdfWeight::get_wdfn(Xapian::termcount wdf, Xapian::termcount doclen,
		      Xapian::termcount uniqterms,
		      Xapian::termcount wdfdocmax,
		      wdf_norm wdf_normalization) const
{
    switch (wdf_normalization) {
	case wdf_norm::BOOLEAN:
	    if (wdf == 0) return 0;
	    return 1.0;
	case wdf_norm::SQUARE:
	    return (wdf * wdf);
	case wdf_norm::LOG:
	    if (wdf == 0) return 0;
	    return (1 + log(double(wdf)));
	case wdf_norm::PIVOTED: {
	    if (wdf == 0) return 0;
	    double normlen = doclen / get_average_length();
	    double norm_factor = 1 / (1 - param_slope + (param_slope * normlen));
	    return ((1 + log(1 + log(double(wdf)))) * norm_factor + param_delta);
	}
	case wdf_norm::LOG_AVERAGE: {
	    if (wdf == 0) return 0;
	    double uniqterm_double = uniqterms;
	    double doclen_double = doclen;
	    double wdf_avg = 1;
	    if (doclen_double == 0 || uniqterm_double == 0)
		wdf_avg = 1;
	    else
		wdf_avg = doclen_double / uniqterm_double;
	    double num = 1 + log(double(wdf));
	    double den = 1 + log(wdf_avg);
	    return num / den;
	}
	case wdf_norm::AUG_LOG: {
	    if (wdf == 0) return 0;
	    return (0.2 + 0.8 * log(1.0 + wdf));
	}
	case wdf_norm::SQRT: {
	    if (wdf == 0) return 0;
	    return (sqrt(wdf - 0.5) + 1);
	}
	case wdf_norm::AUG_AVERAGE: {
	    if (wdf == 0) return 0;
	    return 0.9 + 0.1 * (double(wdf) / (double(doclen) / uniqterms));
	}
	case wdf_norm::MAX:
	    if (rare(wdfdocmax == 0)) return 0;
	    return double(wdf) / wdfdocmax;
	case wdf_norm::AUG: {
	    if (wdf == 0) return 0;
	    return 0.5 + 0.5 * (double(wdf) / wdfdocmax);
	}
	case wdf_norm::NONE:
	    break;
    }
    return wdf;
}

double
TfIdfWeight::get_idfn(idf_norm idf_normalization) const
{
    Xapian::doccount termfreq = 1;
    if (idf_normalization != idf_norm::NONE) termfreq = get_termfreq();
    double N = 1.0;
    if (idf_normalization == idf_norm::PROB ||
	idf_normalization == idf_norm::SQUARE ||
	idf_normalization == idf_norm::PIVOTED ||
	idf_normalization == idf_norm::TFIDF)
	    N = get_collection_size();
    Xapian::termcount collfreq = 1;
    switch (idf_normalization) {
	case idf_norm::NONE:
	    return 1.0;
	case idf_norm::PROB:
	    // All documents are indexed by the term
	    if (N == termfreq) return 0;
	    return log((N - termfreq) / termfreq);
	case idf_norm::FREQ:
	    return (1.0 / termfreq);
	case idf_norm::SQUARE: {
	    double x = log(N / termfreq);
	    return x * x;
	}
	case idf_norm::PIVOTED:
	    return log((N + 1) / termfreq);
	case idf_norm::GLOBAL_FREQ: {
	    collfreq = get_collection_freq();
	    return (double(collfreq) / termfreq);
	}
	case idf_norm::LOG_GLOBAL_FREQ: {
	    collfreq = get_collection_freq();
	    return log(double(collfreq) / termfreq + 1);
	}
	case idf_norm::INCREMENTED_GLOBAL_FREQ: {
	    collfreq = get_collection_freq();
	    return (double(collfreq) / termfreq + 1);
	}
	case idf_norm::SQRT_GLOBAL_FREQ: {
	    collfreq = get_collection_freq();
	    return sqrt(double(collfreq) / termfreq - 0.9);
	}
	case idf_norm::TFIDF:
	    break;
    }
    return (log(N / termfreq));
}

double
TfIdfWeight::get_wtn(double wt, wt_norm wt_normalization) const
{
    (void)wt_normalization;
    return wt;
}

static inline void
parameter_error(const char* message)
{
    Xapian::Weight::Internal::parameter_error(message, "tfidf");
}

TfIdfWeight *
TfIdfWeight::create_from_parameters(const char * p) const
{
    if (*p == '\0')
	return new Xapian::TfIdfWeight();
    string wdf_normalisation, idf_normalisation, wt_normalisation;
    if (!Xapian::Weight::Internal::param_name(&p, wdf_normalisation))
	parameter_error("Parameter 1 (wdf_normalisation) is invalid");
    if (!Xapian::Weight::Internal::param_name(&p, idf_normalisation))
	parameter_error("Parameter 2 (idf_normalisation) is invalid");
    if (!Xapian::Weight::Internal::param_name(&p, wt_normalisation))
	parameter_error("Parameter 3 (wt_normalisation) is invalid");
    if (*p)
	parameter_error("Extra data after wt_normalisation");

    int wdf_code = keyword(wdf_norm_tab, wdf_normalisation.data(),
			   wdf_normalisation.size());
    if (wdf_code < 0) {
	parameter_error("Parameter 1 (wdf_normalisation) is invalid");
    }
    wdf_norm wdf_normalisation_ = static_cast<wdf_norm>(wdf_code);

    int idf_code = keyword(idf_norm_tab, idf_normalisation.data(),
			   idf_normalisation.size());
    if (idf_code < 0) {
	parameter_error("Parameter 2 (idf_normalisation) is invalid");
    }
    idf_norm idf_normalisation_ = static_cast<idf_norm>(idf_code);

    if (wt_normalisation != "NONE") {
	parameter_error("Parameter 3 (wt_normalisation) is invalid");
    }
    wt_norm wt_normalisation_ = wt_norm::NONE;
    return new Xapian::TfIdfWeight(wdf_normalisation_, idf_normalisation_,
				   wt_normalisation_);
}

}
