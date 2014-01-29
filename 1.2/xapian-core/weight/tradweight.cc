/** @file tradweight.cc
 * @brief Xapian::TradWeight class - the "traditional" probabilistic formula
 */
/* Copyright (C) 2009,2010 Olly Betts
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

#include <cmath>

using namespace std;

namespace Xapian {

TradWeight *
TradWeight::clone() const
{
    return new TradWeight(param_k);
}

void
TradWeight::init(double factor)
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
    }
#else
    if (tw < 2) tw = tw * 0.5 + 1;
    termweight = log(tw) * factor;
#endif

    LOGVALUE(WTCALC, termweight);

    if (param_k == 0) {
	// If param_k is 0 then the document length doesn't affect the weight.
	len_factor = 0;
    } else {
	len_factor = get_average_length();
	// len_factor can be zero if all documents are empty (or the database is
	// empty!)
	if (len_factor != 0) len_factor = param_k / len_factor;
    }

    LOGVALUE(WTCALC, len_factor);
}

string
TradWeight::name() const
{
    return "Xapian::TradWeight";
}

string
TradWeight::serialise() const
{
    return serialise_double(param_k);
}

TradWeight *
TradWeight::unserialise(const string & s) const
{
    const char *ptr = s.data();
    const char *end = ptr + s.size();
    double k = unserialise_double(&ptr, end);
    if (rare(ptr != end))
	throw Xapian::NetworkError("Extra data in TradWeight::unserialise()");
    return new TradWeight(k);
}

Xapian::weight
TradWeight::get_sumpart(Xapian::termcount wdf, Xapian::termcount len) const
{
    double wdf_double(wdf);
    return termweight * (wdf_double / (len * len_factor + wdf_double));
}

Xapian::weight
TradWeight::get_maxpart() const
{
    // FIXME: need to force non-zero wdf_max to stop percentages breaking...
    double wdf_max(max(get_wdf_upper_bound(), Xapian::termcount(1)));
    Xapian::termcount doclen_lb = get_doclength_lower_bound();
    return termweight * (wdf_max / (doclen_lb * len_factor + wdf_max));
}

Xapian::weight
TradWeight::get_sumextra(Xapian::termcount) const
{
    return 0;
}

Xapian::weight
TradWeight::get_maxextra() const
{
    return 0;
}

}
