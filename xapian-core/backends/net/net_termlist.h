/* net_termlist.h
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

#ifndef OM_HGUARD_NET_TERMLIST_H
#define OM_HGUARD_NET_TERMLIST_H

#include "om/omtypes.h"
#include "om/omerror.h"
#include "termlist.h"
#include "expandweight.h"
#include "netclient.h"
#include "net_database.h"

/** An item in a NetworkTermList.
 */
class NetworkTermListItem {
    public:
	/** The "name" of this term.
	 */
	om_termname tname;

	/** The term frequency.
	 *  This is the number of documents (in the network database)
	 *  indexed by the term.
	 */
	om_doccount termfreq;

	/** The within-document-frequency of the term.
	 *
	 *  This information may not be available, in which case the field
	 *  should have a value of 0.
	 */
	om_termcount wdf;

	/** Sorted list of positions
	 *
	 *  This information may not be present, in which case the vector
	 *  will be empty.
	 *
	 *  Note that, if this information _is_ present, the wdf information
	 *  should also be present (but need not neccessarily agree with
	 *  the length of the position vector, for various reasons).
	 */
	std::vector<om_termpos> positions;
};

/** A term list for a database on the other side of a network connection.
 *  The termlist is serialised across the network, and rebuilt into this
 *  object on the client side.
 */
class NetworkTermList : public LeafTermList {
    friend class NetworkDatabase;
    private:
	/** The list of items comprising the termlist.
	 */
	std::vector<NetworkTermListItem> items;

	/** The current position in the list.
	 */
	std::vector<NetworkTermListItem>::const_iterator current_position;

	/** Whether we have yet started iterating through the list.
	 */
	bool started;

	/** The length of the document for which this is the termlist.
	 *
	 *  Note that this is not a normalised document length.
	 */
	om_doclength document_length;

	/** The number of documents in the database in which this
	 *  document resides.
	 *
	 *  Note that this may not be the number of documents in the combined
	 *  database when multiple databases are being searched.
	 */
	om_doccount database_size;

	///  Keep a reference to our database
	OmRefCntPtr<const NetworkDatabase> this_db;

	/** Standard constructor is private: NetworkTermLists are created
	 *  by NetworkDatabase object only, which is a friend.
	 *
	 *  @param average_length_  The average length of a document
	 *  @param database_size_
	 */
	NetworkTermList(om_doclength average_length_,
			om_doccount  database_size_,
			const std::vector<NetClient::TermListItem> &items_,
			OmRefCntPtr<const NetworkDatabase> this_db_);
    public:

	/** Get the number of terms in the termlist.
	 */
	om_termcount get_approx_size() const;

	OmExpandBits get_weighting() const;
	const om_termname get_termname() const;
	om_termcount get_wdf() const;
	om_doccount get_termfreq() const;
	TermList * next();
	bool   at_end() const;
};

#endif /* OM_HGUARD_NET_TERMLIST_H */
