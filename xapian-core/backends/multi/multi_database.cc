/* multi_database.cc: interface to multiple database access
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

#include <stdio.h>

#include "omassert.h"
#include "multi_postlist.h"
#include "multi_termlist.h"
#include "multi_database.h"
#include "database_builder.h"

#include <string>
#include <vector>
#include <list>

#include "om/omerror.h"

///////////////////////////
// Actual database class //
///////////////////////////

MultiDatabase::MultiDatabase(const DatabaseBuilderParams & params)
	: length_initialised(false)
{
    // Check validity of parameters
    if(params.readonly != true) {
	throw OmInvalidArgumentError("MultiDatabase must be opened readonly.");
    }

    if(params.paths.size() != 0) {
	throw OmInvalidArgumentError("MultiDatabase requires no path parameters.");
    }
    if(params.subdbs.size() <= 0) {
	throw OmInvalidArgumentError("MultiDatabase requires at least one sub-database.");
    }

    // Loop through all params in subdbs, creating a database for each one,
    // with the specified parameters.  Override some parameters though:
    // If params.readonly is set, everything should be opened readonly

    vector<DatabaseBuilderParams>::const_iterator p;
    for(p = params.subdbs.begin(); p != params.subdbs.end(); p++) {
	DatabaseBuilderParams sub_params = *p;
	if(params.readonly) sub_params.readonly = params.readonly;

	databases.push_back(DatabaseBuilder::create(sub_params));
    }
}

MultiDatabase::MultiDatabase(vector<OmRefCntPtr<IRDatabase> > databases_)
	: length_initialised(false)
{
    if(databases_.size() <= 0) {
	throw OmInvalidArgumentError("MultiDatabase requires at least one sub-database.");
    }

    databases = databases_;
}


MultiDatabase::~MultiDatabase()
{
    // Close all databases
}


om_doccount
MultiDatabase::get_doccount() const
{   
    om_doccount docs = 0;

    vector<OmRefCntPtr<IRDatabase> >::const_iterator i = databases.begin();
    while(i != databases.end()) {
	docs += (*i)->get_doccount();
	i++;
    }

    return docs;
}


om_doclength
MultiDatabase::get_avlength() const
{   
    if(!length_initialised) {
	om_doccount docs = 0;
	om_doclength totlen = 0;

	vector<OmRefCntPtr<IRDatabase> >::const_iterator i = databases.begin();
	while(i != databases.end()) {
	    om_doccount db_doccount = (*i)->get_doccount();
	    docs += db_doccount;
	    totlen += (*i)->get_avlength() * db_doccount;
	    i++;
	}

	avlength = totlen / docs;
	length_initialised = true;
    }

    return avlength;
}


om_doccount
MultiDatabase::get_termfreq(const om_termname & tname) const
{   
    if(!term_exists(tname)) return 0;
    PostList *pl = open_post_list(tname);
    om_doccount freq = 0;
    if(pl) freq = pl->get_termfreq();
    delete pl;
    return freq;
}


LeafPostList *
MultiDatabase::open_post_list(const om_termname & tname) const
{
    Assert(term_exists(tname));

    om_doccount offset = 1;
    om_doccount multiplier = databases.size();

    list<MultiPostListInternal> pls;
    vector<OmRefCntPtr<IRDatabase> >::const_iterator i = databases.begin();
    while(i != databases.end()) {
	if((*i)->term_exists(tname)) {
	    MultiPostListInternal pl((*i)->open_post_list(tname),
				     offset, multiplier);
	    pls.push_back(pl);
	}
	offset++;
	i++;
    }
    Assert(pls.begin() != pls.end());
    
    LeafPostList * newpl = new MultiPostList(pls, this);
    return newpl;
}

LeafTermList *
MultiDatabase::open_term_list(om_docid did) const {
    om_doccount multiplier = databases.size();

    om_docid realdid = (did - 1) / multiplier + 1;
    om_doccount dbnumber = (did - 1) % multiplier;

    TermList *newtl;
    newtl = (*(databases.begin() + dbnumber))->open_term_list(realdid);
    return new MultiTermList(newtl,
			     (databases.begin() + dbnumber)->get(),
			     this);
}

LeafDocument *
MultiDatabase::open_document(om_docid did) const
{
    om_doccount multiplier = databases.size();

    om_docid realdid = (did - 1) / multiplier + 1;
    om_doccount dbnumber = (did - 1) % multiplier;

    return (*(databases.begin() + dbnumber))->open_document(realdid);
}

om_doclength
MultiDatabase::get_doclength(om_docid did) const
{
    om_doccount multiplier = databases.size();

    om_docid realdid = (did - 1) / multiplier + 1;
    om_doccount dbnumber = (did - 1) % multiplier;

    return (*(databases.begin() + dbnumber))->get_doclength(realdid);
}

bool
MultiDatabase::term_exists(const om_termname & tname) const
{
    DebugMsg("MultiDatabase::term_exists(`" << tname.c_str() << "'): ");
    set<om_termname>::const_iterator p = terms.find(tname);

    bool found = false;

    if (p == terms.end()) {
	vector<OmRefCntPtr<IRDatabase> >::const_iterator i = databases.begin();
	while(i != databases.end()) {
	    found = (*i)->term_exists(tname);
	    if(found) break;
	    i++;
	}

	if(found) {
	    if (terms.size() > 500) {
		DebugMsg("(term cache full - clearing) " << endl);
		terms.clear();
	    }
	    DebugMsg("found in sub-database" << endl);
	    terms.insert(tname);
	} else {
	    DebugMsg("not in collection" << endl);
	}
    } else {
	found = true;
	DebugMsg("found in cache" << endl);
    }
    return found;
}
