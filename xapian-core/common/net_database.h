/* net_database.h: C++ class definition for network database access
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

#ifndef OM_HGUARD_NET_DATABASE_H
#define OM_HGUARD_NET_DATABASE_H

#include "config.h"
#include <memory>  // auto_ptr
#include "omassert.h"
#include "database.h"
#include "netclient.h"

class NetworkDatabase : public IRDatabase {
    friend class DatabaseBuilder;
    friend class NetworkMatch;
    private:
        /// Reference to the network link object
    	OmRefCntPtr<NetClient> link;

	// indicator for error checking
	bool opened;

	/// Set up the connection, including swapping statistics.
	void initialise_link();

	NetworkDatabase();
	void open(const DatabaseBuilderParams & params);
    public:
	~NetworkDatabase();

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
	    throw OmUnimplementedError("NetworkDatabase::make_term() not implemented");
	}
	om_docid make_doc(const om_docname &) {
	    throw OmUnimplementedError("NetworkDatabase::make_doc() not implemented");
	}
	void make_posting(const om_termname &, unsigned int, unsigned int) {
	    throw OmUnimplementedError("NetworkDatabase::make_posting() not implemented");
	}

	// Introspection methods...
	bool is_network() const;
};

inline om_doclength
NetworkDatabase::get_avlength() const
{
    throw OmUnimplementedError("NetworkDatabase::get_avlength() not implemented");
#if 0
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
#endif
}

inline om_doccount
NetworkDatabase::get_termfreq(const om_termname & tname) const
{
    throw OmUnimplementedError("NetworkDatabase::get_termfreq() not implemented");
#if 0
    if(!term_exists(tname)) return 0;
    PostList *pl = open_post_list(tname, NULL);
    om_doccount freq = 0;
    if(pl) freq = pl->get_termfreq();
    delete pl;
    return freq;
#endif
}

inline bool
NetworkDatabase::is_network() const
{
    return true;
}

#endif /* OM_HGUARD_NET_DATABASE_H */
