/* sleepy_postlist.cc: Access to postlists stored in sleepycat databases
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


#include <stdlib.h>
#include "om/omtypes.h"
#include "omassert.h"
#include "utils.h"
#include "sleepy_postlist.h"
#include "sleepy_database_internals.h"

///////////////
// Postlists //
///////////////

SleepyPostList::SleepyPostList(om_termid tid_,
			       SleepyDatabaseInternals * internals_,
			       const om_termname & tname_)
	: tname(tname_),
	  mylist(internals_->postlist_db,
		 reinterpret_cast<void *>(&tid_),
		 sizeof(tid_))
{
}

SleepyPostList::~SleepyPostList()
{
}


om_doccount
SleepyPostList::get_termfreq() const
{
    return mylist.get_item_count();
}

om_docid
SleepyPostList::get_docid() const
{
#if 0
    Assert(!at_end());
    Assert(pos != 0);
    return data[pos - 1];
#endif
}

om_weight SleepyPostList::get_weight() const
{
#if 0
    Assert(!at_end());
    Assert(ir_wt != NULL);
    
    om_doccount wdf = 1;

    return ir_wt->get_sumpart(wdf, 1.0);
#endif
}

PostList *
SleepyPostList::next(om_weight w_min)
{
#if 0
    Assert(!at_end());
    pos ++;
    return NULL;
#endif
}

PostList *
SleepyPostList::skip_to(om_docid did, om_weight w_min)
{
#if 0
    Assert(!at_end());
    if(pos == 0) pos++;
    while (!at_end() && data[pos - 1] < did) {
	PostList *ret = next(w_min);
	if (ret) return ret;
    }
    return NULL;
#endif
}

bool
SleepyPostList::at_end() const
{
#if 0
    if(pos > termfreq) return true;
    return false;
#endif
}

string
SleepyPostList::intro_term_description() const
{
    return tname + ":" + inttostring(get_termfreq());
}
