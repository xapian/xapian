/** @file expandweight.cc
 * @brief Calculate term weights for the ESet.
 */
/* Copyright (C) 2007,2008,2011 Olly Betts
 * Copyright (C) 2011 Action Without Borders
 * Copyright (C) 2013 Aarsh Shah
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include "expandweight.h"

#include "debuglog.h"
#include "omassert.h"
#include "api/termlist.h"

using namespace std;

namespace Xapian {
namespace Internal {

void
ExpandWeight::collect_stats(TermList * merger, const std::string & term)
{
    LOGCALL(MATCH, void, "ExpandWeight::collect_stats", merger | term);

    merger->accumulate_stats(stats);

    collection_freq = db.get_collection_freq(term);

    double termfreq = stats.termfreq;
    double rtermfreq = stats.rtermfreq;

    LOGVALUE(EXPAND, rsize);
    LOGVALUE(EXPAND, rtermfreq);
    LOGVALUE(EXPAND, dbsize);
    LOGVALUE(EXPAND, stats.dbsize);

    if (stats.dbsize == dbsize) {
	// Either we're expanding from just one database, or we got stats from
	// all the sub-databases (because at least one relevant document from
	// each sub-database contained this term), so termfreq should already
	// be exact.
	AssertEqParanoid(termfreq, db.get_termfreq(term));
    } else {
	AssertRel(stats.dbsize,<,dbsize);
	// We're expanding from more than one database and the stats we've got
	// only cover some of the sub-databases, so termfreq only includes
	// those sub-databases.
	if (use_exact_termfreq) {
	    LOGLINE(EXPAND, "Had to request exact termfreq");
	    termfreq = db.get_termfreq(term);
	} else {
	    // Approximate the termfreq by scaling it up from the databases we
	    // do have information from.
	    termfreq *= double(dbsize) / double(stats.dbsize);
	    LOGLINE(EXPAND, "termfreq is approx " << stats.termfreq << " * " <<
			    dbsize << " / " << stats.dbsize << " = " <<
			    termfreq);
	    LOGVALUE(EXPAND, db.get_termfreq(term));
	    if (termfreq < rtermfreq) {
		// termfreq must be at least rtermfreq, since there are at
		// least rtermfreq documents indexed by this term.
		LOGLINE(EXPAND, "termfreq must be at least rtermfreq");
		termfreq = rtermfreq;
	    } else {
		// termfreq can't be more than (dbsize - rsize + rtermfreq)
		// since the number of relevant documents not indexed by this
		// term can't be more than the number of documents not indexed
		// by this term, so:
		//
		//     rsize - rtermfreq <= dbsize - termfreq
		// <=> termfreq <= dbsize - (rsize - rtermfreq)
		double termfreq_upper_bound = dbsize - (rsize - rtermfreq);
		if (termfreq > termfreq_upper_bound) {
		    LOGLINE(EXPAND, "termfreq can't be more than "
				    "dbsize - (rsize + rtermfreq)");
		    termfreq = termfreq_upper_bound;
		}
	    }
	}
    }
    LOGVALUE(EXPAND, termfreq);
    stats.termfreq = termfreq;
    stats.rtermfreq = rtermfreq;
}

}
}
