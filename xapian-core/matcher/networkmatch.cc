/* networkmatch.cc
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

#include "config.h"
#include "networkmatch.h"

#include "stats.h"
#include "utils.h"

#include "msetpostlist.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>

RemoteSubMatch::RemoteSubMatch(const Database *db_,
			       const OmQueryInternal * query,
			       const OmRSet & omrset, const OmSettings &opts,
			       StatsGatherer *gatherer_)
	: is_prepared(false), db(dynamic_cast<const NetworkDatabase *>(db_)),
	  gatherer(gatherer_)
{	    
    // make sure that the database was a NetworkDatabase after all
    // (dynamic_cast<foo *> returns 0 if the cast fails)
    Assert(db);
    statssource = new NetworkStatsSource(gatherer_, db->link);

    db->link->set_query(query, opts, omrset);
    db->link->register_statssource(statssource);

    AutoPtr<RSet> new_rset(new RSet(db, omrset));
    rset = new_rset;

}

RemoteSubMatch::~RemoteSubMatch()
{
    delete statssource;
}

PostList *
RemoteSubMatch::get_postlist(om_doccount maxitems, MultiMatch *matcher)
//		       const OmMatchDecider *mdecider,
{
// FIXME: for efficiency, MatchDecider should probably be applied remotely
//    if (mdecider != 0) {
//	throw OmInvalidArgumentError("Can't use a match decider remotely");
//    }
    // FIXME: no longer needed
    db->link->send_global_stats(*(gatherer->get_stats()));
    postlist = new PendingMSetPostList(db, maxitems);
    return postlist;
}

bool
RemoteSubMatch::prepare_match(bool nowait)
{
    if (!is_prepared) {
	bool finished_query = db->link->finish_query();

	if (!finished_query) {
	    if (nowait) {
		return false;
	    } else {
		do {
		    db->link->wait_for_input();
		} while (!db->link->finish_query());
	    }
	}

	// Read the remote statistics and give them to the stats source
	//
	Stats mystats;
	bool read_remote_stats = db->link->get_remote_stats(mystats);
	if (!read_remote_stats) {
	    if (nowait) return false;
	    do {
		db->link->wait_for_input();
	    } while (!db->link->get_remote_stats(mystats));
	}
	statssource->take_remote_stats(mystats);

	is_prepared = true;
    }
    return true;
}

void
RemoteSubMatch::start_match(om_doccount maxitems)
{
    Assert(is_prepared);
    db->link->send_global_stats(*(gatherer->get_stats()));
    OmMSet mset;
    bool res = db->link->get_mset(0, maxitems, mset);
    // FIXME: improve this
    // link should always return false for first call to get_mset
    Assert(res == false);
}
