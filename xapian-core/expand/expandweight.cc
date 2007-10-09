/* expandweight.cc: C++ class for weight calculation routines
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2004,2007 Olly Betts
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

#include "expandweight.h"
#include "termlist.h"

#include "omassert.h"
#include "omdebug.h"

namespace Xapian {
namespace Internal {

ExpandWeight::ExpandWeight(const Xapian::Database &db_,
			   Xapian::doccount rsize_,
			   bool use_exact_termfreq_,
			   double expand_k_)
	: db(db_),
	  rsize(rsize_),
	  use_exact_termfreq(use_exact_termfreq_),
	  expand_k(expand_k_)
{
    DEBUGCALL(MATCH, void, "ExpandWeight", db_ << ", " << rsize_ << ", " << use_exact_termfreq_ << ", " << expand_k_);
}

Xapian::weight
ExpandWeight::get_weight(TermList * merger, const string &tname) const
{
    DEBUGCALL(MATCH, Xapian::weight, "ExpandWeight::get_weight", "[merger], " << tname);
    ExpandStats stats(db.get_avlength(), expand_k);
    merger->accumulate_stats(stats);
    double termfreq = double(stats.termfreq);
    const double rtermfreq = stats.rtermfreq;

    Xapian::doccount dbsize = db.get_doccount();
    if (stats.dbsize != dbsize) {
	if (!use_exact_termfreq) {
	    termfreq *= double(dbsize) / stats.dbsize;
	    DEBUGLINE(EXPAND, "Approximating termfreq of `" << tname << "': " <<
		      stats.termfreq << " * " << dbsize << " / " <<
		      stats.dbsize << " = " << termfreq << " (true value is:" <<
		      db.get_termfreq(tname) << ")");
	    // termfreq must be at least rtermfreq since there are at least
	    // rtermfreq documents indexed by this term.  And it can't be
	    // more than (dbsize - rsize + rtermfreq) since the number
	    // of relevant documents not indexed by this term can't be
	    // more than the number of documents not indexed by this term.
	    if (termfreq < rtermfreq) {
		termfreq = rtermfreq;
	    } else {
		const double upper_bound = dbsize - rsize + rtermfreq;
		if (termfreq > upper_bound) termfreq = upper_bound;
	    }
	} else {
	    termfreq = db.get_termfreq(tname);
	    DEBUGLINE(EXPAND, "Asked database for termfreq of `" << tname <<
		      "': " << termfreq);
	}
    }

    DEBUGMSG(EXPAND, "ExpandWeight::get_weight: "
	     "N=" << dbsize << ", "
	     "n=" << termfreq << ", "
	     "R=" << rsize << ", "
	     "r=" << rtermfreq << ", "
	     "mult=" << stats.multiplier);

    Xapian::weight tw;
    tw = (rtermfreq + 0.5) * (dbsize - rsize - termfreq + rtermfreq + 0.5) /
	    ((rsize - rtermfreq + 0.5) * (termfreq - rtermfreq + 0.5));
    Assert(tw > 0);

    // FIXME This is to guarantee nice properties (monotonic increase) of the
    // weighting function.  Actually, I think the important point is that
    // it ensures that tw is positive.
    // Check whether this actually helps / whether it hinders efficiency
    if (tw < 2) {
	tw = tw / 2 + 1;
    }
    tw = log(tw);

    DEBUGLINE(EXPAND, " => Term weight = " << tw <<
	      " Expand weight = " << stats.multiplier * tw);

    //RETURN(rtermfreq * tw);
    RETURN(stats.multiplier * tw);
}

}
}
