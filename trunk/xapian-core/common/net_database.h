/* net_database.h: C++ class definition for network database access
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004 Olly Betts
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

#include "autoptr.h"  // auto_ptr
#include "omdebug.h"
#include "database.h"
#include "netclient.h"

class RemoteSubMatch;
class PendingMSetPostList;

/** A network database.  This is a reference to a remote database, and is
 *  mainly used by a RemoteSubMatch object.
 */
class NetworkDatabase : public Xapian::Database::Internal {
    friend class RemoteSubMatch;
    friend class PendingMSetPostList;
    private:
        /// Reference to the network link object
    	Xapian::Internal::RefCntPtr<NetClient> link;

	/// Set up the connection, including swapping statistics.
	void initialise_link();

    public:
	NetworkDatabase(Xapian::Internal::RefCntPtr<NetClient> link_);

	~NetworkDatabase();

	Xapian::doccount  get_doccount() const;
	Xapian::doclength get_avlength() const;
	Xapian::doclength get_doclength(Xapian::docid did) const;

	Xapian::doccount get_termfreq(const string & tname) const;
	Xapian::termcount get_collection_freq(const string & tname) const;
	bool term_exists(const string & tname) const;
	bool has_positions() const;

	LeafPostList * do_open_post_list(const string & tname) const;
	LeafTermList * open_term_list(Xapian::docid did) const;
	Xapian::Document::Internal * open_document(Xapian::docid did, bool lazy = false) const;
	PositionList * open_position_list(Xapian::docid did,
					const string & tname) const;
	TermList * open_allterms() const;

	void request_document(Xapian::docid did) const;
	Xapian::Document::Internal * collect_document(Xapian::docid did) const;
	
	// keep-alive
	void keep_alive() const;

	// Introspection methods...
	const NetworkDatabase * as_networkdatabase() const { return this; }
};

inline Xapian::termcount
NetworkDatabase::get_collection_freq(const string & /*tname*/) const
{
    throw Xapian::UnimplementedError("NetworkDatabase::get_collection_freq() not implemented.");
}

#endif /* OM_HGUARD_NET_DATABASE_H */
