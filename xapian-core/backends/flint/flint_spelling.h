/** @file flint_spelling.h
 * @brief Spelling correction data for a flint database.
 */
/* Copyright (C) 2007 Olly Betts
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

#ifndef XAPIAN_INCLUDED_FLINT_SPELLING_H
#define XAPIAN_INCLUDED_FLINT_SPELLING_H

#include <xapian/types.h>

#include "flint_table.h"
#include "termlist.h"

#include <map>
#include <set>
#include <string>
#include <string.h> // For memcpy() and memcmp().

struct fragment {
    char data[4];

    // Default constructor.
    fragment() { }

    // Allow implicit conversion.
    fragment(char data_[4]) { memcpy(data, data_, 4); }

    char & operator[] (unsigned i) { return data[i]; }
    const char & operator[] (unsigned i) const { return data[i]; }

    operator std::string () const {
	return string(data, data[0] == 'M' ? 4 : 3);
    }
};

inline bool operator<(const fragment &a, const fragment &b) {
    return memcmp(a.data, b.data, 4) < 0;
}

class FlintSpellingTable : public FlintTable {
    void add_fragment(fragment frag, const string & word);
    void remove_fragment(fragment frag, const string & word);

    std::map<std::string, Xapian::termcount> wordfreq_changes;
    std::map<fragment, std::set<std::string> > termlist_deltas;

  public:
    /** Create a new FlintSpellingTable object.
     *
     *  This method does not create or open the table on disk - you
     *  must call the create() or open() methods respectively!
     *
     *  @param dbdir		The directory the flint database is stored in.
     *  @param readonly		true if we're opening read-only, else false.
     */
    FlintSpellingTable(std::string dbdir, bool readonly)
	: FlintTable(dbdir + "/spelling.", readonly, Z_DEFAULT_COMPRESSION, true) { }

    // Merge in batched-up changes.
    void merge_changes();

    // Discard batched-up changes.
    void discard_changes() {
	wordfreq_changes.clear();
	termlist_deltas.clear();
    }

    void add_word(const std::string & word, Xapian::termcount freqinc);
    void remove_word(const std::string & word, Xapian::termcount freqdec);

    TermList * open_termlist(const std::string & word);

    Xapian::doccount get_word_frequency(const string & word) const;

    // Override methods of FlintTable (NB: these aren't virtual, but we always
    // call them on the subclass in cases where it matters).
    bool is_modified() const {
	return !wordfreq_changes.empty() || FlintTable::is_modified();
    }

    void create_and_open(unsigned int blocksize) {
	// The spelling table is created lazily, but erase it in case we're
	// overwriting an existing database and it already exists.
	FlintTable::erase();
	FlintTable::set_block_size(blocksize);
    }

    void commit(flint_revision_number_t revision) {
	merge_changes();
	FlintTable::commit(revision);
    }

    void cancel() {
	discard_changes();
	FlintTable::cancel();
    }
};

/** The list of words containing a particular trigram. */
class FlintSpellingTermList : public TermList {
    /// The encoded data.
    std::string data;

    /// Position in the data.
    unsigned p;

    /// The current term.
    std::string current_term;

    /// Copying is not allowed.
    FlintSpellingTermList(const FlintSpellingTermList &);

    /// Assignment is not allowed.
    void operator=(const FlintSpellingTermList &);

  public:
    /// Constructor.
    FlintSpellingTermList(const std::string & data_)
	: data(data_), p(0) { }

    /// Destructor.
    ~FlintSpellingTermList();

    Xapian::termcount get_approx_size() const;

    void accumulate_stats(Xapian::Internal::ExpandStats & stats) const;

    std::string get_termname() const;

    Xapian::termcount get_wdf() const;

    Xapian::doccount get_termfreq() const;

    Xapian::termcount get_collection_freq() const;

    TermList *next();

    bool at_end() const;

    Xapian::termcount positionlist_count() const;

    Xapian::PositionIterator positionlist_begin() const;
};

#endif // XAPIAN_INCLUDED_FLINT_SPELLING_H
