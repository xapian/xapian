/* bm25weight.cc: C++ class for weight calculation routines
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#include <math.h>
#include "config.h"

#include "omdebug.h"
#include "bm25weight.h"
#include "stats.h"

///////////////////////////////////////////////////////////////////////////

// const double A = 1; // used with wqf (which we don't do yet)
const double B = 1;
const double D = .5;
const double C = .5;

// The following parameters cause BM25Weight to behave identically to
// TradWeight.
//const double A = 1;
//const double B = 1;
//const double D = 1;
//const double C = 0;

// Calculate weights using statistics retrieved from databases
void
BM25Weight::calc_termweight() const
{
    Assert(initialised);
    Assert(stats->get_total_average_length() != 0);

    om_doccount dbsize = stats->get_total_collection_size();
    lenpart = B * D / stats->get_total_average_length();

    om_doccount termfreq = stats->get_total_termfreq(tname);

    DEBUGMSG(WTCALC, "Statistics: N=" << dbsize << " n_t=" << termfreq);

    om_weight tw = 0;
    om_doccount rsize = stats->get_total_rset_size();
    if (rsize != 0) {
	om_doccount rtermfreq = stats->get_total_reltermfreq(tname);

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

    DEBUGLINE(WTCALC, " => termweight = " << tw);
    termweight = tw;
    weight_calculated = true;
}

om_weight
BM25Weight::get_sumpart(om_termcount wdf, om_doclength len) const
{
    if(!weight_calculated) calc_termweight();

    om_weight wt = (double) wdf / (len * lenpart + B * (1 - D) + wdf);
    DEBUGMSG(WTCALC, "(wdf,len,lenpart) = (" << wdf << "," << len << "," <<
	     lenpart << ") =>  wtadj = " << wt);

    wt *= termweight;

    DEBUGLINE(WTCALC, " =>  sumpart = " << wt);

    return wt;
}

om_weight
BM25Weight::get_maxpart() const
{
    if(!weight_calculated) calc_termweight();
    DEBUGLINE(WTCALC, "maxpart = " << termweight);
    return termweight;
}

/* Should return C * querysize * (1-len) / (1+len)
 * However, want to return a positive value, so add (C * querysize) to return.
 * ie: return C * querysize / (1 + len)  (factor of 2 is incorporated into C)
 */
om_weight
BM25Weight::get_sumextra(om_doclength len) const
{
    om_doclength normlen = len / stats->get_total_average_length();
    om_weight extra = C * querysize / (1 + normlen);
    DEBUGLINE(WTCALC, "len = " << len <<
	      " querysize = " << querysize <<
	      " =>  normlen = " << normlen <<
	      " =>  sumextra = " << extra);
    return extra;
}

om_weight
BM25Weight::get_maxextra() const
{
    om_weight maxextra = C * querysize;
    DEBUGLINE(WTCALC, "querysize = " << querysize <<
	      " =>  maxextra = " << maxextra);
    return maxextra;
}
