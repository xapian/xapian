/* multi_database.h: C++ class definition for multiple database access
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

#ifndef OM_HGUARD_MULTI_DATABASE_H
#define OM_HGUARD_MULTI_DATABASE_H

#include "omassert.h"
#include "database.h"
#include <stdlib.h>
#include <set>
#include <vector>
#include <list>

class MultiDatabase : public IRDatabase {
    friend class DatabaseBuilder;
    private:
	mutable set<om_termname> terms;

	vector<IRDatabase *> databases;

	mutable bool length_initialised;
	mutable om_doclength avlength;

	bool opened; // Whether we have opened the database (ie, added a sub database)
	mutable bool used;// Have we used the database (if so, can't add more  databases)

	MultiDatabase();
	void open(const DatabaseBuilderParams & params);
    public:
	~MultiDatabase();

	void set_root(IRDatabase * db);

	om_doccount  get_doccount() const;
	om_doclength get_avlength() const;
	om_doclength get_doclength(om_docid did) const;

	om_doccount get_termfreq(const om_termname & tname) const;
	bool term_exists(const om_termname & tname) const;

	LeafPostList * open_post_list(const om_termname & tname, RSet * rset) const;
	LeafTermList * open_term_list(om_docid did) const;
	LeafDocument * open_document(om_docid did) const;

	void make_term(const om_termname &) {
	    throw OmUnimplementedError("DADatabase::make_term() not implemented");
	}
	om_docid make_doc(const om_docname &) {
	    throw OmUnimplementedError("DADatabase::make_doc() not implemented");
	}
	void make_posting(const om_termname &, unsigned int, unsigned int) {
	    throw OmUnimplementedError("DADatabase::make_posting() not implemented");
	}
};

inline om_doccount
MultiDatabase::get_doccount() const
{
    Assert(opened);
    Assert((used = true) == true);

    om_doccount docs = 0;

    vector<IRDatabase *>::const_iterator i = databases.begin();
    while(i != databases.end()) {
	docs += (*i)->get_doccount();
	i++;
    }

    return docs;
}

inline om_doclength
MultiDatabase::get_avlength() const
{
    Assert(opened);
    Assert((used = true) == true);

    if(!length_initialised) {
	om_doccount docs = 0;
	om_doclength totlen = 0;

	vector<IRDatabase *>::const_iterator i = databases.begin(); 
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

inline om_doccount
MultiDatabase::get_termfreq(const om_termname & tname) const
{
    if(!term_exists(tname)) return 0;
    PostList *pl = open_post_list(tname, NULL);
    om_doccount freq = 0;
    if(pl) freq = pl->get_termfreq();
    delete pl;
    return freq;
}

#endif /* OM_HGUARD_MULTI_DATABASE_H */
