/** @file honey_spelling.h
 * @brief Spelling correction data for a honey database.
 */
/* Copyright (C) 2007,2008,2009,2010,2011,2014,2015,2016,2017,2018 Olly Betts
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

#ifndef XAPIAN_INCLUDED_HONEY_SPELLING_H
#define XAPIAN_INCLUDED_HONEY_SPELLING_H

#include <xapian/types.h>

#include "honey_lazytable.h"
#include "api/termlist.h"

#include <map>
#include <set>
#include <string>
#include <cstring> // For memcpy() and memcmp().

namespace Honey {

const unsigned KEY_PREFIX_BOOKEND = 0x00;
const unsigned KEY_PREFIX_HEAD = 0x01;
const unsigned KEY_PREFIX_MIDDLE = 0x02;
const unsigned KEY_PREFIX_TAIL = 0x03;
const unsigned KEY_PREFIX_WORD = 0x04;

inline std::string
make_spelling_wordlist_key(const std::string& word)
{
    if (rare(static_cast<unsigned char>(word[0]) <= KEY_PREFIX_WORD))
	return char(KEY_PREFIX_WORD) + word;
    return word;
}

struct fragment {
    char data[4];

    /// Default constructor.
    fragment() { }

    /// Zero-initialising constructor.
    explicit fragment(int) { std::memset(data, 0, 4); }

    /// Allow implicit conversion.
    explicit fragment(char data_[4]) { std::memcpy(data, data_, 4); }

    char & operator[] (unsigned i) { return data[i]; }
    const char & operator[] (unsigned i) const { return data[i]; }

    operator std::string() const {
	return std::string(data, data[0] == KEY_PREFIX_MIDDLE ? 4 : 3);
    }

    bool operator<(const fragment &b) const {
	return std::memcmp(data, b.data, 4) < 0;
    }
};

}

class HoneySpellingTable : public HoneyLazyTable {
    void toggle_word(const std::string & word);
    void toggle_fragment(Honey::fragment frag, const std::string & word);

    mutable std::map<std::string, Xapian::termcount> wordfreq_changes;

    /** Changes to make to the termlists.
     *
     *  This list is essentially xor-ed with the list on disk, so an entry
     *  here either means a new entry needs to be added on disk, or an
     *  existing entry on disk needs to be removed.  We do it this way so
     *  we don't need to store an additional add/remove flag for every
     *  word.
     */
    mutable std::map<Honey::fragment, std::set<std::string> > termlist_deltas;

    /** Used to track an upper bound on wordfreq. */
    Xapian::termcount wordfreq_upper_bound = 0;

  public:
    /** Create a new HoneySpellingTable object.
     *
     *  This method does not create or open the table on disk - you
     *  must call the create() or open() methods respectively!
     *
     *  @param dbdir		The directory the honey database is stored in.
     *  @param readonly		true if we're opening read-only, else false.
     */
    HoneySpellingTable(const std::string & dbdir, bool readonly)
	: HoneyLazyTable("spelling", dbdir + "/spelling.", readonly) { }

    HoneySpellingTable(int fd, off_t offset_, bool readonly)
	: HoneyLazyTable("spelling", fd, offset_, readonly) { }

    /** Merge in batched-up changes. */
    void merge_changes();

    void add_word(const std::string & word, Xapian::termcount freqinc);
    Xapian::termcount remove_word(const std::string & word,
				  Xapian::termcount freqdec);

    TermList * open_termlist(const std::string & word);

    Xapian::doccount get_word_frequency(const std::string & word) const;

    void set_wordfreq_upper_bound(Xapian::termcount ub) {
	wordfreq_upper_bound = ub;
    }

    /** Override methods of HoneyTable.
     *
     *  NB: these aren't virtual, but we always call them on the subclass in
     *  cases where it matters.
     *  @{
     */

    bool is_modified() const {
	return !wordfreq_changes.empty() || HoneyTable::is_modified();
    }

    /** Returns updated wordfreq upper bound. */
    Xapian::termcount flush_db() {
	merge_changes();
	HoneyTable::flush_db();
	return wordfreq_upper_bound;
    }

    void cancel(const Honey::RootInfo & root_info,
		honey_revision_number_t rev) {
	// Discard batched-up changes.
	wordfreq_changes.clear();
	termlist_deltas.clear();

	HoneyTable::cancel(root_info, rev);
    }

    // @}
};

/** The list of words containing a particular trigram. */
class HoneySpellingTermList : public TermList {
    /// The encoded data.
    std::string data;

    /// Position in the data.
    unsigned p = 0;

    /// The current term.
    std::string current_term;

    /** Number of constant characters on the end of the value.
     *
     *  Valid values once iterating are 0, 1, 2.  Before iteration, can be
     *  0 (no head or tail), 2 (two tails), -1 (one head, one tail -> 1 once
     *  iterating) or -2 (two heads, no tail -> 0 once iterating).
     */
    int tail = 0;

    /// Copying is not allowed.
    HoneySpellingTermList(const HoneySpellingTermList &);

    /// Assignment is not allowed.
    void operator=(const HoneySpellingTermList &);

  public:
    /// Constructor.
    explicit HoneySpellingTermList(const std::string& data_)
	: data(data_) { }

    /// Constructor for head/bookend/tail terms.
    HoneySpellingTermList(const std::string& data_,
			  const char* key)
	: data(data_) {
	unsigned char first_ch = key[0];
	AssertRel(first_ch, <, Honey::KEY_PREFIX_WORD);
	switch (first_ch) {
	    case Honey::KEY_PREFIX_BOOKEND:
		tail = -1;
		break;
	    case Honey::KEY_PREFIX_HEAD:
		tail = -2;
		break;
	    case Honey::KEY_PREFIX_TAIL:
		tail = 2;
		break;
	}
	if (tail != 0)
	    current_term.assign(key + 1, 2);
    }

    Xapian::termcount get_approx_size() const;

    std::string get_termname() const;

    Xapian::termcount get_wdf() const;

    Xapian::doccount get_termfreq() const;

    Xapian::termcount get_collection_freq() const;

    TermList * next();

    TermList * skip_to(const std::string & term);

    bool at_end() const;

    Xapian::termcount positionlist_count() const;

    PositionList* positionlist_begin() const;
};

#endif // XAPIAN_INCLUDED_HONEY_SPELLING_H
