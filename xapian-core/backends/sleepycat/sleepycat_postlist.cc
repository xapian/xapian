/* sleepycat_postlist.cc: Access to postlists stored in sleepycat databases
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


#include <stdlib.h>
#include "om/omtypes.h"
#include "omdebug.h"
#include "utils.h"
#include "sleepycat_postlist.h"
#include "sleepycat_database_internals.h"

///////////////
// Postlists //
///////////////

SleepycatPostList::SleepycatPostList(om_termid tid_,
				     SleepycatDatabaseInternals * internals_,
				     const om_termname & tname_,
				     OmRefCntPtr<const SleepycatDatabase> this_db_)
	: tname(tname_),
	  mylist(internals_->postlist_db,
		 reinterpret_cast<void *>(&tid_),
		 sizeof(tid_), true),
	  this_db(this_db_)
{
    mylist.move_to_start();
}

SleepycatPostList::~SleepycatPostList()
{
}


om_doccount
SleepycatPostList::get_termfreq() const
{
    return mylist.get_item_count();
}

om_docid
SleepycatPostList::get_docid() const
{
    return mylist.get_current_item().id;
}

om_doclength
SleepycatPostList::get_doclength() const
{
    return mylist.get_current_item().doclength;
}

om_weight
SleepycatPostList::get_weight() const
{
    Assert(ir_wt != NULL);

    om_termcount wdf = mylist.get_current_item().wdf;
    if(wdf == 0) {
	DEBUGLINE(DB, "WDF not present in postlist - using 1.");
	wdf = 1;
    }

    return ir_wt->get_sumpart(wdf, get_doclength());
}

om_termcount
SleepycatPostList::get_wdf() const
{
    om_termcount wdf = mylist.get_current_item().wdf;
    if (wdf == 0) {
	DEBUGLINE(DB, "WDF not present in postlist - using 1.");
	wdf = 1;
    }
    return wdf;
}

PositionList *
SleepycatPostList::get_position_list()
{
    mypositions.set_data(mylist.get_current_item().positions);
    return &mypositions;
}

PostList *
SleepycatPostList::next(om_weight w_min)
{
    mylist.move_to_next_item();
    return NULL;
}

PostList *
SleepycatPostList::skip_to(om_docid did, om_weight w_min)
{
    mylist.skip_to_item(did);
    return NULL;
}

bool
SleepycatPostList::at_end() const
{
    return mylist.at_end();
}

std::string
SleepycatPostList::intro_term_description() const
{
    return tname + ":" + om_tostring(get_termfreq());
}
