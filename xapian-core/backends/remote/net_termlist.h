/** @file net_termlist.h
 * @brief Termlist in a remote db
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003,2006,2007,2009,2010,2011 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef OM_HGUARD_NET_TERMLIST_H
#define OM_HGUARD_NET_TERMLIST_H

#include <string>

#include <xapian/types.h>
#include <xapian/error.h>
#include "api/termlist.h"
#include "expand/expandweight.h"
#include "remote-database.h"

using namespace std;

/** An item in a NetworkTermList.
 */
class NetworkTermListItem {
    public:
	/** The "name" of this term.
	 */
	string tname;

	/** The term frequency.
	 *  This is the number of documents (in the network database)
	 *  indexed by the term.
	 */
	Xapian::doccount termfreq;

	/** The within-document-frequency of the term.
	 *
	 *  This information may not be available, in which case the field
	 *  should have a value of 0.
	 */
	Xapian::termcount wdf;
};

/** A term list for a database on the other side of a network connection.
 *  The termlist is serialised across the network, and rebuilt into this
 *  object on the client side.
 */
class NetworkTermList : public TermList {
    friend class RemoteDatabase;
    private:
	/** The list of items comprising the termlist.
	 */
	vector<NetworkTermListItem> items;

	/** The current position in the list.
	 */
	vector<NetworkTermListItem>::const_iterator current_position;

	/** Whether we have yet started iterating through the list.
	 */
	bool started;

	/** The length of the document for which this is the termlist.
	 *
	 *  Note that this is not a normalised document length.
	 */
	Xapian::termcount document_length;

	/** The number of documents in the database in which this
	 *  document resides.
	 *
	 *  Note that this may not be the number of documents in the combined
	 *  database when multiple databases are being searched.
	 */
	Xapian::doccount database_size;

	///  Keep a reference to our database
	Xapian::Internal::intrusive_ptr<const RemoteDatabase> this_db;

	/// The id of the document this termlist came from (or 0 if not applicable).
	Xapian::docid did;

	/** Standard constructor is private: NetworkTermLists are created
	 *  by RemoteDatabase object only, which is a friend.
	 *
	 *  @param document_length_ The (non-normalised) length of the document
	 *  @param database_size_   The number of documents in the database
	 *  @param this_db_	    The database
	 *  @param did_		    The document id
	 */
	NetworkTermList(Xapian::termcount document_length_,
			Xapian::doccount database_size_,
			Xapian::Internal::intrusive_ptr<const RemoteDatabase> this_db_,
			Xapian::docid did_);
    public:

	/** Get the number of terms in the termlist.
	 */
	Xapian::termcount get_approx_size() const;

	// Collate weighting information for the current term.
	void accumulate_stats(Xapian::Internal::ExpandStats &stats) const;
	string get_termname() const;
	Xapian::termcount get_wdf() const;
	Xapian::doccount get_termfreq() const;
	TermList * next();
	TermList * skip_to(const std::string &term);
	bool at_end() const;

	Xapian::termcount positionlist_count() const;
	Xapian::PositionIterator positionlist_begin() const;
};

#endif /* OM_HGUARD_NET_TERMLIST_H */
