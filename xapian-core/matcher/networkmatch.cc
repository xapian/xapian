/* networkmatch.cc
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
	  max_weight_needs_fetch(true) /*,
	  wt_type(IRWeight::WTTYPE_BM25),
	  do_collapse(false) */
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
	    };
	};
	statssource.take_remote_stats(mystats);

	is_prepared = true;
    }
    return true;
}

#if 0
void
NetworkMatch::finish_query()
{
    database->link->finish_query();
}
#endif

void
NetworkMatch::link_to_multi(StatsGatherer *gatherer_)
{
    gatherer = gatherer_;
    statssource.connect_to_gatherer(gatherer);
//    statsleaf.my_collection_size_is(database->get_doccount());
//    statsleaf.my_average_length_is(database->get_avlength());
}

//////////////////////////////////////////////////////////
// ########## PAST THIS POINT IMPLEMENTATION ########## //
// ########## IS JUST COPIED FROM LEAFMATCH, ########## //
// ########## AND IS HENCE BOGUS TO THE MAX. ########## //
//////////////////////////////////////////////////////////

NetworkMatch::~NetworkMatch()
{
#if 0
    del_query_tree();
#endif
}

#if 0
LeafPostList *
NetworkMatch::mk_postlist(const om_termname& tname, RSet * rset)
{
    // FIXME - this should be centralised into a postlist factory
    LeafPostList * pl = database->open_post_list(tname, rset);
    if(rset) rset->will_want_termfreq(tname);

    IRWeight * wt = mk_weight(1, tname, rset);
    statsleaf.my_termfreq_is(tname, pl->get_termfreq());
    // Query size of 1 for now.  FIXME
    pl->set_termweight(wt);
    return pl;
}
#endif


#if 0
void
NetworkMatch::mk_extra_weight()
{
    if(extra_weight == 0) {
	extra_weight = mk_weight(1, "", rset);
    }
}
#endif

#if 0
IRWeight *
NetworkMatch::mk_weight(om_doclength querysize_,
		     om_termname tname_,
		     const RSet * rset_)
{
    IRWeight * wt = IRWeight::create(wt_type);
    //IRWeight * wt = new TradWeight();
    weights.push_back(wt); // Remember it for deleting
    wt->set_stats(&statsleaf, querysize_, tname_, rset_);
    return wt;
}
#endif

#if 0
void
NetworkMatch::del_query_tree()
{
    delete query;
    query = 0;

    extra_weight = 0;
    while (!weights.empty()) {
	delete(weights.back());
	weights.pop_back();
    }
}
#endif

/////////////////////////////////////////////////////////////////////
// Setting query options
//
void
NetworkMatch::set_options(const OmMatchOptions & moptions_)
{
    database->link->set_options(moptions_);
}

void
NetworkMatch::set_rset(RSet *rset_)
{
    Assert(false);
#if 0
    Assert(query == NULL);
    rset = rset_;
    del_query_tree();
#endif
}

void
NetworkMatch::set_weighting(IRWeight::weight_type wt_type_)
{
    database->link->set_weighting(wt_type_);
    max_weight_needs_fetch = true;
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
#if 0
    recalculate_maxweight = true;
#endif
}

// This is the method which runs the query, generating the M set
bool
NetworkMatch::get_mset(om_doccount first,
		       om_doccount maxitems,
		       vector<OmMSetItem> & mset,
		       om_doccount * mbound,
		       om_weight * greatest_wt,
		       const OmMatchDecider *mdecider,
		       bool nowait)
{
    Assert(is_prepared);

    if (mdecider != 0) {
	throw OmInvalidArgumentError("Can't use a match decider remotely");
    }

    database->link->send_global_stats(*(gatherer->get_stats()));

    bool finished = false;
    if (nowait) {
	finished = database->link->get_mset(first, maxitems,
					    mset, mbound, greatest_wt);
    } else {
	finished = database->link->get_mset(first, maxitems,
					    mset, mbound, greatest_wt);
	while (!finished) {
	    database->link->wait_for_input();
	    finished = database->link->get_mset(first, maxitems,
						mset, mbound, greatest_wt);
	};
    }
    return finished;
}
