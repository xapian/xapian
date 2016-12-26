/** @file chert_spelling.h
 * @brief Spelling correction data for a chert database.
 */
/* Copyright (C) 2007,2008,2009,2010,2011 Olly Betts
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

#ifndef XAPIAN_INCLUDED_CHERT_SPELLING_H
#define XAPIAN_INCLUDED_CHERT_SPELLING_H

#include <xapian/types.h>

#include "chert_lazytable.h"
#include "api/termlist.h"

#include <map>
#include <set>
#include <string>
#include <cstring> // For memcpy() and memcmp().

struct fragment {
    char data[4];

    // Default constructor.
    fragment() { }

    // Allow implicit conversion.
    explicit fragment(char data_[4]) { std::memcpy(data, data_, 4); }

    char & operator[] (unsigned i) { return data[i]; }
    const char & operator[] (unsigned i) const { return data[i]; }

    operator std::string () const {
	return std::string(data, data[0] == 'M' ? 4 : 3);
    }
};

inline bool operator<(const fragment &a, const fragment &b) {
    return std::memcmp(a.data, b.data, 4) < 0;
}

class ChertSpellingTable : public ChertLazyTable {
    void toggle_word(const std::string & word);
    void toggle_fragment(fragment frag, const std::string & word);

    std::map<std::string, Xapian::termcount> wordfreq_changes;

    /** Changes to make to the termlists.
     *
     *  This list is essentially xor-ed with the list on disk, so an entry
     *  here either means a new entry needs to be added on disk, or an
     *  existing entry on disk needs to be removed.  We do it this way so
     *  we don't need to store an additional add/remove flag for every
     *  word.
     */
    std::map<fragment, std::set<std::string> > termlist_deltas;

  public:
    /** Create a new ChertSpellingTable object.
     *
     *  This method does not create or open the table on disk - you
     *  must call the create() or open() methods respectively!
     *
     *  @param dbdir		The directory the chert database is stored in.
     *  @param readonly		true if we're opening read-only, else false.
     */
    ChertSpellingTable(const std::string & dbdir, bool readonly)
	: ChertLazyTable("spelling", dbdir + "/spelling.", readonly,
			 Z_DEFAULT_STRATEGY) { }

    // Merge in batched-up changes.
    void merge_changes();

    void add_word(const std::string & word, Xapian::termcount freqinc);
    void remove_word(const std::string & word, Xapian::termcount freqdec);

    TermList * open_termlist(const std::string & word);

    Xapian::doccount get_word_frequency(const std::string & word) const;

    /** Override methods of ChertTable.
     *
     *  NB: these aren't virtual, but we always call them on the subclass in
     *  cases where it matters.
     *  @{
     */

    bool is_modified() const {
	return !wordfreq_changes.empty() || ChertTable::is_modified();
    }

    void flush_db() {
	merge_changes();
	ChertTable::flush_db();
    }

    void cancel() {
	// Discard batched-up changes.
	wordfreq_changes.clear();
	termlist_deltas.clear();

	ChertTable::cancel();
    }

    // @}
};

/** The list of words containing a particular trigram. */
class ChertSpellingTermList : public TermList {
    /// The encoded data.
    std::string data;

    /// Position in the data.
    unsigned p;

    /// The current term.
    std::string current_term;

    /// Copying is not allowed.
    ChertSpellingTermList(const ChertSpellingTermList &);

    /// Assignment is not allowed.
    void operator=(const ChertSpellingTermList &);

  public:
    /// Constructor.
    explicit ChertSpellingTermList(const std::string & data_)
	: data(data_), p(0) { }

    Xapian::termcount get_approx_size() const;

    std::string get_termname() const;

    Xapian::termcount get_wdf() const;

    Xapian::doccount get_termfreq() const;

    Xapian::termcount get_collection_freq() const;

    TermList * next();

    TermList * skip_to(const std::string & term);

    bool at_end() const;

    Xapian::termcount positionlist_count() const;

    Xapian::PositionIterator positionlist_begin() const;
};

#endif // XAPIAN_INCLUDED_CHERT_SPELLING_H
