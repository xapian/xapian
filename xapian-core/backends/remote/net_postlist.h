/** @file net_postlist.h
 *  @brief Postlists for remote databases
 */
/* Copyright (C) 2007 Lemur Consulting Ltd
 * Copyright (C) 2007 Olly Betts
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

#ifndef XAPIAN_INCLUDED_NET_POSTLIST_H
#define XAPIAN_INCLUDED_NET_POSTLIST_H

#include <string>

#include "leafpostlist.h"
#include "remote-database.h"
#include "serialise.h"
#include "serialise-double.h"

using namespace std;

/** A postlist in a remote database.
 */
class NetworkPostList : public LeafPostList {
    friend class RemoteDatabase;

    Xapian::Internal::RefCntPtr<const RemoteDatabase> db;
    string term;

    string postings;
    bool started;
    const char * pos;
    const char * pos_end;

    Xapian::docid lastdocid;
    Xapian::termcount lastwdf;
    Xapian::doclength lastdoclen;
    Xapian::Internal::RefCntPtr<PositionList> lastposlist;

    Xapian::doccount termfreq;

    /// Append a posting to the end of the postlist.
    void append_posting(const string & serialised) {
	Assert(pos == NULL);
	Assert(!started);
	postings.append(serialised);
    }

  public:
    /// Default constructor.
    NetworkPostList(Xapian::Internal::RefCntPtr<const RemoteDatabase> db_,
		    const string & term_);

    /// Get number of documents indexed by this term.
    Xapian::doccount get_termfreq() const {
	return termfreq;
    }

    /// Get the current document ID.
    Xapian::docid get_docid() const
    {
	return lastdocid;
    }

    /// Get the length of the current document.
    Xapian::doclength get_doclength() const
    {
	return lastdoclen;
    }

    /// Get the Within Document Frequency of the term in the current document.
    Xapian::termcount get_wdf() const
    {
	return lastwdf;
    }

    /// Read the position list for the term in the current document and
    /// return a pointer to it (owned by the PostList).
    PositionList * read_position_list();

    /// Read the position list for the term in the current document and
    /// return a pointer to it (not owned by the PostList).
    PositionList * open_position_list() const;

    /// Move to the next document in the postlist (the weight parameter is
    /// ignored).
    PostList * next(Xapian::weight);

    /// Skip forward to the next document with document ID >= the supplied
    /// document ID (the weight parameter is ignored).
    PostList * skip_to(Xapian::docid did, Xapian::weight weight);

    /// Return true if and only if we've moved off the end of the list.
    bool at_end() const
    {
	return (pos == NULL && started);
    }

    /// Get a description of the postlist.
    string get_description() const;
};

#endif /* XAPIAN_INCLUDED_NET_POSTLIST_H */
