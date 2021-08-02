/** @file
 *  @brief Postlists for remote databases
 */
/* Copyright (C) 2007,2009 Lemur Consulting Ltd
 * Copyright (C) 2007,2008,2009,2011,2019 Olly Betts
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

#include "backends/leafpostlist.h"
#include "omassert.h"
#include "remote-database.h"

/** A postlist in a remote database.
 */
class NetworkPostList : public LeafPostList {
    friend class RemoteDatabase;

    Xapian::Internal::intrusive_ptr<const RemoteDatabase> db;

    std::string postings;
    bool started = false;
    const char* pos = NULL;
    const char* pos_end = NULL;

    Xapian::docid lastdocid = 0;
    Xapian::termcount lastwdf = 0;

    Xapian::doccount termfreq;

  public:
    /// Constructor.
    NetworkPostList(Xapian::Internal::intrusive_ptr<const RemoteDatabase> db_,
		    const std::string& term_,
		    Xapian::doccount termfreq_,
		    std::string&& postings_)
	: LeafPostList(term_),
	  db(db_), postings(std::move(postings_)), termfreq(termfreq_) { }

    /// Get number of documents indexed by this term.
    Xapian::doccount get_termfreq() const;

    /// Get the current document ID.
    Xapian::docid get_docid() const;

    /// Get the Within Document Frequency of the term in the current document.
    Xapian::termcount get_wdf() const;

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

    Xapian::termcount get_wdf_upper_bound() const;

    /// Get a description of the postlist.
    std::string get_description() const;
};

#endif /* XAPIAN_INCLUDED_NET_POSTLIST_H */
