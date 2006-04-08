/* bm25weight.cc: Class for BM25 weight calculation
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005 Olly Betts
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include <config.h>

#include <math.h>

#include "omdebug.h"
#include <xapian/enquire.h>
#include "stats.h"

namespace Xapian {

BM25Weight * BM25Weight::clone() const {
    return new BM25Weight(k1, k2, k3, b, min_normlen);
}

std::string BM25Weight::name() const { return "BM25"; }

string BM25Weight::serialise() const {
    return om_tostring(k1) + ' ' + om_tostring(k2) + ' ' +
	   om_tostring(k3) + ' ' + om_tostring(b) + ' ' +
	   om_tostring(min_normlen);
}

BM25Weight * BM25Weight::unserialise(const string & s) const {
    // We never actually modify through p, but strtod takes a char **
    // as the second parameter and we can't pass &p if p is const char *
    // (sigh)
    char *p = const_cast<char *>(s.c_str());
    double k1_, k2_, k3_, b_;
    k1_ = strtod(p, &p);
    k2_ = strtod(p, &p);
    k3_ = strtod(p, &p);
    b_ = strtod(p, &p);
    return new BM25Weight(k1_, k2_, k3_, b_, strtod(p, NULL));
}

// Calculate weights using statistics retrieved from databases
void
BM25Weight::calc_termweight() const
{
    DEBUGCALL(MATCH, void, "BM25Weight::calc_termweight", "");

    Xapian::doccount dbsize = internal->get_total_collection_size();
    lenpart = internal->get_total_average_length();

    // lenpart == 0 if there are no documents, or only empty documents.
    if (lenpart != 0) lenpart = 1 / lenpart;

    Xapian::doccount termfreq = internal->get_total_termfreq(tname);

    DEBUGMSG(WTCALC, "Statistics: N=" << dbsize << " n_t=" << termfreq);

    Xapian::weight tw = 0;
    Xapian::doccount rsize = internal->get_total_rset_size();
    if (rsize != 0) {
	Xapian::doccount rtermfreq = internal->get_total_reltermfreq(tname);

	DEBUGMSG(WTCALC, " R=" << rsize << " r_t=" << rtermfreq);

	// termfreq must be at least rtermfreq since there are at least
	// rtermfreq documents indexed by this term.  And it can't be
	// more than (dbsize - rsize + rtermfreq) since the number
	// of releveant documents not indexed by this term can't be
	// more than the number of documents not indexed by this term.
	Assert(termfreq >= rtermfreq);
	Assert(termfreq <= dbsize - rsize + rtermfreq);

	tw = (rtermfreq + 0.5) * (dbsize - rsize - termfreq + rtermfreq + 0.5) /
	     ((rsize - rtermfreq + 0.5) * (termfreq - rtermfreq + 0.5));
    } else {
	tw = (dbsize - termfreq + 0.5) / (termfreq + 0.5);
    }

    Assert(tw > 0);

    if (tw < 2) {
	tw = tw / 2 + 1;
    }
    tw = log(tw);
    
    tw *= (k3 + 1) * wqf / (k3 + wqf);

    DEBUGLINE(WTCALC, " => termweight = " << tw);
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
    DEBUGMSG(WTCALC, "(wdf,len,lenpart) = (" << wdf << "," << len << "," <<
	     lenpart << ") => wtadj = " << wt);

    wt *= termweight;

    DEBUGLINE(WTCALC, " => sumpart = " << wt);

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
    Xapian::doclength normlen = len * lenpart;
    if (normlen < min_normlen) normlen = min_normlen;
    Xapian::weight extra = 2 * k2 * querysize / (1 + normlen);
    DEBUGLINE(WTCALC, "len = " << len << " querysize = " << querysize <<
	      " => normlen = " << normlen << " => sumextra = " << extra);
    RETURN(extra);
}

Xapian::weight
BM25Weight::get_maxextra() const
{
    DEBUGCALL(MATCH, Xapian::weight, "BM25Weight::get_maxextra", "");
    Xapian::weight maxextra = 2 * k2 * querysize;
    DEBUGLINE(WTCALC, "querysize = " << querysize <<
	      " => maxextra = " << maxextra);
    RETURN(maxextra);
}

bool BM25Weight::get_sumpart_needs_doclength() const { 
    return (b != 0 && k1 != 0 && lenpart != 0);
}

}
