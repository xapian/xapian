/* bm25weight.cc: Class for BM25 weight calculation
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007,2008 Olly Betts
 * Copyright 2007 Lemur Consulting Ltd
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>

#include <math.h>

#include <xapian/enquire.h>

#include "omassert.h"
#include "omdebug.h"
#include "serialise-double.h"
#include "weightinternal.h"

using namespace std;

namespace Xapian {

BM25Weight * BM25Weight::clone() const {
    return new BM25Weight(k1, k2, k3, b, min_normlen);
}

string BM25Weight::name() const { return "BM25"; }

string BM25Weight::serialise() const {
    string result = serialise_double(k1);
    result += serialise_double(k2);
    result += serialise_double(k3);
    result += serialise_double(b);
    result += serialise_double(min_normlen);
    return result;
}

BM25Weight * BM25Weight::unserialise(const string & s) const {
    const char *p = s.data();
    const char *p_end = p + s.size();
    double k1_ = unserialise_double(&p, p_end);
    double k2_ = unserialise_double(&p, p_end);
    double k3_ = unserialise_double(&p, p_end);
    double b_ = unserialise_double(&p, p_end);
    double min_normlen_ = unserialise_double(&p, p_end);
    // FIXME: should check that (p == p_end).
    return new BM25Weight(k1_, k2_, k3_, b_, min_normlen_);
}

// Calculate weights using statistics retrieved from databases
void
BM25Weight::calc_termweight() const
{
    DEBUGCALL(MATCH, void, "BM25Weight::calc_termweight", "");

    lenpart = internal->average_length;
    // lenpart == 0 if there are no documents, or only empty documents.
    if (lenpart != 0) lenpart = 1 / lenpart;

    Xapian::doccount termfreq = internal->termfreq;

    LOGLINE(WTCALC, "Statistics: N=" << internal->collection_size <<
		    " n_t=" << termfreq << " lenpart=" << lenpart);

    Xapian::weight tw = 0;
    if (internal->rset_size != 0) {
	Xapian::doccount rtermfreq = internal->reltermfreq;

	LOGLINE(WTCALC, " R=" << internal->rset_size << " r_t=" << rtermfreq);

	// termfreq must be at least rtermfreq since there are at least
	// rtermfreq documents indexed by this term.  And it can't be more than
	// (internal->collection_size - internal->rset_size + rtermfreq) since
	// the number of relevant documents not indexed by this term can't be
	// more than the number of documents not indexed by this term.
	Assert(termfreq >= rtermfreq);
	Assert(termfreq <= internal->collection_size - internal->rset_size + rtermfreq);

	tw = ((rtermfreq + 0.5) *
	      (internal->collection_size - internal->rset_size - termfreq + rtermfreq + 0.5)) /
	     ((internal->rset_size - rtermfreq + 0.5) *
	      (termfreq - rtermfreq + 0.5));
    } else {
	tw = (internal->collection_size - termfreq + 0.5) / (termfreq + 0.5);
    }

    Assert(tw > 0);

    if (tw < 2) {
	tw = tw / 2 + 1;
    }
    tw = log(tw);

    tw *= (k3 + 1) * wqf / (k3 + wqf);

    LOGLINE(WTCALC, " => termweight = " << tw);
    termweight = tw;
    weight_calculated = true;
}

Xapian::weight
BM25Weight::get_sumpart(Xapian::termcount wdf, Xapian::doclength len) const
{
    DEBUGCALL(MATCH, Xapian::weight, "BM25Weight::get_sumpart", wdf << ", " << len);
    if (!weight_calculated) calc_termweight();

    Xapian::doclength normlen = len * lenpart;
    if (normlen < min_normlen) normlen = min_normlen;

    double denom = k1 * (normlen * b + (1 - b)) + wdf;
    Xapian::weight wt;
    if (denom != 0) {
	wt = double(wdf) * (k1 + 1) / denom;
    } else {
	wt = 0;
    }
    LOGLINE(WTCALC, "(wdf,len,lenpart) = (" << wdf << "," << len << "," <<
		    lenpart << ") => wtadj = " << wt);

    wt *= termweight;

    LOGLINE(WTCALC, " => sumpart = " << wt);

    RETURN(wt);
}

Xapian::weight
BM25Weight::get_maxpart() const
{
    DEBUGCALL(MATCH, Xapian::weight, "BM25Weight::get_maxpart", "");
    if (!weight_calculated) calc_termweight();
    RETURN((k1 + 1) * termweight);
}

/* Should return k2 * querysize * (1-len) / (1+len)
 * However, want to return a positive value, so add (k2 * querysize) to
 * return.  ie: return 2 * k2 * querysize / (1 + len)
 */
Xapian::weight
BM25Weight::get_sumextra(Xapian::doclength len) const
{
    DEBUGCALL(MATCH, Xapian::weight, "BM25Weight::get_sumextra", len);
    if (!weight_calculated) calc_termweight();

    Xapian::doclength normlen = len * lenpart;
    if (normlen < min_normlen) normlen = min_normlen;
    Xapian::weight extra = 2 * k2 * querysize / (1 + normlen);
    LOGLINE(WTCALC, "len = " << len << " querysize = " << querysize <<
		    " => normlen = " << normlen << " => sumextra = " << extra);
    RETURN(extra);
}

Xapian::weight
BM25Weight::get_maxextra() const
{
    DEBUGCALL(MATCH, Xapian::weight, "BM25Weight::get_maxextra", "");
    Xapian::weight maxextra = 2 * k2 * querysize;
    LOGLINE(WTCALC, "querysize = " << querysize <<
		    " => maxextra = " << maxextra);
    RETURN(maxextra);
}

bool BM25Weight::get_sumpart_needs_doclength() const {
    if (!weight_calculated) calc_termweight();
    return (b != 0 && k1 != 0 && lenpart != 0);
}

}
