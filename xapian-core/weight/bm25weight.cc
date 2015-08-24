/** @file bm25weight.cc
 * @brief Xapian::BM25Weight class - the BM25 probabilistic formula
 */
/* Copyright (C) 2009,2010,2014,2015 Olly Betts
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

BM25Weight *
BM25Weight::clone() const
{
    return new BM25Weight(param_k1, param_k2, param_k3, param_b,
			  param_min_normlen);
}

void
BM25Weight::init(double factor)
{
    Xapian::doccount tf = get_termfreq();

    Xapian::weight tw = 0;
    if (get_rset_size() != 0) {
	Xapian::doccount reltermfreq = get_reltermfreq();

	// There can't be more relevant documents indexed by a term than there
	// are documents indexed by that term.
	AssertRel(reltermfreq,<=,tf);

	// There can't be more relevant documents indexed by a term than there
	// are relevant documents.
	AssertRel(reltermfreq,<=,get_rset_size());

	Xapian::doccount reldocs_not_indexed = get_rset_size() - reltermfreq;

	// There can't be more relevant documents not indexed by a term than
	// there are documents not indexed by that term.
	AssertRel(reldocs_not_indexed,<=,get_collection_size() - tf);

	Xapian::doccount Q = get_collection_size() - reldocs_not_indexed;

	Xapian::doccount nonreldocs_indexed = tf - reltermfreq;
	double numerator = (reltermfreq + 0.5) * (Q - tf + 0.5);
	double denom = (reldocs_not_indexed + 0.5) * (nonreldocs_indexed + 0.5);
	tw = numerator / denom;
    } else {
	tw = (get_collection_size() - tf + 0.5) / (tf + 0.5);
    }

    AssertRel(tw,>,0);

    // The "official" formula can give a negative termweight in unusual cases
    // (without an RSet, when a term indexes more than half the documents in
    // the database).  These negative weights aren't actually helpful, and it
    // is common for implementations to replace them with a small positive
    // weight or similar.
    //
    // Truncating to zero doesn't seem a great approach in practice as it
    // means that some terms in the query can have no affect at all on the
    // ranking, and that some results can have zero weight, both of which
    // are seem surprising.
    //
    // Xapian 1.0.x and earlier adjusted the termweight for any term indexing
    // more than a third of documents, which seems rather "intrusive".  That's
    // what the code currently enabled does, but perhaps it would be better to
    // do something else. (FIXME)
#if 0
    if (rare(tw <= 1.0)) {
	termweight = 0;
    } else {
	termweight = log(tw) * factor;
	if (param_k3 != 0) {
	    double wqf_double = get_wqf();
	    termweight *= (param_k3 + 1) * wqf_double / (param_k3 + wqf_double);
	}
    }
#else
    if (tw < 2) tw = tw * 0.5 + 1;
    termweight = log(tw) * factor;
    if (param_k3 != 0) {
	double wqf_double = get_wqf();
	termweight *= (param_k3 + 1) * wqf_double / (param_k3 + wqf_double);
    }
#endif
    termweight *= (param_k1 + 1);

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
BM25Weight::name() const
{
    return "Xapian::BM25Weight";
}

string
BM25Weight::serialise() const
{
    string result = serialise_double(param_k1);
    result += serialise_double(param_k2);
    result += serialise_double(param_k3);
    result += serialise_double(param_b);
    result += serialise_double(param_min_normlen);
    return result;
}

BM25Weight *
BM25Weight::unserialise(const string & s) const
{
    const char *ptr = s.data();
    const char *end = ptr + s.size();
    double k1 = unserialise_double(&ptr, end);
    double k2 = unserialise_double(&ptr, end);
    double k3 = unserialise_double(&ptr, end);
    double b = unserialise_double(&ptr, end);
    double min_normlen = unserialise_double(&ptr, end);
    if (rare(ptr != end))
	throw Xapian::NetworkError("Extra data in BM25Weight::unserialise()");
    return new BM25Weight(k1, k2, k3, b, min_normlen);
}

Xapian::weight
BM25Weight::get_sumpart(Xapian::termcount wdf, Xapian::termcount len) const
{
    LOGCALL(WTCALC, Xapian::weight, "BM25Weight::get_sumpart", wdf | len);
    Xapian::doclength normlen = max(len * len_factor, param_min_normlen);

    double wdf_double = wdf;
    double denom = param_k1 * (normlen * param_b + (1 - param_b)) + wdf_double;
    AssertRel(denom,>,0);
    RETURN(termweight * (wdf_double / denom));
}

Xapian::weight
BM25Weight::get_maxpart() const
{
    LOGCALL(WTCALC, Xapian::weight, "BM25Weight::get_maxpart", NO_ARGS);
    double denom = param_k1;
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
		 max(max(get_wdf_upper_bound(), get_doclength_lower_bound()) * len_factor, param_min_normlen);
	    denom *= (normlen_lb * param_b + (1 - param_b));
	}
    }
    double wdf_max = get_wdf_upper_bound();
    denom += wdf_max;
    AssertRel(denom,>,0);
    RETURN(termweight * (wdf_max / denom));
}

/* The BM25 formula gives:
 *
 * param_k2 * query_length * (1 - normlen) / (1 + normlen)
 *
 * To avoid negative sumextra we add the constant (param_k2 * query_length)
 * to give:
 *
 * 2 * param_k2 * query_length / (1 + normlen)
 */
Xapian::weight
BM25Weight::get_sumextra(Xapian::termcount len) const
{
    LOGCALL(WTCALC, Xapian::weight, "BM25Weight::get_sumextra", len);
    Xapian::weight num = (2.0 * param_k2 * get_query_length());
    RETURN(num / (1.0 + max(len * len_factor, param_min_normlen)));
}

Xapian::weight
BM25Weight::get_maxextra() const
{
    LOGCALL(WTCALC, Xapian::weight, "BM25Weight::get_maxextra", NO_ARGS);
    if (param_k2 == 0.0)
	RETURN(0.0);
    Xapian::weight num = (2.0 * param_k2 * get_query_length());
    RETURN(num / (1.0 + max(get_doclength_lower_bound() * len_factor,
			    param_min_normlen)));
}

}
