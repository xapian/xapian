/* stats.cc
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

#include "stats.h"
#include "omassert.h"

void
StatsGatherer::add_child(StatsSource *source) {
    sources.push_back(source);
}

void
StatsGatherer::contrib_stats(const Stats & extra_stats)
{
    Assert(have_gathered == false);
    total_stats += extra_stats;
}

const Stats *
LocalStatsGatherer::get_stats() const
{
    if(!have_gathered) {
	for (vector<StatsSource *>::iterator i = sources.begin();
	     i != sources.end();
	     ++i) {
	    (*i)->contrib_my_stats();
	}
	have_gathered = true;
    }

    return (&total_stats);
}

const Stats *
NetworkStatsGatherer::get_stats() const
{
    if (!have_gathered) {
	for (vector<StatsSource *>::iterator i = sources.begin();
	     i != sources.end();
	     ++i) {
	    (*i)->contrib_my_stats();
	}
	have_gathered = true;
    }
    // FIXME: wait until the global stats have arrived
    Assert(have_global_stats);

    return &total_stats;
}

void
NetworkStatsGatherer::set_global_stats(Stats stats)
{
    total_stats = stats;
    have_global_stats = true;
}

void
StatsSource::perform_request() const
{
    Assert(total_stats == 0);
    total_stats = gatherer->get_stats();
    Assert(total_stats != 0);

#ifdef MUS_DEBUG_VERBOSE
    DebugMsg("StatsSource::perform_request(): stats are:" << endl);
    DebugMsg("  collection_size = " << total_stats->collection_size << endl);
    DebugMsg("  rset_size = "       << total_stats->rset_size << endl);
    DebugMsg("  average_length = "  << total_stats->average_length << endl);

    map<om_termname, om_doccount>::const_iterator i;
    for(i = total_stats->termfreq.begin();
	i != total_stats->termfreq.end(); i++)
    {
	DebugMsg("  termfreq of `" << i->first <<
		 "'\tis " << i->second << endl);
    }
    for(i = total_stats->reltermfreq.begin();
	i != total_stats->reltermfreq.end(); i++)
    {
	DebugMsg("  reltermfreq of `" << i->first <<
		 "'\tis " << i->second << endl);
    }
#endif /* MUS_DEBUG_VERBOSE */
}

