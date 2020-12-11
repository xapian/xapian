/** @file
 * @brief Spelling correction data for a honey database.
 */
/* Copyright (C) 2004,2005,2006,2007,2008,2009,2010,2011,2015,2017,2018,2020 Olly Betts
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

#include <config.h>

#include <xapian/error.h>
#include <xapian/types.h>

#include "expand/expandweight.h"
#include "expand/termlistmerger.h"
#include "honey_spelling.h"
#include "omassert.h"
#include "pack.h"

#include "../prefix_compressed_strings.h"

#include <algorithm>
#include <map>
#include <queue>
#include <vector>
#include <set>
#include <string>

using namespace Honey;
using namespace std;

void
HoneySpellingTable::merge_changes()
{
    for (auto i : termlist_deltas) {
	const string& key = i.first;
	const set<string>& changes = i.second;

	auto d = changes.begin();
	if (d == changes.end()) continue;

	string updated;
	string current;
	PrefixCompressedStringWriter out(updated);
	if (get_exact_entry(key, current)) {
	    PrefixCompressedStringItor in(current, key);
	    updated.reserve(current.size()); // FIXME plus some?
	    while (!in.at_end() && d != changes.end()) {
		const string& word = *in;
		Assert(d != changes.end());
		int cmp = word.compare(*d);
		if (cmp < 0) {
		    out.append(word);
		    ++in;
		} else if (cmp > 0) {
		    out.append(*d);
		    ++d;
		} else {
		    // If an existing entry is in the changes list, that means
		    // we should remove it.
		    ++in;
		    ++d;
		}
	    }
	    if (!in.at_end()) {
		// FIXME : easy to optimise this to a fix-up and substring copy.
		while (!in.at_end()) {
		    out.append(*in++);
		}
	    }
	}
	while (d != changes.end()) {
	    out.append(*d++);
	}
	if (!updated.empty()) {
	    add(key, updated);
	} else {
	    del(key);
	}
    }
    termlist_deltas.clear();

    map<string, Xapian::termcount>::const_iterator j;
    for (j = wordfreq_changes.begin(); j != wordfreq_changes.end(); ++j) {
	const string& key = make_spelling_wordlist_key(j->first);
	Xapian::termcount wordfreq = j->second;
	if (wordfreq) {
	    string tag;
	    pack_uint_last(tag, wordfreq);
	    add(key, tag);
	    if (wordfreq > wordfreq_upper_bound)
		wordfreq_upper_bound = wordfreq;
	} else {
	    del(key);
	}
    }
    wordfreq_changes.clear();
}

void
HoneySpellingTable::toggle_fragment(fragment frag, const string& word)
{
    auto i = termlist_deltas.find(frag);
    if (i == termlist_deltas.end()) {
	i = termlist_deltas.insert(make_pair(frag, set<string>())).first;
    }
    // The commonest case is that we're adding lots of words, so try insert
    // first and if that reports that the word already exists, remove it.
    auto res = i->second.insert(word);
    if (!res.second) {
	// word is already in the set, so remove it.
	i->second.erase(res.first);
    }
}

void
HoneySpellingTable::add_word(const string& word, Xapian::termcount freqinc)
{
    if (word.size() <= 1) return;

    map<string, Xapian::termcount>::iterator i = wordfreq_changes.find(word);
    if (i != wordfreq_changes.end()) {
	// Word "word" already exists and has been modified.
	if (i->second) {
	    i->second += freqinc;
	    return;
	}
	// If "word" is currently modified such that it no longer exists, so
	// we need to execute the code below to re-add trigrams for it.
	i->second = freqinc;
    } else {
	string data;
	if (get_exact_entry(make_spelling_wordlist_key(word), data)) {
	    // Word "word" already exists, so increment its count.
	    Xapian::termcount freq;
	    const char* p = data.data();
	    if (!unpack_uint_last(&p, p + data.size(), &freq) || freq == 0) {
		throw Xapian::DatabaseCorruptError("Bad spelling word freq");
	    }
	    wordfreq_changes[word] = freq + freqinc;
	    return;
	}
	wordfreq_changes[word] = freqinc;
    }

    // Add trigrams for word.
    toggle_word(word);
}

Xapian::termcount
HoneySpellingTable::remove_word(const string& word, Xapian::termcount freqdec)
{
    if (word.size() <= 1) return freqdec;

    map<string, Xapian::termcount>::iterator i = wordfreq_changes.find(word);
    if (i != wordfreq_changes.end()) {
	if (i->second == 0) {
	    // Word has already been deleted.
	    return freqdec;
	}
	// Word "word" exists and has been modified.
	if (freqdec < i->second) {
	    i->second -= freqdec;
	    return 0;
	}
	freqdec -= i->second;

	// Mark word as deleted.
	i->second = 0;
    } else {
	string data;
	if (!get_exact_entry(make_spelling_wordlist_key(word), data)) {
	    // This word doesn't exist.
	    return freqdec;
	}

	Xapian::termcount freq;
	const char* p = data.data();
	if (!unpack_uint_last(&p, p + data.size(), &freq)) {
	    throw Xapian::DatabaseCorruptError("Bad spelling word freq");
	}
	if (freqdec < freq) {
	    wordfreq_changes[word] = freq - freqdec;
	    return 0;
	}
	freqdec -= freq;

	// Mark word as deleted.
	wordfreq_changes[word] = 0;
    }

    // Remove trigrams for word.
    toggle_word(word);

    return freqdec;
}

void
HoneySpellingTable::toggle_word(const string& word)
{
    fragment buf(0);

    if (word.size() <= 4) {
	// We also generate 'bookends' for two, three, and four character
	// terms so we can handle transposition of the middle two characters
	// of a four character word, substitution or deletion of the middle
	// character of a three character word, or insertion in the middle of a
	// two character word.
	// 'Bookends':
	buf[0] = KEY_PREFIX_BOOKEND;
	buf[1] = word[0];
	buf[2] = word[word.size() - 1];
	toggle_fragment(buf, word);
    }

    // Head:
    buf[0] = KEY_PREFIX_HEAD;
    buf[1] = word[0];
    buf[2] = word[1];
    toggle_fragment(buf, word);

    // Tail:
    buf[0] = KEY_PREFIX_TAIL;
    buf[1] = word[word.size() - 2];
    buf[2] = word[word.size() - 1];
    toggle_fragment(buf, word);

    if (word.size() > 2) {
	set<fragment> done;
	// Middles:
	buf[0] = KEY_PREFIX_MIDDLE;
	for (size_t start = 0; start <= word.size() - 3; ++start) {
	    memcpy(buf.data + 1, word.data() + start, 3);
	    // Don't toggle the same fragment twice or it will cancel out.
	    // Bug fixed in 1.2.6.
	    if (done.insert(buf).second)
		toggle_fragment(buf, word);
	}
    }
}

struct TermListGreaterApproxSize {
    bool operator()(const TermList* a, const TermList* b) const {
	return a->get_approx_size() > b->get_approx_size();
    }
};

TermList*
HoneySpellingTable::open_termlist(const string& word)
{
    // This should have been handled by Database::get_spelling_suggestion().
    AssertRel(word.size(),>,1);

    // Merge any pending changes to disk, but don't call commit() so they
    // won't be switched live.
    if (!wordfreq_changes.empty()) merge_changes();

    vector<TermList*> termlists;
    try {
	string data;
	fragment buf(0);

	if (word.size() <= 4) {
	    // We also generate 'bookends' for two, three, and four character
	    // terms so we can handle transposition of the middle two
	    // characters of a four character word, substitution or deletion of
	    // the middle character of a three character word, or insertion in
	    // the middle of a two character word.
	    buf[0] = KEY_PREFIX_BOOKEND;
	    buf[1] = word[0];
	    buf[2] = word[word.size() - 1];
	    if (get_exact_entry(string(buf), data))
		termlists.push_back(new HoneySpellingTermList(data, buf.data));
	}

	// Head:
	buf[0] = KEY_PREFIX_HEAD;
	buf[1] = word[0];
	buf[2] = word[1];
	if (get_exact_entry(string(buf), data))
	    termlists.push_back(new HoneySpellingTermList(data, buf.data));

	if (word.size() == 2) {
	    // For two letter words, we generate H and T terms for the
	    // transposed form so that we can produce good spelling
	    // suggestions.
	    // AB -> BA
	    buf[1] = word[1];
	    buf[2] = word[0];
	    if (get_exact_entry(string(buf), data))
		termlists.push_back(new HoneySpellingTermList(data, buf.data));
	    buf[0] = KEY_PREFIX_TAIL;
	    if (get_exact_entry(string(buf), data))
		termlists.push_back(new HoneySpellingTermList(data, buf.data));
	}

	// Tail:
	buf[0] = KEY_PREFIX_TAIL;
	buf[1] = word[word.size() - 2];
	buf[2] = word[word.size() - 1];
	if (get_exact_entry(string(buf), data))
	    termlists.push_back(new HoneySpellingTermList(data, buf.data));

	if (word.size() > 2) {
	    // Middles:
	    buf[0] = KEY_PREFIX_MIDDLE;
	    for (size_t start = 0; start <= word.size() - 3; ++start) {
		memcpy(buf.data + 1, word.data() + start, 3);
		if (get_exact_entry(string(buf), data))
		    termlists.push_back(new HoneySpellingTermList(data));
	    }

	    if (word.size() == 3) {
		// For three letter words, we generate the two "single
		// transposition" forms too, so that we can produce good
		// spelling suggestions.
		// ABC -> BAC
		buf[1] = word[1];
		buf[2] = word[0];
		if (get_exact_entry(string(buf), data))
		    termlists.push_back(new HoneySpellingTermList(data));
		// ABC -> ACB
		buf[1] = word[0];
		buf[2] = word[2];
		buf[3] = word[1];
		if (get_exact_entry(string(buf), data))
		    termlists.push_back(new HoneySpellingTermList(data));
	    }
	}

	return make_termlist_merger(termlists);
    } catch (...) {
	// Make sure we delete all the TermList objects to avoid leaking
	// memory.
	for (auto& t : termlists) {
	    delete t;
	}
	throw;
    }
}

Xapian::doccount
HoneySpellingTable::get_word_frequency(const string& word) const
{
    map<string, Xapian::termcount>::const_iterator i;
    i = wordfreq_changes.find(word);
    if (i != wordfreq_changes.end()) {
	// Modified frequency for word:
	return i->second;
    }

    string data;
    if (get_exact_entry(make_spelling_wordlist_key(word), data)) {
	// Word "word" already exists.
	Xapian::termcount freq;
	const char* p = data.data();
	if (!unpack_uint_last(&p, p + data.size(), &freq)) {
	    throw Xapian::DatabaseCorruptError("Bad spelling word freq");
	}
	return freq;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////

Xapian::termcount
HoneySpellingTermList::get_approx_size() const
{
    // This is only used to decide how to build a OR-tree of TermList objects
    // so we just need to return "sizes" which are ordered roughly correctly.
    return data.size();
}

std::string
HoneySpellingTermList::get_termname() const
{
    return current_term;
}

Xapian::termcount
HoneySpellingTermList::get_wdf() const
{
    return 1;
}

Xapian::doccount
HoneySpellingTermList::get_termfreq() const
{
    return 1;
}

TermList*
HoneySpellingTermList::next()
{
    if (p == data.size()) {
	p = 0;
	data.resize(0);
	return NULL;
    }

    size_t keep = 0;
    if (rare(tail < 0)) {
	tail += 2;
	keep = current_term.size() - tail;
    } else if (usual(!current_term.empty())) {
	keep = data[p++] ^ MAGIC_XOR_VALUE;
    }
    size_t add;
    if (p == data.size() ||
	(add = data[p] ^ MAGIC_XOR_VALUE) >= data.size() - p) {
	throw Xapian::DatabaseCorruptError("Bad spelling data (too little "
					   "left)");
    }
    current_term.replace(keep, current_term.size() - tail - keep,
			 data.data() + p + 1, add);
    p += add + 1;

    return NULL;
}

TermList*
HoneySpellingTermList::skip_to(const string& term)
{
    while (!data.empty() && current_term < term) {
	(void)HoneySpellingTermList::next();
    }
    return NULL;
}

bool
HoneySpellingTermList::at_end() const
{
    return data.empty();
}

Xapian::termcount
HoneySpellingTermList::positionlist_count() const
{
    throw
	Xapian::UnimplementedError("HoneySpellingTermList::"
				   "positionlist_count() "
				   "not implemented");
}

PositionList*
HoneySpellingTermList::positionlist_begin() const
{
    throw
	Xapian::UnimplementedError("HoneySpellingTermList::"
				   "positionlist_begin() "
				   "not implemented");
}
