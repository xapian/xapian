/* multi_database.cc: interface to multiple database access
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

#include <stdio.h>

#include "omdebug.h"
#include "multi_postlist.h"
#include "multi_termlist.h"
#include "multi_database.h"
#include "database_builder.h"

#include <vector>

#include "om/omerror.h"

///////////////////////////
// Actual database class //
///////////////////////////

MultiDatabase::MultiDatabase(const OmSettings & params)
{
    throw OmInvalidOperationError("MultiDatabase::MultiDatabase(const "
				  "OmSettings & params) not supported");
}

MultiDatabase::MultiDatabase(std::vector<RefCntPtr<Database> > databases_)
	: length_initialised(false)
{
    if (databases_.empty()) {
	throw OmInvalidArgumentError("MultiDatabase requires at least one sub-database.");
    }

    multiplier = databases_.size();
    databases = databases_;
}

MultiDatabase::~MultiDatabase()
{
    try {
	internal_end_session();
    } catch (...) {
	// Ignore any exceptions, since we may be being called due to an
	// exception anyway.  internal_end_session() should have already
	// been called, in the normal course of events.
    }
}

om_doccount
MultiDatabase::get_doccount() const
{
    om_doccount docs = 0;

    std::vector<RefCntPtr<Database> >::const_iterator i;
    for (i = databases.begin(); i != databases.end(); i++) {
	docs += (*i)->get_doccount();
    }
    return docs;
}

om_doclength
MultiDatabase::get_avlength() const
{
    if (!length_initialised) {
	om_doccount docs = 0;
	om_doclength totlen = 0;

	std::vector<RefCntPtr<Database> >::const_iterator i;
	for (i = databases.begin(); i != databases.end(); i++) {
	    om_doccount db_doccount = (*i)->get_doccount();
	    docs += db_doccount;
	    totlen += (*i)->get_avlength() * db_doccount;
	}

	avlength = totlen / docs;
	length_initialised = true;
    }

    return avlength;
}

om_doccount
MultiDatabase::get_termfreq(const om_termname & tname) const
{
    if (!term_exists(tname)) return 0;
    PostList *pl = open_post_list(tname);
    om_doccount freq = 0;
    if (pl) freq = pl->get_termfreq();
    delete pl;
    return freq;
}

LeafPostList *
MultiDatabase::do_open_post_list(const om_termname & tname) const
{
    Assert(term_exists(tname));
    
    std::vector<LeafPostList *> pls;
    try {
	std::vector<RefCntPtr<Database> >::const_iterator i;
	for (i = databases.begin(); i != databases.end(); i++) {
	    pls.push_back((*i)->open_post_list(tname));
	    pls.back()->next();
	}
	Assert(pls.begin() != pls.end());
    } catch (...) {
	std::vector<LeafPostList *>::iterator i;
	for (i = pls.begin(); i != pls.end(); i++) {
	    delete *i;
	    *i = 0;
	}
	throw;
    }

    return new MultiPostList(pls,
			     RefCntPtr<const MultiDatabase>(RefCntPtrToThis(), this));
}

LeafTermList *
MultiDatabase::open_term_list(om_docid did) const {
    om_docid realdid = (did - 1) / multiplier + 1;
    om_doccount dbnumber = (did - 1) % multiplier;

    TermList *newtl = databases[dbnumber]->open_term_list(realdid);
    return new MultiTermList(newtl, databases[dbnumber],
			     RefCntPtr<const Database>(RefCntPtrToThis(), this));
}

LeafDocument *
MultiDatabase::open_document(om_docid did) const
{
    om_docid realdid = (did - 1) / multiplier + 1;
    om_doccount dbnumber = (did - 1) % multiplier;

    return databases[dbnumber]->open_document(realdid);
}

om_doclength
MultiDatabase::get_doclength(om_docid did) const
{
    om_docid realdid = (did - 1) / multiplier + 1;
    om_doccount dbnumber = (did - 1) % multiplier;

    return databases[dbnumber]->get_doclength(realdid);
}

bool
MultiDatabase::term_exists(const om_termname & tname) const
{
    Assert(tname.size() != 0);
    std::vector<RefCntPtr<Database> >::const_iterator i;
    for (i = databases.begin(); i != databases.end(); i++) {
	if ((*i)->term_exists(tname)) return true;
    }
    return false;
}
