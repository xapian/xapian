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
#include "netserver.h"
#include "netclient.h"

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

NetworkStatsGatherer::NetworkStatsGatherer(NetServer *nserv_)
	: have_global_stats(false), nserv(nserv_)
{
}

const Stats *
NetworkStatsGatherer::get_stats() const
{
    fetch_local_stats();
    fetch_global_stats();

    return &total_stats;
}

void
NetworkStatsGatherer::fetch_local_stats() const
{
    if (!have_gathered) {
	for (vector<StatsSource *>::iterator i = sources.begin();
	     i != sources.end();
	     ++i) {
	    (*i)->contrib_my_stats();
	}
	have_gathered = true;
    }
}

Stats
NetworkStatsGatherer::get_local_stats() const
{
    fetch_local_stats();
    return total_stats;
}

void
NetworkStatsGatherer::fetch_global_stats() const
{
    Assert(have_gathered);

    //nserv->send_local_stats(total_stats);
    total_stats = nserv->get_global_stats();

    have_global_stats = true;
}

LocalStatsSource::LocalStatsSource()
{
}

LocalStatsSource::~LocalStatsSource()
{
}

NetworkStatsSource::NetworkStatsSource(OmRefCntPtr<NetClient> nclient_)
	: nclient(nclient_),
          have_remote_stats(false)
{
}

NetworkStatsSource::~NetworkStatsSource()
{
}

void
NetworkStatsSource::contrib_my_stats()
{
    Assert(have_remote_stats);
    gatherer->contrib_stats(my_stats);
}

void
NetworkStatsSource::take_remote_stats(Stats stats)
{
    my_stats = stats;
    have_remote_stats = true;
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

