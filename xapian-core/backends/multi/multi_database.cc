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

///////////////////////////
// Actual database class //
///////////////////////////

MultiDatabase::MultiDatabase()
	: length_initialised(false)
{
    Assert((opened = false) == false);
    Assert((used = false) == false);
}

MultiDatabase::~MultiDatabase() {
    // Close all databases
    while(databases.begin() != databases.end()) {
	delete *(databases.begin());
	databases.erase(databases.begin());
    }
}

void
MultiDatabase::set_root(IRDatabase * db) {
    Assert(!used);
    root = db;

    vector<IRDatabase *>::const_iterator i = databases.begin();
    while(i != databases.end()) {
	(*i)->set_root(db);
	i++;
    }
}

void
MultiDatabase::open(const DatabaseBuilderParams & params) {
    Assert(!used);

    // Check validity of parameters
    Assert(params.paths.size() == 0);
    Assert(params.subdbs.size() > 0);

    // Loop through all params in subdbs, creating a database for each one,
    // with the specified parameters.  Override some parameters though:
    // If params.readonly is set, everything should be opened readonly

    vector<DatabaseBuilderParams>::const_iterator p;
    for(p = params.subdbs.begin(); p != params.subdbs.end(); p++) {
	DatabaseBuilderParams sub_params = *p;
	if(params.readonly) sub_params.readonly = params.readonly;
	if(params.root != NULL) sub_params.root = params.root;

	databases.push_back(DatabaseBuilder::create(sub_params));
    }

    opened = true;
}

LeafPostList *
MultiDatabase::open_post_list(const om_termname & tname, RSet * rset) const
{
    Assert(opened);
    Assert((used = true) == true);
    Assert(term_exists(tname));

    om_doccount offset = 1;
    om_doccount multiplier = databases.size();

    list<MultiPostListInternal> pls;
    vector<IRDatabase *>::const_iterator i = databases.begin();
    while(i != databases.end()) {
	if((*i)->term_exists(tname)) {
	    MultiPostListInternal pl((*i)->open_post_list(tname, rset),
				     offset, multiplier);
	    pls.push_back(pl);
	}
	offset++;
	i++;
    }
    Assert(pls.begin() != pls.end());
    
    LeafPostList * newpl = new MultiPostList(pls);
    return newpl;
}

LeafTermList *
MultiDatabase::open_term_list(om_docid did) const {
    Assert(opened);
    Assert((used = true) == true);

    om_doccount multiplier = databases.size();

    om_docid realdid = (did - 1) / multiplier + 1;
    om_doccount dbnumber = (did - 1) % multiplier;

    TermList *newtl;
    newtl = (*(databases.begin() + dbnumber))->open_term_list(realdid);
    return new MultiTermList(newtl, *(databases.begin() + dbnumber), this);
}

LeafDocument *
MultiDatabase::open_document(om_docid did) const
{
    Assert(opened);
    Assert((used = true) == true);

    om_doccount multiplier = databases.size();

    om_docid realdid = (did - 1) / multiplier + 1;
    om_doccount dbnumber = (did - 1) % multiplier;

    return (*(databases.begin() + dbnumber))->open_document(realdid);
}

om_doclength
MultiDatabase::get_doclength(om_docid did) const
{
    Assert(opened);
    Assert((used = true) == true);

    om_doccount multiplier = databases.size();

    om_docid realdid = (did - 1) / multiplier + 1;
    om_doccount dbnumber = (did - 1) % multiplier;

    return (*(databases.begin() + dbnumber))->get_doclength(realdid);
}

bool
MultiDatabase::term_exists(const om_termname & tname) const
{
    Assert(opened);
    Assert((used = true) == true);

    //DebugMsg("MultiDatabase::term_exists(`" << tname.c_str() << "'): ");
    set<om_termname>::const_iterator p = terms.find(tname);

    bool found = false;

    if (p == terms.end()) {
	vector<IRDatabase *>::const_iterator i = databases.begin();
	while(i != databases.end()) {
	    found = (*i)->term_exists(tname);
	    if(found) break;
	    i++;
	}

	if(found) {
	    //DebugMsg("found in sub-database" << endl);
	    terms.insert(tname);
	} else {
	    //DebugMsg("not in collection" << endl);
	}
    } else {
	found = true;
	//DebugMsg("found in cache" << endl);
    }
    return found;
}
