/** @file net_postlist.h
 *  @brief Postlists for remote databases
 */
/* Copyright (C) 2007,2009 Lemur Consulting Ltd
 * Copyright (C) 2007,2008,2009,2011 Olly Betts
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

#include "api/leafpostlist.h"
#include "omassert.h"
#include "remote-database.h"

using namespace std;

/** A postlist in a remote database.
 */
class NetworkPostList : public LeafPostList {
    friend class RemoteDatabase;

    Xapian::Internal::intrusive_ptr<const RemoteDatabase> db;

    string postings;
    bool started;
    const char * pos;
    const char * pos_end;

    Xapian::docid lastdocid;
    Xapian::termcount lastwdf;
    Xapian::Internal::intrusive_ptr<PositionList> lastposlist;

    Xapian::doccount termfreq;

    /// Append a posting to the end of the postlist.
    void append_posting(const string & serialised) {
	Assert(pos == NULL);
	Assert(!started);
	postings.append(serialised);
    }

  public:
    /// Constructor.
    NetworkPostList(Xapian::Internal::intrusive_ptr<const RemoteDatabase> db_,
		    const string & term_)
	: LeafPostList(term_),
	  db(db_), started(false), pos(NULL), pos_end(NULL),
	  lastdocid(0), lastwdf(0), termfreq(0)
    {
	termfreq = db->read_post_list(term, *this);
    }

    /// Get number of documents indexed by this term.
    Xapian::doccount get_termfreq() const;

    /// Get the current document ID.
    Xapian::docid get_docid() const;

    /// Get the length of the current document.
    Xapian::termcount get_doclength() const;

    /// Get the number of unique terms in the current document.
    Xapian::termcount get_unique_terms() const;

    /// Get the Within Document Frequency of the term in the current document.
    Xapian::termcount get_wdf() const;

    /// Read the position list for the term in the current document and
    /// return a pointer to it (owned by the PostList).
    PositionList * read_position_list();

    /// Read the position list for the term in the current document and
    /// return a pointer to it (not owned by the PostList).
    PositionList * open_position_list() const;

    /// Move to the next document in the postlist (the weight parameter is
    /// ignored).
    PostList * next(double);

    /// Skip forward to the next document with document ID >= the supplied
    /// document ID (the weight parameter is ignored).
    PostList * skip_to(Xapian::docid did, double weight);

    /// Return true if and only if we've moved off the end of the list.
    bool at_end() const;

    /// Get a description of the postlist.
    string get_description() const;
};

#endif /* XAPIAN_INCLUDED_NET_POSTLIST_H */
