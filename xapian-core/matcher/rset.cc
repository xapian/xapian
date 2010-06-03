/* rset.cc
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2007,2009 Olly Betts
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
#include "rset.h"

#include "database.h"
#include "stats.h"
#include "omassert.h"
#include "omdebug.h"

#include "autoptr.h"
#include "termlist.h"

void
RSetI::calculate_stats()
{
    DEBUGCALL(MATCH, void, "RSetI::calculate_stats", "");
    Assert(!calculated_reltermfreqs);
    std::set<Xapian::docid>::const_iterator doc;
    for (doc = documents.begin(); doc != documents.end(); doc++) {
	DEBUGLINE(WTCALC, "Counting reltermfreqs in document " << *doc << " [ ");
	if (dbroot) {
	    AutoPtr<TermList> tl(dbroot->open_term_list(*doc));
	    tl->next();
	    while (!tl->at_end()) {
		// FIXME - can this lookup be done faster?
		// Store termnames in a hash for each document, rather than
		// a list?
		string tname = tl->get_termname();
		if (reltermfreqs.find(tname) != reltermfreqs.end()) {
		    reltermfreqs[tname] ++;
		    DEBUGLINE(WTCALC, tname << " now has reltermfreq of " << reltermfreqs[tname]);
		}
		tl->next();
	    }
	} else {
	    Xapian::TermIterator tl = root.termlist_begin(*doc);
	    Xapian::TermIterator tlend = root.termlist_end(*doc);
	    while (tl != tlend) {
		// FIXME - can this lookup be done faster?
		// Store termnames in a hash for each document, rather than
		// a list?
		string tname = *tl;
		if (reltermfreqs.find(tname) != reltermfreqs.end()) {
		    reltermfreqs[tname] ++;
		    DEBUGLINE(WTCALC, tname << " now has reltermfreq of " << reltermfreqs[tname]);
		}
		tl++;
	    }
	}
	DEBUGLINE(WTCALC, "] ");
    }
    calculated_reltermfreqs = true;
}

void
RSetI::contribute_stats(Stats & stats)
{
    DEBUGCALL(MATCH, void, "RSetI::contribute_stats", stats);
    calculate_stats();

    std::map<string, Xapian::doccount>::const_iterator i;
    for (i = reltermfreqs.begin(); i != reltermfreqs.end(); i++) {
	stats.set_reltermfreq(i->first, i->second);
    }
    stats.rset_size += get_rsize();
}
