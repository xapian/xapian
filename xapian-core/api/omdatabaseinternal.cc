/* omdatabaseinternal.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003 Olly Betts
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

#include <config.h>
#include "omdatabaseinternal.h"
#include "alltermslist.h"
#include "emptyalltermslist.h"
#include "multialltermslist.h"

#include "../backends/multi/multi_postlist.h"
#include "../backends/multi/multi_termlist.h"

#include "omdebug.h"
#include <vector>

using namespace std;

OmDatabase::Internal::Internal(Database *db)
{
    add_database(db);
}

void
OmDatabase::Internal::add_database(Database *db)
{
    RefCntPtr<Database> newdb(db);
    databases.push_back(newdb);
}

void
OmDatabase::Internal::add_database(RefCntPtr<Database> newdb)
{
    databases.push_back(newdb);
}

om_doclength
OmDatabase::Internal::get_avlength() const
{
    om_doccount docs = 0;
    om_doclength totlen = 0;

    vector<RefCntPtr<Database> >::const_iterator i;
    for (i = databases.begin(); i != databases.end(); ++i) {
	om_doccount db_doccount = (*i)->get_doccount();
	docs += db_doccount;
	totlen += (*i)->get_avlength() * db_doccount;
    }
    if (docs == 0) return 0.0;

    return totlen / docs;
}

LeafPostList *
OmDatabase::Internal::open_post_list(const string & tname,
				     const OmDatabase &db) const
{
    // Don't bother checking that the term exists first.  If it does, we
    // just end up doing more work, and if it doesn't, we save very little
    // work.
    vector<LeafPostList *> pls;
    try {
	vector<RefCntPtr<Database> >::const_iterator i;
	for (i = databases.begin(); i != databases.end(); i++) {
	    pls.push_back((*i)->open_post_list(tname));
	    pls.back()->next();
	}
	Assert(pls.begin() != pls.end());
    } catch (...) {
	vector<LeafPostList *>::iterator i;
	for (i = pls.begin(); i != pls.end(); i++) {
	    delete *i;
	    *i = 0;
	}
	throw;
    }

    return new MultiPostList(pls, db);
}

LeafTermList *
OmDatabase::Internal::open_term_list(om_docid did, const OmDatabase &db) const
{
    unsigned int multiplier = databases.size();
    Assert(multiplier != 0);
    om_docid realdid = (did - 1) / multiplier + 1;
    om_doccount dbnumber = (did - 1) % multiplier;

    return new MultiTermList(databases[dbnumber]->open_term_list(realdid),
			     databases[dbnumber], db);
}

Document *
OmDatabase::Internal::open_document(om_docid did) const
{
    unsigned int multiplier = databases.size();
    Assert(multiplier != 0);
    om_docid realdid = (did - 1) / multiplier + 1;
    om_doccount dbnumber = (did - 1) % multiplier;

    return databases[dbnumber]->open_document(realdid);
}

PositionList *
OmDatabase::Internal::open_position_list(om_docid did,
					 const string &tname) const
{
    unsigned int multiplier = databases.size();
    Assert(multiplier != 0);
    om_docid realdid = (did - 1) / multiplier + 1;
    om_doccount dbnumber = (did - 1) % multiplier;
    return databases[dbnumber]->open_position_list(realdid, tname);
}

TermList *
OmDatabase::Internal::open_allterms() const
{
    if (databases.empty()) return new EmptyAllTermsList();
    
    vector<TermList *> lists;

    vector<RefCntPtr<Database> >::const_iterator i;
    for (i = databases.begin(); i != databases.end(); ++i) {
	lists.push_back((*i)->open_allterms());
    }

    if (lists.size() == 1) return lists[0];

    return new MultiAllTermsList(lists);
}
