/* net_database.h: C++ class definition for network database access
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
class NetworkDatabase : public Database {
    friend class DatabaseBuilder;
    friend class RemoteSubMatch;
    friend class PendingMSetPostList;
    private:
        /// Reference to the network link object
    	RefCntPtr<NetClient> link;

	/// Set up the connection, including swapping statistics.
	void initialise_link();

	//@{
	/** NetworkDatabase is a readonly database type, and thus this method
	 *  is not supported: if called an exception will be thrown.
	 */
	void do_begin_session() {
	    throw Xapian::UnimplementedError(
		"NetworkDatabase::begin_session() not implemented: readonly database type");
	}

	void do_end_session() {
	    throw Xapian::UnimplementedError(
		"NetworkDatabase::do_end_session() not implemented: readonly database type");
	}

	void do_flush() {
	    throw Xapian::UnimplementedError(
		"NetworkDatabase::flush() not implemented: readonly database type");
	}

	void do_begin_transaction() {
	    throw Xapian::UnimplementedError(
		"NetworkDatabase::begin_transaction() not implemented: readonly database type");
	}

	void do_commit_transaction() {
	    throw Xapian::UnimplementedError(
		"NetworkDatabase::commit_transaction() not implemented: readonly database type");
	}

	void do_cancel_transaction() {
	    throw Xapian::UnimplementedError(
		"NetworkDatabase::cancel_transaction() not implemented: readonly database type");
	}

	om_docid do_add_document(const OmDocument & /*document*/) {
	    throw Xapian::UnimplementedError(
		"NetworkDatabase::add_document() not implemented: readonly database type");
	}

	void do_delete_document(om_docid /*did*/) {
	    throw Xapian::UnimplementedError(
		"NetworkDatabase::delete_document() not implemented: readonly database type");
	}

	void do_replace_document(om_docid /*did*/, const OmDocument & /*document*/) {
	    throw Xapian::UnimplementedError(
		"NetworkDatabase::replace_document() not implemented: readonly database type");
	}

	//@}

    public:
	NetworkDatabase(RefCntPtr<NetClient> link_);

	~NetworkDatabase();

	om_doccount  get_doccount() const;
	om_doclength get_avlength() const;
	om_doclength get_doclength(om_docid did) const;

	om_doccount get_termfreq(const string & tname) const;
	om_termcount get_collection_freq(const string & tname) const;
	bool term_exists(const string & tname) const;

	LeafPostList * do_open_post_list(const string & tname) const;
	LeafTermList * open_term_list(om_docid did) const;
	Document * open_document(om_docid did, bool lazy = false) const;
	PositionList * open_position_list(om_docid did,
					const string & tname) const;
	TermList * open_allterms() const;

	void request_document(om_docid did) const;
	Document * collect_document(om_docid did) const;
	
	// keep-alive
	void keep_alive() const;

	// Introspection methods...
	const NetworkDatabase * as_networkdatabase() const { return this; }
};

inline om_termcount
NetworkDatabase::get_collection_freq(const string & /*tname*/) const
{
    throw Xapian::UnimplementedError("NetworkDatabase::get_collection_freq() not implemented: data not stored in database.");
}

#endif /* OM_HGUARD_NET_DATABASE_H */
