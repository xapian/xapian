/* sleepy_termlist.cc: Termlists in sleepycat databases
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

// Sleepycat database stuff
#include <db_cxx.h>

#include "utils.h"
#include "omdebug.h"
#include "sleepy_termlist.h"
#include "sleepy_termcache.h"
#include "sleepy_database_internals.h"
#include "sleepy_database.h"
#include <stdlib.h>

SleepyTermList::SleepyTermList(om_docid did_,
			       const SleepyDatabase * database_,
			       const SleepyDatabaseInternals * internals_,
			       const SleepyDatabaseTermCache *termcache_)
	: mylist(internals_->termlist_db,
		 reinterpret_cast<void *>(&did_),
		 sizeof(did_),
		 false),
	  database(database_),
	  termcache(termcache_),
	  db_size(database->get_doccount())
{
    norm_len = get_doclength() / database->get_avlength();
    mylist.move_to_start();
}

SleepyTermList::~SleepyTermList()
{
}

om_termcount
SleepyTermList::get_approx_size() const
{
    return mylist.get_item_count();
}

OmExpandBits
SleepyTermList::get_weighting() const
{
    Assert(wt != NULL);

    return wt->get_bits(SleepyTermList::get_wdf(),
			norm_len,
			SleepyTermList::get_termfreq(),
			db_size);
}

const om_termname
SleepyTermList::get_termname() const
{
    return termcache->term_id_to_name(mylist.get_current_item().id);
}

om_termcount
SleepyTermList::get_wdf() const
{
    om_termcount wdf = mylist.get_current_item().wdf;
    if(wdf == 0) {
	DebugMsg("WDF not present in termlist - using 1." << endl);
	wdf = 1;
    }
    return wdf;
}

om_doccount
SleepyTermList::get_termfreq() const
{
    om_doccount tf = mylist.get_current_item().termfreq;
    if(tf == 0) {
	DebugMsg("Term frequency not present in termlist - getting from database" << endl);
	tf = database->get_termfreq(SleepyTermList::get_termname());
    }
    return tf;
}   

TermList *
SleepyTermList::next()
{
    mylist.move_to_next_item();
    return NULL;
}

bool
SleepyTermList::at_end() const
{
    return mylist.at_end();
}

om_doclength
SleepyTermList::get_doclength() const
{
    om_doclength doclength = mylist.get_wdfsum();
    return doclength;
}
