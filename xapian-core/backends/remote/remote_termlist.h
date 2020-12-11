/** @file
 * @brief Iterate terms in a remote document
 */
/* Copyright (C) 2007,2008,2011,2018 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_REMOTE_TERMLIST_H
#define XAPIAN_INCLUDED_REMOTE_TERMLIST_H

#include "api/termlist.h"

class RemoteDatabase;

/// Iterate terms in a remote document.
class RemoteTermList : public TermList {
    /// Don't allow assignment.
    void operator=(const RemoteTermList &) = delete;

    /// Don't allow copying.
    RemoteTermList(const RemoteTermList &) = delete;

    std::string current_term;

    Xapian::termcount current_wdf;

    Xapian::doccount current_termfreq;

    Xapian::termcount num_entries;

    Xapian::termcount doclen;

    Xapian::doccount db_size;

    Xapian::Internal::intrusive_ptr<const RemoteDatabase> db;

    Xapian::docid did;

    std::string data;

    const char* p = NULL;

  public:
    /// Construct.
    RemoteTermList(Xapian::termcount num_entries_,
		   Xapian::termcount doclen_,
		   Xapian::doccount db_size_,
		   const RemoteDatabase* db_,
		   Xapian::docid did_,
		   std::string&& data_)
	: num_entries(num_entries_),
	  doclen(doclen_),
	  db_size(db_size_),
	  db(db_),
	  did(did_),
	  data(data_) {}

    /// Return approximate size of this termlist.
    Xapian::termcount get_approx_size() const;

    void accumulate_stats(Xapian::Internal::ExpandStats& stats) const;

    /// Return the termname at the current position.
    std::string get_termname() const;

    /// Return the wdf for the term at the current position.
    Xapian::termcount get_wdf() const;

    /// Return the term frequency for the term at the current position.
    Xapian::doccount get_termfreq() const;

    /// Advance the current position to the next term in the termlist.
    TermList* next();

    /** Skip forward to the specified term.
     *
     *  If the specified term isn't in the list, position ourselves on the
     *  first term after @a term (or at_end() if no terms after @a term exist).
     */
    TermList* skip_to(const std::string& term);

    /// Return true if the current position is past the last term in this list.
    bool at_end() const;

    /// Return the length of the position list for the current position.
    Xapian::termcount positionlist_count() const;

    /// Return PositionList for the current position.
    PositionList* positionlist_begin() const;
};

#endif // XAPIAN_INCLUDED_REMOTE_TERMLIST_H
