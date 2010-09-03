/* flint_alltermslist.h: A termlist containing all terms in a flint database.
 *
 * Copyright (C) 2005,2007,2008,2009,2010 Olly Betts
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

#ifndef XAPIAN_INCLUDED_FLINT_ALLTERMSLIST_H
#define XAPIAN_INCLUDED_FLINT_ALLTERMSLIST_H

#include "alltermslist.h"
#include "flint_database.h"
#include "flint_postlist.h"

class FlintCursor;

class FlintAllTermsList : public AllTermsList {
    /// Copying is not allowed.
    FlintAllTermsList(const FlintAllTermsList &);

    /// Assignment is not allowed.
    void operator=(const FlintAllTermsList &);

    /// Keep a reference to our database to stop it being deleted.
    Xapian::Internal::RefCntPtr<const FlintDatabase> database;

    /** A cursor which runs through the postlist table reading termnames from
     *  the keys.
     */
    FlintCursor * cursor;

    /// The termname at the current position.
    std::string current_term;

    /// The prefix to restrict the terms to.
    std::string prefix;

    /** The term frequency of the term at the current position.
     *
     *  If this value is zero, then we haven't read the term frequency or
     *  collection frequency for the current term yet.  We need to call
     *  read_termfreq_and_collfreq() to read these.
     */
    mutable Xapian::doccount termfreq;

    /// The collection frequency of the term at the current position.
    mutable Xapian::termcount collfreq;

    /// Read and cache the term frequency and collection frequency.
    void read_termfreq_and_collfreq() const;

  public:
    FlintAllTermsList(Xapian::Internal::RefCntPtr<const FlintDatabase> database_,
		      const std::string & prefix_)
	: database(database_), cursor(NULL), prefix(prefix_), termfreq(0) { }

    /// Destructor.
    ~FlintAllTermsList();

    /** Returns the current termname.
     *
     *  Either next() or skip_to() must have been called before this
     *  method can be called.
     */
    std::string get_termname() const;

    /** Returns the term frequency of the current term.
     *
     *  Either next() or skip_to() must have been called before this
     *  method can be called.
     */
    Xapian::doccount get_termfreq() const;

    /** Returns the collection frequency of the current term.
     *
     *  Either next() or skip_to() must have been called before this
     *  method can be called.
     */
    Xapian::termcount get_collection_freq() const;

    /// Advance to the next term in the list.
    TermList * next();

    /// Advance to the first term which is >= tname.
    TermList * skip_to(const std::string &tname);

    /// True if we're off the end of the list
    bool at_end() const;
};

#endif /* XAPIAN_INCLUDED_FLINT_ALLTERMSLIST_H */
