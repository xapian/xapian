/* bm25weight.cc: C++ class for weight calculation routines
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004 Olly Betts
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
    return new BM25Weight(A, B, C, D, min_normlen);
}

std::string BM25Weight::name() const { return "BM25"; }

string BM25Weight::serialise() const {
    return om_tostring(A) + ' ' + om_tostring(B) + ' ' +
	   om_tostring(C) + ' ' + om_tostring(D) + ' ' +
	   om_tostring(min_normlen);
}

Weight * BM25Weight::unserialise(const string & s) const {
    // We never actually modify through p, but strtod takes a char **
    // as the second parameter and we can't pass &p if p is const char *
    // (sigh)
    char *p = const_cast<char *>(s.c_str());
    double A, B, C, D;
    A = strtod(p, &p);
    B = strtod(p, &p);
    C = strtod(p, &p);
    D = strtod(p, &p);
    return new BM25Weight(A, B, C, D, strtod(p, NULL));
}

// Calculate weights using statistics retrieved from databases
void
BM25Weight::calc_termweight() const
{
    DEBUGCALL(MATCH, void, "BM25Weight::calc_termweight", "");

    Xapian::doccount dbsize = internal->get_total_collection_size();
    BD = B * D;
    lenpart = internal->get_total_average_length();

    // Just to ensure okay behaviour: should only happen if no data
    // (though there could be empty documents).
    if (lenpart == 0) lenpart = 1;

    lenpart = 1 / lenpart;

    Xapian::doccount termfreq = internal->get_total_termfreq(tname);

    DEBUGMSG(WTCALC, "Statistics: N=" << dbsize << " n_t=" << termfreq);

    Xapian::weight tw = 0;
    Xapian::doccount rsize = internal->get_total_rset_size();
    if (rsize != 0) {
	Xapian::doccount rtermfreq = internal->get_total_reltermfreq(tname);

	DEBUGMSG(WTCALC, " R=" << rsize << " r_t=" << rtermfreq);

	tw = (rtermfreq + 0.5) * (dbsize - rsize - termfreq + rtermfreq + 0.5) /
	     ((rsize - rtermfreq + 0.5) * (termfreq - rtermfreq + 0.5));
    } else {
	tw = (dbsize - termfreq + 0.5) / (termfreq + 0.5);
    }
    if (tw < 2) {
	// if size and/or termfreq is estimated we can get tw <= 0
	// so handle this gracefully
	if (tw <= 1e-6) tw = 1e-6;
	tw = tw / 2 + 1;
    }
    tw = log(tw);
    
    tw *= (A + 1) * wqf / (A + wqf);

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

    double denom = (normlen * BD + B * (1 - D) + wdf);
    Xapian::weight wt;
    if (denom != 0) {
	wt = double(wdf) * (B + 1) / denom;
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
    DEBUGLINE(WTCALC, "maxpart = " << ((B + 1) * termweight));
    RETURN((B + 1) * termweight);
}

/* Should return C * querysize * (1-len) / (1+len)
 * However, want to return a positive value, so add (C * querysize) to
 * return.  ie: return C * querysize / (1 + len)  (factor of 2 is
 * incorporated into C)
 */
Xapian::weight
BM25Weight::get_sumextra(Xapian::doclength len) const
{
    DEBUGCALL(MATCH, Xapian::weight, "BM25Weight::get_sumextra", len);
    Xapian::doclength normlen = len * lenpart;
    if (normlen < min_normlen) normlen = min_normlen;
    Xapian::weight extra = C * querysize / (1 + normlen);
    DEBUGLINE(WTCALC, "len = " << len << " querysize = " << querysize <<
	      " => normlen = " << normlen << " => sumextra = " << extra);
    RETURN(extra);
}

Xapian::weight
BM25Weight::get_maxextra() const
{
    DEBUGCALL(MATCH, Xapian::weight, "BM25Weight::get_maxextra", "");
    Xapian::weight maxextra = C * querysize;
    DEBUGLINE(WTCALC, "querysize = " << querysize <<
	      " => maxextra = " << maxextra);
    RETURN(maxextra);
}

bool BM25Weight::get_sumpart_needs_doclength() const { return (lenpart != 0); }

}
