/* tradweight.cc: C++ class for weight calculation routines
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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

#include "database.h"
#include "tradweight.h"
#include "rset.h"
#include "omassert.h"

#include "config.h"

const double k = 1;

// Calculate weights using statistics retrieved from databases
void
TradWeight::calc_termweight() const
{
    Assert(initialised);

    om_doccount dbsize = root->get_doccount();
    lenpart = k / root->get_avlength();

    DebugMsg("Statistics: N=" << dbsize << " n_t=" << termfreq);

    om_weight tw = 0;
    om_doccount rsize;
    if(rset != NULL && (rsize = rset->get_rsize()) != 0) {
	om_doccount rtermfreq = rset->get_reltermfreq(tname);

	DebugMsg(" R=" << rsize << " r_t=" << rtermfreq);

	tw = (rtermfreq + 0.5) * (dbsize - rsize - termfreq + rtermfreq + 0.5) /
	     ((rsize - rtermfreq + 0.5) * (termfreq - rtermfreq + 0.5));
    } else {
	tw = (dbsize - termfreq + 0.5) / (termfreq + 0.5);
    }

    // FIXME This is to guarantee nice properties (monotonic increase) of the
    // weighting function.
    // Check whether this actually helps / whether it hinders efficiency
    if (tw < 2) {
	// if size and/or termfreq is estimated we can get tw <= 0
	// so handle this gracefully
	if (tw <= 1e-6) tw = 1e-6;
	tw = tw / 2 + 1;
    }
    tw = log(tw);

    DebugMsg(" => termweight = " << tw << endl);
    termweight = tw;
    weight_calculated = true;
}

om_weight
TradWeight::get_sumpart(om_doccount wdf, om_doclength len) const
{
    if(!weight_calculated) calc_termweight();

    om_weight wt = (double) wdf / (len * lenpart + wdf);

    wt *= termweight;

    return wt;
}

om_weight
TradWeight::get_maxpart() const
{
    if(!weight_calculated) calc_termweight();

    return termweight;
}

om_weight
TradWeight::get_sumextra(om_doclength len) const
{
    return 0;
}

om_weight
TradWeight::get_maxextra() const
{
    return 0;
}
