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

#ifndef _multi_database_h_
#define _multi_database_h_

#include "omassert.h"
#include "database.h"
#include <stdlib.h>
#include <set>
#include <vector>
#include <list>

class MultiDatabase : public virtual IRDatabase {
    friend class DatabaseBuilder;
    private:
	mutable set<termname> terms;

	vector<IRDatabase *> databases;

	mutable bool length_initialised;
	mutable doclength avlength;

	bool opened; // Whether we have opened the database (ie, added a subDB)
	mutable bool used;// Have we used the database (if so, can't add more DBs)

	MultiDatabase();
	void open(const DatabaseBuilderParams & params);
    public:
	~MultiDatabase();

	void set_root(IRDatabase * db);

	doccount  get_doccount() const;
	doclength get_avlength() const;

	doccount get_termfreq(const termname & tname) const;
	bool term_exists(const termname & tname) const;

	DBPostList * open_post_list(const termname & tname, RSet * rset) const;
	DBTermList * open_term_list(docid did) const;
	IRDocument * open_document(docid did) const;

	void make_term(const termname &) {
	    throw OmError("DADatabase::make_term() not implemented");
	}
	docid make_doc(const docname &) {
	    throw OmError("DADatabase::make_doc() not implemented");
	}
	void make_posting(const termname &, unsigned int, unsigned int) {
	    throw OmError("DADatabase::make_posting() not implemented");
	}
};

inline doccount
MultiDatabase::get_doccount() const
{
    // FIXME - lazy evaluation?
    Assert(opened);
    Assert((used = true) == true);

    doccount docs = 0;

    vector<IRDatabase *>::const_iterator i = databases.begin();
    while(i != databases.end()) {
	docs += (*i)->get_doccount();
	i++;
    }

    return docs;
}

inline doclength
MultiDatabase::get_avlength() const
{
    // FIXME - lazy evaluation?
    Assert(opened);
    Assert((used = true) == true);

    if(!length_initialised) {
	doccount docs = 0;
	doclength totlen = 0;

	vector<IRDatabase *>::const_iterator i = databases.begin(); 
	while(i != databases.end()) {
	    doccount db_doccount = (*i)->get_doccount();
	    docs += db_doccount;
	    totlen += (*i)->get_avlength() * db_doccount;
	    i++;
	}

	avlength = totlen / docs;
	length_initialised = true;
    }

    return avlength;
}

inline doccount
MultiDatabase::get_termfreq(const termname & tname) const
{
    if(!term_exists(tname)) return 0;
    PostList *pl = open_post_list(tname, NULL);
    doccount freq = 0;
    if(pl) freq = pl->get_termfreq();
    delete pl;
    return freq;
}

#endif /* _multi_database_h_ */
