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

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>

////////////////////////////////////
// Initialisation and cleaning up //
////////////////////////////////////

NetworkMatch::NetworkMatch(IRDatabase *database_)
	: database(dynamic_cast<NetworkDatabase *>(database_)),
	  statssource(database->link),
	  max_weight_needs_fetch(true)
{
    // make sure that the database was a NetworkDatabase after all
    // (dynamic_cast<foo *> returns 0 if the cast fails)
    Assert(database != 0);

    database->link->register_statssource(&statssource);
}

bool
NetworkMatch::prepare_match(bool nowait)
{
    if (!is_prepared) {
	bool finished_query = database->link->finish_query();

	if (!finished_query) {
	    if (nowait) {
		return false;
	    } else {
		do {
		    database->link->wait_for_input();
		} while (!database->link->finish_query());
	    }
	}

	// Read the remote statistics and give them to the stats source
	//
	Stats mystats;
	bool read_remote_stats = database->link->get_remote_stats(mystats);
	if (!read_remote_stats) {
	    if (nowait) {
		return false;
	    } else {
		do {
		    database->link->wait_for_input();
		} while (!database->link->get_remote_stats(mystats));
	    }
	}
	statssource.take_remote_stats(mystats);

	is_prepared = true;
    }
    return true;
}

void
NetworkMatch::link_to_multi(StatsGatherer *gatherer_)
{
    gatherer = gatherer_;
    statssource.connect_to_gatherer(gatherer);
//    statsleaf.my_collection_size_is(database->get_doccount());
//    statsleaf.my_average_length_is(database->get_avlength());
}

NetworkMatch::~NetworkMatch()
{
}

/////////////////////////////////////////////////////////////////////
// Setting query options
//
void
NetworkMatch::set_options(const OmSettings & mopts)
{
    database->link->set_options(mopts);
    // weighting scheme has potentially changed
    max_weight_needs_fetch = true;
}

void
NetworkMatch::set_rset(const OmRSet & omrset)
{
    database->link->set_rset(omrset);
}

////////////////////////
// Building the query //
////////////////////////

void
NetworkMatch::set_query(const OmQueryInternal *query_)
{
    database->link->set_query(query_);
    max_weight_needs_fetch = true;
}

// Return the maximum possible weight, calculating it if necessary.
om_weight
NetworkMatch::get_max_weight()
{
    Assert(is_prepared);
    if (max_weight_needs_fetch) {
	max_weight = database->link->get_max_weight();
	max_weight_needs_fetch = false;
    }

    return max_weight;
}

///////////////////
// Run the query //
///////////////////

// This method is called by branch postlists when they rebalance
// in order to recalculate the weights in the tree
void
NetworkMatch::recalc_maxweight()
{
    Assert(false);
}

// This is the method which runs the query, generating the M set
bool
NetworkMatch::get_mset(om_doccount first,
		       om_doccount maxitems,
		       OmMSet &mset,
		       const OmMatchDecider *mdecider,
		       bool nowait)
{
    // FIXME: need to pass termfreqandwts to each link->get_mset() call and
    // to get the results back.
    Assert(is_prepared);

    if (mdecider != 0) {
	throw OmInvalidArgumentError("Can't use a match decider remotely");
    }

    database->link->send_global_stats(*(gatherer->get_stats()));

    bool finished = false;
    if (nowait) {
	finished = database->link->get_mset(first, maxitems,
					    mset);
    } else {
	finished = database->link->get_mset(first, maxitems,
					    mset);
	while (!finished) {
	    database->link->wait_for_input();
	    finished = database->link->get_mset(first, maxitems,
						mset);
	}
    }
    return finished;
}
