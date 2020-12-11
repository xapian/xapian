/** @file
 * @brief Xapian::BM25PlusWeight class - the BM25+ probabilistic formula
 */
/* Copyright (C) 2009,2010,2011,2012,2014,2015,2016 Olly Betts
 * Copyright (C) 2016  Vivek Pal
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

#include "debuglog.h"
#include "omassert.h"
#include "serialise-double.h"

#include "xapian/error.h"

#include <algorithm>
#include <cmath>

using namespace std;

namespace Xapian {

BM25PlusWeight *
BM25PlusWeight::clone() const
{
    return new BM25PlusWeight(param_k1, param_k2, param_k3, param_b,
			      param_min_normlen, param_delta);
}

void
BM25PlusWeight::init(double factor)
{
    Xapian::doccount tf = get_termfreq();

    if (rare(tf == 0)) {
	termweight = 0;
    } else {
	// BM25+ formula uses IDF = log((total_no_of_docs + 1) / tf)
	termweight = log(double(get_collection_size() + 1) / tf);
	termweight *= factor;
	if (param_k3 != 0) {
	    double wqf_double = get_wqf();
	    termweight *= (param_k3 + 1) * wqf_double / (param_k3 + wqf_double);
	}
    }

    LOGVALUE(WTCALC, termweight);

    if (param_k2 == 0 && (param_b == 0 || param_k1 == 0)) {
	// If k2 is 0, and either param_b or param_k1 is 0 then the document
	// length doesn't affect the weight.
	len_factor = 0;
    } else {
	len_factor = get_average_length();
	// len_factor can be zero if all documents are empty (or the database
	// is empty!)
	if (len_factor != 0) len_factor = 1 / len_factor;
    }

    LOGVALUE(WTCALC, len_factor);
}

string
BM25PlusWeight::name() const
{
    return "Xapian::BM25PlusWeight";
}

string
BM25PlusWeight::serialise() const
{
    string result = serialise_double(param_k1);
    result += serialise_double(param_k2);
    result += serialise_double(param_k3);
    result += serialise_double(param_b);
    result += serialise_double(param_min_normlen);
    result += serialise_double(param_delta);
    return result;
}

BM25PlusWeight *
BM25PlusWeight::unserialise(const string & s) const
{
    const char *ptr = s.data();
    const char *end = ptr + s.size();
    double k1 = unserialise_double(&ptr, end);
    double k2 = unserialise_double(&ptr, end);
    double k3 = unserialise_double(&ptr, end);
    double b = unserialise_double(&ptr, end);
    double min_normlen = unserialise_double(&ptr, end);
    double delta = unserialise_double(&ptr, end);
    if (rare(ptr != end))
	throw Xapian::SerialisationError("Extra data in BM25PlusWeight::unserialise()");
    return new BM25PlusWeight(k1, k2, k3, b, min_normlen, delta);
}

double
BM25PlusWeight::get_sumpart(Xapian::termcount wdf, Xapian::termcount len,
			    Xapian::termcount) const
{
    LOGCALL(WTCALC, double, "BM25PlusWeight::get_sumpart", wdf | len);
    Xapian::doclength normlen = max(len * len_factor, param_min_normlen);

    double wdf_double = wdf;
    double denom = param_k1 * (normlen * param_b + (1 - param_b)) + wdf_double;
    AssertRel(denom,>,0);
    // Parameter delta (δ) is a pseudo tf value to control the scale of the
    // tf lower bound. δ can be tuned for e.g from 0.0 to 1.5 but BM25+ can
    // still work effectively across collections with a fixed δ = 1.0
    RETURN(termweight * ((param_k1 + 1) * wdf_double / denom + param_delta));
}

double
BM25PlusWeight::get_maxpart() const
{
    LOGCALL(WTCALC, double, "BM25PlusWeight::get_maxpart", NO_ARGS);
    double denom = param_k1;
    Xapian::termcount wdf_max = get_wdf_upper_bound();
    if (param_k1 != 0.0) {
	if (param_b != 0.0) {
	    // "Upper-bound Approximations for Dynamic Pruning" Craig
	    // Macdonald, Nicola Tonellotto and Iadh Ounis. ACM Transactions on
	    // Information Systems. 29(4), 2011 shows that evaluating at
	    // doclen=wdf_max is a good bound.
	    //
	    // However, we can do better if doclen_min > wdf_max since then a
	    // better bound can be found by simply evaluating at
	    // doclen=doclen_min and wdf=wdf_max.
	    Xapian::doclength normlen_lb =
		 max(max(wdf_max, get_doclength_lower_bound()) * len_factor,
		     param_min_normlen);
	    denom *= (normlen_lb * param_b + (1 - param_b));
	}
    }
    denom += wdf_max;
    AssertRel(denom,>,0);
    RETURN(termweight * ((param_k1 + 1) * wdf_max / denom + param_delta));
}

/* The paper which describes BM25+ ignores BM25's document-independent
 * component (so implicitly k2=0), but we support non-zero k2 too.
 *
 * The BM25 formula gives:
 *
 * param_k2 * query_length * (1 - normlen) / (1 + normlen)
 *
 * To avoid negative sumextra we add the constant (param_k2 * query_length)
 * to give:
 *
 * 2 * param_k2 * query_length / (1 + normlen)
 */
double
BM25PlusWeight::get_sumextra(Xapian::termcount len, Xapian::termcount) const
{
    LOGCALL(WTCALC, double, "BM25PlusWeight::get_sumextra", len);
    double num = (2.0 * param_k2 * get_query_length());
    RETURN(num / (1.0 + max(len * len_factor, param_min_normlen)));
}

double
BM25PlusWeight::get_maxextra() const
{
    LOGCALL(WTCALC, double, "BM25PlusWeight::get_maxextra", NO_ARGS);
    if (param_k2 == 0.0)
	RETURN(0.0);
    double num = (2.0 * param_k2 * get_query_length());
    RETURN(num / (1.0 + max(get_doclength_lower_bound() * len_factor,
			    param_min_normlen)));
}

}
