/** @file flint_synonym.h
 * @brief Synonym data for a flint database.
 */
/* Copyright (C) 2005,2007,2008,2009 Olly Betts
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

#ifndef XAPIAN_INCLUDED_FLINT_SYNONYM_H
#define XAPIAN_INCLUDED_FLINT_SYNONYM_H

#include <xapian/types.h>

#include "alltermslist.h"
#include "database.h"
#include "flint_table.h"
#include "termlist.h"

#include <set>
#include <string>

class FlintSynonymTable : public FlintTable {
    /// The last term which was updated.
    mutable std::string last_term;

    /// The synonyms for the last term which was updated.
    mutable std::set<std::string> last_synonyms;

  public:
    /** Create a new FlintSynonymTable object.
     *
     *  This method does not create or open the table on disk - you
     *  must call the create() or open() methods respectively!
     *
     *  @param dbdir		The directory the flint database is stored in.
     *  @param readonly		true if we're opening read-only, else false.
     */
    FlintSynonymTable(const std::string & dbdir, bool readonly)
	: FlintTable("synonym", dbdir + "/synonym.", readonly, Z_DEFAULT_STRATEGY, true) { }

    // Merge in batched-up changes.
    void merge_changes();

    // Discard batched-up changes.
    void discard_changes() {
	last_term.resize(0);
	last_synonyms.clear();
    }

    /** Add a synonym for @a term.
     *
     *  If the synonym has already been added, no action is taken.
     */
    void add_synonym(const std::string & term, const std::string & synonym);

    /** Remove a synonym for @a term.
     *
     *  If the synonym doesn't exist, no action is taken.
     */
    void remove_synonym(const std::string & term, const std::string & synonym);

    /** Remove all synonyms for @a term.
     *
     *  If @a term has no synonyms, no action is taken.
     */
    void clear_synonyms(const std::string & term);

    /** Open synonym termlist for a term.
     *
     *  If @a term has no synonyms, NULL is returned.
     */
    TermList * open_termlist(const std::string & term);

    /** Override methods of FlintTable.
     *
     *  NB: these aren't virtual, but we always call them on the subclass in
     *  cases where it matters).
     *  @{
     */

    bool is_modified() const {
	return !last_term.empty() || FlintTable::is_modified();
    }

    void create_and_open(unsigned int blocksize) {
	// The synonym table is created lazily, but erase it in case we're
	// overwriting an existing database and it already exists.
	FlintTable::erase();
	FlintTable::set_block_size(blocksize);
    }

    void flush_db() {
	merge_changes();
	FlintTable::flush_db();
    }

    void cancel() {
	discard_changes();
	FlintTable::cancel();
    }

    // @}
};

class FlintCursor;

class FlintSynonymTermList : public AllTermsList {
    /// Copying is not allowed.
    FlintSynonymTermList(const FlintSynonymTermList &);

    /// Assignment is not allowed.
    void operator=(const FlintSynonymTermList &);

    /// Keep a reference to our database to stop it being deleted.
    Xapian::Internal::RefCntPtr<const Xapian::Database::Internal> database;

    /** A cursor which runs through the synonym table reading termnames from
     *  the keys.
     */
    FlintCursor * cursor;

    /// The prefix to restrict the terms to.
    string prefix;

  public:
    FlintSynonymTermList(Xapian::Internal::RefCntPtr<const Xapian::Database::Internal> database_,
		      FlintCursor * cursor_,
		      const string & prefix_)
	    : database(database_), cursor(cursor_), prefix(prefix_)
    {
	// Position the cursor on the highest key before the first key we want,
	// so that the first call to next() will put us on the first key we
	// want.
	if (prefix.empty()) {
	    cursor->find_entry(string());
	} else {
	    // Seek to the first key before one with the desired prefix.
	    cursor->find_entry_lt(prefix);
	}
    }

    /// Destructor.
    ~FlintSynonymTermList();

    /** Returns the current termname.
     *
     *  Either next() or skip_to() must have been called before this
     *  method can be called.
     */
    string get_termname() const;

    /// Return the term frequency for the term at the current position.
    Xapian::doccount get_termfreq() const;

    /// Return the collection frequency for the term at the current position.
    Xapian::termcount get_collection_freq() const;

    /// Advance to the next term in the list.
    TermList * next();

    /// Advance to the first term which is >= tname.
    TermList * skip_to(const string &tname);

    /// True if we're off the end of the list
    bool at_end() const;
};

#endif // XAPIAN_INCLUDED_FLINT_SYNONYM_H
