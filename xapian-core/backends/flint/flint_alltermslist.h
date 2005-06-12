/* flint_alltermslist.h: A termlist containing all terms in a flint database.
 *
 * Copyright (C) 2005 Olly Betts
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
 */

#ifndef XAPIAN_HGUARD_FLINT_ALLTERMSLIST_H
#define XAPIAN_HGUARD_FLINT_ALLTERMSLIST_H

#include "alltermslist.h"
#include "database.h"
#include "flint_postlist.h"

class FlintCursor;

class FlintAllTermsList : public AllTermsList {
    /// Copying is not allowed.
    FlintAllTermsList(const FlintAllTermsList &);

    /// Assignment is not allowed.
    void operator=(const FlintAllTermsList &);

    /// Keep a reference to our database to stop it being deleted.
    Xapian::Internal::RefCntPtr<const Xapian::Database::Internal> database;

    /** A cursor which runs through the postlist table reading termnames from
     *  the keys.
     */
    FlintCursor * cursor;

    /// The approximate number of terms in this list.
    Xapian::termcount approx_size;

    /// The termname at the current position.
    string current_term;

    /** The term frequency of the term at the current position.
     *
     *  If this value is zero, then we haven't read the term frequency or
     *  collection frequency for the current term yet.  We need to call
     *  read_termfreq_and_collfreq() to read these.
     */
    mutable Xapian::termcount termfreq;

    /// The collection frequency of the term at the current position.
    mutable Xapian::termcount collfreq;

    /// Read and cache the term frequency and collection frequency.
    void read_termfreq_and_collfreq() const;

  public:
    FlintAllTermsList(Xapian::Internal::RefCntPtr<const Xapian::Database::Internal> database_,
		      const FlintPostListTable * pltab) : database(database_), termfreq(0) {
	// The number of entries in the postlist table will be the number of
	// terms, probably plus some extra entries for chunked posting lists,
	// plus 1 for the metainfo key (unless the table is completely empty).
	// FIXME : this can badly overestimate (it could be several times too
	// large) - perhaps keep track of the number of terms (or number of
	// extra chunks) and store this along with the last_docid and
	// total_doclen?  (Mind you, not sure this value is ever used, but
	// we should really make the exact number of terms available somewhere
	// in the API).
	approx_size = pltab->get_entry_count();
	if (approx_size) --approx_size;

	// Seek to the metainfo key, so the first next will advance us to the
	// first real key.
	cursor = pltab->cursor_get();
	cursor->find_entry(string("", 1));
    }

    /// Destructor.
    ~FlintAllTermsList();

    /** Returns the approximate size of the list.
     *
     *  This is probably unused for this class.
     */
    Xapian::termcount get_approx_size() const;

    /** Returns the current termname.
     *
     *  Either next() or skip_to() must have been called before this
     *  method can be called.
     */
    string get_termname() const;

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
    TermList * skip_to(const string &tname);

    /// True if we're off the end of the list
    bool at_end() const;
};

#endif /* XAPIAN_HGUARD_FLINT_ALLTERMSLIST_H */
