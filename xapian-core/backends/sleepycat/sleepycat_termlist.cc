/* sleepy_termlist.cc: Termlists in sleepycat databases
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

// Sleepycat database stuff
#include <db_cxx.h>

#include "utils.h"
#include "omassert.h"
#include "sleepy_termlist.h"
#include "sleepy_termcache.h"
#include <stdlib.h>

SleepyTermList::SleepyTermList(const SleepyDatabaseTermCache *tc_new,
			       om_termid *data_new,
			       om_termcount terms_new,
			       om_doccount dbsize_new)
	: pos(0),
	  data(data_new),
	  terms(terms_new),
	  dbsize(dbsize_new),
	  termcache(tc_new)
{ return; }

SleepyTermList::~SleepyTermList() {
    free(data);
}

om_termcount
SleepyTermList::get_approx_size() const
{
    return terms;
}

OmExpandBits
SleepyTermList::get_weighting() const
{
    Assert(!at_end());
    Assert(pos != 0);
    Assert(wt != NULL);

    om_termcount wdf = 1; // FIXME - not yet stored in data structure
    om_doclength norm_len = 1.0; // FIXME - not yet stored in data structure

    return wt->get_bits(wdf, norm_len, SleepyTermList::get_termfreq(), dbsize);
}

const om_termname
SleepyTermList::get_termname() const
{
    Assert(!at_end());
    Assert(pos != 0);
    return termcache->term_id_to_name(data[pos]);
}

om_termcount
SleepyTermList::get_wdf() const
{
    Assert(!at_end());
    Assert(pos != 0);
    return 1;
}

om_doccount
SleepyTermList::get_termfreq() const
{
    Assert(!at_end());
    Assert(pos != 0);
    return 1;
}   

TermList *
SleepyTermList::next()
{
    Assert(!at_end());
    pos ++;
    return NULL;
}

bool
SleepyTermList::at_end() const
{
    if(pos > terms) return true;
    return false;
}

