/* tradweight.cc: C++ class for weight calculation routines
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2006,2008 Olly Betts
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

TradWeight * TradWeight::clone() const {
    return new TradWeight(param_k);
}

string TradWeight::name() const { return "Trad"; }

string TradWeight::serialise() const {
    return serialise_double(param_k);
}

TradWeight * TradWeight::unserialise(const string & s) const {
    const char *p = s.data();
    const char *p_end = p + s.size();
    double param_k_ = unserialise_double(&p, p_end);
    // FIXME: should check that (p == p_end).
    return new TradWeight(param_k_);
}

// Calculate weights using statistics retrieved from databases
void
TradWeight::calc_termweight() const
{
    DEBUGCALL(MATCH, void, "TradWeight::calc_termweight", "");

    lenpart = internal->average_length;
    // lenpart == 0 if there are no documents, or only empty documents.
    if (lenpart != 0) lenpart = param_k / lenpart;

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

    // FIXME This is to guarantee nice properties (monotonic increase) of the
    // weighting function.  Actually, I think the important point is that
    // it ensures that tw is positive.
    // Check whether this actually helps / whether it hinders efficiency
    if (tw < 2) {
	tw = tw / 2 + 1;
    }

    termweight = log(tw);
    LOGVALUE(WTCALC, termweight);
    weight_calculated = true;
}

Xapian::weight
TradWeight::get_sumpart(Xapian::termcount wdf, Xapian::doclength len) const
{
    DEBUGCALL(MATCH, Xapian::weight, "TradWeight::get_sumpart", wdf << ", " << len);
    if (!weight_calculated) calc_termweight();

    Xapian::weight wt = double(wdf) / (len * lenpart + wdf);

    wt *= termweight;

    RETURN(wt);
}

Xapian::weight
TradWeight::get_maxpart() const
{
    DEBUGCALL(MATCH, Xapian::weight, "TradWeight::get_maxpart", "");
    if (!weight_calculated) calc_termweight();

    RETURN(termweight);
}

Xapian::weight
TradWeight::get_sumextra(Xapian::doclength /*len*/) const
{
    DEBUGCALL(MATCH, Xapian::weight, "TradWeight::get_sumextra", "/*len*/");
    RETURN(0);
}

Xapian::weight
TradWeight::get_maxextra() const
{
    DEBUGCALL(MATCH, Xapian::weight, "TradWeight::get_maxextra", "");
    RETURN(0);
}

bool TradWeight::get_sumpart_needs_doclength() const {
    if (!weight_calculated) calc_termweight();
    return (lenpart != 0);
}

}
