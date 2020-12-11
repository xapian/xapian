/** @file
 * @brief Synonym data for a honey database.
 */
/* Copyright (C) 2005,2007,2008,2009,2011,2014,2016,2017 Olly Betts
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

#ifndef XAPIAN_INCLUDED_HONEY_SYNONYM_H
#define XAPIAN_INCLUDED_HONEY_SYNONYM_H

#include <xapian/types.h>

#include "api/termlist.h"
#include "backends/alltermslist.h"
#include "honey_cursor.h"
#include "honey_lazytable.h"

#include <set>
#include <string>

class HoneyDatabase;

namespace Honey {
    class RootInfo;
}

class HoneySynonymTable : public HoneyLazyTable {
    /// The last term which was updated.
    mutable std::string last_term;

    /// The synonyms for the last term which was updated.
    mutable std::set<std::string> last_synonyms;

  public:
    /** Create a new HoneySynonymTable object.
     *
     *  This method does not create or open the table on disk - you
     *  must call the create() or open() methods respectively!
     *
     *  @param dbdir		The directory the honey database is stored in.
     *  @param readonly		true if we're opening read-only, else false.
     */
    HoneySynonymTable(const std::string& dbdir, bool readonly)
	: HoneyLazyTable("synonym", dbdir + "/synonym.", readonly) { }

    HoneySynonymTable(int fd, off_t offset_, bool readonly)
	: HoneyLazyTable("synonym", fd, offset_, readonly) { }

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
    void add_synonym(const std::string& term, const std::string& synonym);

    /** Remove a synonym for @a term.
     *
     *  If the synonym doesn't exist, no action is taken.
     */
    void remove_synonym(const std::string& term, const std::string& synonym);

    /** Remove all synonyms for @a term.
     *
     *  If @a term has no synonyms, no action is taken.
     */
    void clear_synonyms(const std::string& term);

    /** Open synonym termlist for a term.
     *
     *  If @a term has no synonyms, NULL is returned.
     */
    TermList* open_termlist(const std::string& term) const;

    /** Override methods of HoneyTable.
     *
     *  NB: these aren't virtual, but we always call them on the subclass in
     *  cases where it matters.
     *  @{
     */

    bool is_modified() const {
	return !last_term.empty() || HoneyTable::is_modified();
    }

    void flush_db() {
	merge_changes();
	HoneyTable::flush_db();
    }

    void cancel(const Honey::RootInfo& root_info,
		honey_revision_number_t rev) {
	discard_changes();
	HoneyTable::cancel(root_info, rev);
    }

    // @}
};

class HoneyCursor;

class HoneySynonymTermList : public AllTermsList {
    /// Copying is not allowed.
    HoneySynonymTermList(const HoneySynonymTermList&);

    /// Assignment is not allowed.
    void operator=(const HoneySynonymTermList&);

    /// Keep a reference to our database to stop it being deleted.
    Xapian::Internal::intrusive_ptr<const HoneyDatabase> database;

    /** A cursor which runs through the synonym table reading termnames from
     *  the keys.
     */
    HoneyCursor* cursor;

    /// The prefix to restrict the terms to.
    std::string prefix;

  public:
    HoneySynonymTermList(const HoneyDatabase* database_,
			 HoneyCursor* cursor_,
			 const std::string& prefix_)
	: database(database_), cursor(cursor_), prefix(prefix_)
    {
	// Set the cursor to its end to signal we haven't started yet.
	cursor->to_end();
    }

    /// Destructor.
    ~HoneySynonymTermList();

    Xapian::termcount get_approx_size() const;

    /** Returns the current termname.
     *
     *  Either next() or skip_to() must have been called before this
     *  method can be called.
     */
    std::string get_termname() const;

    /// Return the term frequency for the term at the current position.
    Xapian::doccount get_termfreq() const;

    /// Advance to the next term in the list.
    TermList* next();

    /// Advance to the first term which is >= term.
    TermList* skip_to(const std::string& term);

    /// True if we're off the end of the list
    bool at_end() const;
};

#endif // XAPIAN_INCLUDED_HONEY_SYNONYM_H
