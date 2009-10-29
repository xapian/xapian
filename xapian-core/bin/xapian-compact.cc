/* xapian-compact.cc: Compact a flint or chert database, or merge and compact several.
 *
 * Copyright (C) 2004,2005,2006,2007,2008,2009 Olly Betts
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

#include <config.h>

#include "safeerrno.h"

#include <fstream>
#include <iostream>
#include <queue>

#include <cstdio> // for rename()
#include <cstring>
#include <sys/types.h>
#include "utils.h"
#include "valuestats.h"

#include "internaltypes.h"

// FIXME: this currently works for chert, but we are really going to need
// separate versions eventually.
#include "flint_table.h"
#include "flint_cursor.h"
#include "flint_utils.h"

#include <xapian.h>

#include "gnu_getopt.h"

using namespace std;

#define PROG_NAME "xapian-compact"
#define PROG_DESC "Compact a flint or chert database, or merge and compact several"

#define OPT_HELP 1
#define OPT_VERSION 2
#define OPT_NO_RENUMBER 3

static void show_usage() {
    cout << "Usage: "PROG_NAME" [OPTIONS] SOURCE_DATABASE... DESTINATION_DATABASE\n\n"
"Options:\n"
"  -b, --blocksize   Set the blocksize in bytes (e.g. 4096) or K (e.g. 4K)\n"
"                    (must be between 2K and 64K and a power of 2, default 8K)\n"
"  -n, --no-full     Disable full compaction\n"
"  -F, --fuller      Enable fuller compaction (not recommended if you plan to\n"
"                    update the compacted database)\n"
"  -m, --multipass   If merging more than 3 databases, merge the postlists in\n"
"                    multiple passes (which is generally faster but requires\n"
"                    more disk space for temporary files)\n"
"      --no-renumber Preserve the numbering of document ids (useful if you\n"
"                    external references to them, or have set the to match\n"
"                    unique ids from an external source).  Currently this\n"
"                    option isn't supported when merging databases.\n"
"  --help            display this help and exit\n"
"  --version         output version information and exit" << endl;
}

static inline bool
is_metainfo_key(const string & key)
{
    return key.size() == 1 && key[0] == '\0';
}

static inline bool
is_user_metadata_key(const string & key)
{
    return key.size() > 1 && key[0] == '\0' && key[1] == '\xc0';
}

static inline bool
is_valuestats_key(const string & key)
{
    return key.size() > 1 && key[0] == '\0' && key[1] == '\xd0';
}

static inline bool
is_valuechunk_key(const string & key)
{
    return key.size() > 1 && key[0] == '\0' && key[1] == '\xd8';
}

class PostlistCursor : private FlintCursor {
    Xapian::docid offset;

  public:
    string key, tag;
    Xapian::docid firstdid;
    Xapian::termcount tf, cf;

    PostlistCursor(FlintTable *in, Xapian::docid offset_)
	: FlintCursor(in), offset(offset_), firstdid(0)
    {
	find_entry("");
	next();
    }

    ~PostlistCursor()
    {
	delete FlintCursor::get_table();
    }

    bool next() {
	if (!FlintCursor::next()) return false;
	// We put all chunks into the non-initial chunk form here, then fix up
	// the first chunk for each term in the merged database as we merge.
	read_tag();
	key = current_key;
	tag = current_tag;
	tf = cf = 0;
	if (is_metainfo_key(key)) return true;
	if (is_user_metadata_key(key)) return true;
	if (is_valuestats_key(key)) return true;
	if (is_valuechunk_key(key)) {
	    const char * p = key.data();
	    const char * end = p + key.length();
	    p += 2;
	    Xapian::valueno slot;
	    if (!F_unpack_uint(&p, end, &slot))
		throw Xapian::DatabaseCorruptError("bad value key");
	    Xapian::docid did;
	    if (!F_unpack_uint_preserving_sort(&p, end, &did))
		throw Xapian::DatabaseCorruptError("bad value key");
	    did += offset;
	    key = string("\0\xd8", 2) + F_pack_uint(slot) + F_pack_uint_preserving_sort(did);
	    return true;
	}
	// Adjust key if this is *NOT* an initial chunk.
	// key is: F_pack_string_preserving_sort(tname)
	// plus optionally: F_pack_uint_preserving_sort(did)
	const char * d = key.data();
	const char * e = d + key.size();
	string tname;
	if (!F_unpack_string_preserving_sort(&d, e, tname))
	    throw Xapian::DatabaseCorruptError("Bad postlist key");
	if (d == e) {
	    // This is an initial chunk for a term, so adjust tag header.
	    d = tag.data();
	    e = d + tag.size();
	    if (!F_unpack_uint(&d, e, &tf) ||
		!F_unpack_uint(&d, e, &cf) ||
		!F_unpack_uint(&d, e, &firstdid)) {
		throw Xapian::DatabaseCorruptError("Bad postlist tag");
	    }
	    ++firstdid;
	    tag.erase(0, d - tag.data());
	} else {
	    // Not an initial chunk, so adjust key.
	    size_t tmp = d - key.data();
	    if (!F_unpack_uint_preserving_sort(&d, e, &firstdid) || d != e)
		throw Xapian::DatabaseCorruptError("Bad postlist key");
	    key.erase(tmp);
	}
	firstdid += offset;
	return true;
    }
};

class PostlistCursorGt {
  public:
    /** Return true if and only if a's key is strictly greater than b's key.
     */
    bool operator()(const PostlistCursor *a, const PostlistCursor *b) {
	if (a->key > b->key) return true;
	if (a->key != b->key) return false;
	return (a->firstdid > b->firstdid);
    }
};

static string
encode_valuestats(Xapian::doccount freq,
		  const string & lbound, const string & ubound)
{
    string value = F_pack_uint(freq);
    value += F_pack_string(lbound);
    // We don't store or count empty values, so neither of the bounds
    // can be empty.  So we can safely store an empty upper bound when
    // the bounds are equal.
    if (lbound != ubound) value += ubound;
    return value;
}

static void
merge_postlists(FlintTable * out, vector<Xapian::docid>::const_iterator offset,
		vector<string>::const_iterator b, vector<string>::const_iterator e,
		Xapian::docid tot_off, bool is_chert)
{
    totlen_t tot_totlen = 0;
    Xapian::termcount doclen_lbound = static_cast<Xapian::termcount>(-1);
    Xapian::termcount wdf_ubound = 0;
    Xapian::termcount doclen_ubound = 0;
    priority_queue<PostlistCursor *, vector<PostlistCursor *>, PostlistCursorGt> pq;
    for ( ; b != e; ++b, ++offset) {
	FlintTable *in = new FlintTable("postlist", *b, true);
	in->open();
	if (!in->get_entry_count()) {
	    // Skip empty tables.
	    delete in;
	    continue;
	}

	// PostlistCursor takes ownership of FlintTable in and is
	// responsible for deleting it.
	PostlistCursor * cur = new PostlistCursor(in, *offset);
	// Merge the METAINFO tags from each database into one.
	// They have a key consisting of a single zero byte.
	// They may be absent, if the database contains no documents.  If it
	// has user metadata we'll still get here.
	if (is_metainfo_key(cur->key)) {
	    const char * data = cur->tag.data();
	    const char * end = data + cur->tag.size();
	    Xapian::docid dummy_did = 0;
	    if (!F_unpack_uint(&data, end, &dummy_did)) {
		throw Xapian::DatabaseCorruptError("Tag containing meta information is corrupt.");
	    }
	    if (is_chert) {
		Xapian::termcount doclen_lbound_tmp;
		if (!F_unpack_uint(&data, end, &doclen_lbound_tmp)) {
		    throw Xapian::DatabaseCorruptError("Tag containing meta information is corrupt.");
		}
		doclen_lbound = min(doclen_lbound, doclen_lbound_tmp);

		Xapian::termcount wdf_ubound_tmp;
		if (!F_unpack_uint(&data, end, &wdf_ubound_tmp)) {
		    throw Xapian::DatabaseCorruptError("Tag containing meta information is corrupt.");
		}
		wdf_ubound = max(wdf_ubound, wdf_ubound_tmp);

		Xapian::termcount doclen_ubound_tmp;
		if (!F_unpack_uint(&data, end, &doclen_ubound_tmp)) {
		    throw Xapian::DatabaseCorruptError("Tag containing meta information is corrupt.");
		}
		doclen_ubound_tmp += wdf_ubound_tmp;
		doclen_ubound = max(doclen_ubound, doclen_ubound_tmp);
	    }
	    totlen_t totlen = 0;
	    if (!F_unpack_uint_last(&data, end, &totlen)) {
		throw Xapian::DatabaseCorruptError("Tag containing meta information is corrupt.");
	    }
	    tot_totlen += totlen;
	    if (tot_totlen < totlen) {
		throw "totlen wrapped!";
	    }
	}
	if (cur->next()) {
	    pq.push(cur);
	} else {
	    delete cur;
	}
    }

    {
	string tag = F_pack_uint(tot_off);
	if (is_chert) {
	    tag += F_pack_uint(doclen_lbound);
	    tag += F_pack_uint(wdf_ubound);
	    tag += F_pack_uint(doclen_ubound - wdf_ubound);
	}
	tag += F_pack_uint_last(tot_totlen);
	out->add(string("", 1), tag);
    }

    string last_key;
    {
	// Merge user metadata.
	string last_tag;
	while (!pq.empty()) {
	    PostlistCursor * cur = pq.top();
	    const string& key = cur->key;
	    if (!is_user_metadata_key(key)) break;

	    const string & tag = cur->tag;
	    if (key == last_key) {
		if (tag != last_tag)
		    cerr << "Warning: duplicate user metadata key with different tag value - picking arbitrary tag value" << endl;
	    } else {
		out->add(key, tag);
		last_key = key;
		last_tag = tag;
	    }

	    pq.pop();
	    if (cur->next()) {
		pq.push(cur);
	    } else {
		delete cur;
	    }
	}
    }

    {
	// Merge valuestats.
	Xapian::doccount freq = 0;
	string lbound, ubound;

	string last_tag;
	while (!pq.empty()) {
	    PostlistCursor * cur = pq.top();
	    const string& key = cur->key;
	    if (!is_valuestats_key(key)) break;
	    if (key != last_key) {
		// For the first valuestats key, last_key will be the previous
		// key we wrote, which we don't want to overwrite.  This is the
		// only time that freq will be 0, so check that.
		if (freq) {
		    out->add(last_key, encode_valuestats(freq, lbound, ubound));
		    freq = 0;
		}
		last_key = key;
	    }

	    const string & tag = cur->tag;

	    const char * pos = tag.data();
	    const char * end = pos + tag.size();

	    Xapian::doccount f;
	    string l, u;
	    if (!F_unpack_uint(&pos, end, &f)) {
		if (*pos == 0) throw Xapian::DatabaseCorruptError("Incomplete stats item in value table");
		throw Xapian::RangeError("Frequency statistic in value table is too large");
	    }
	    if (!F_unpack_string(&pos, end, l)) {
		if (*pos == 0) throw Xapian::DatabaseCorruptError("Incomplete stats item in value table");
		throw Xapian::RangeError("Lower bound in value table is too large");
	    }
	    size_t len = end - pos;
	    if (len == 0) {
		u = l;
	    } else {
		u.assign(pos, len);
	    }
	    if (freq == 0) {
		freq = f;
		lbound = l;
		ubound = u;
	    } else {
		freq += f;
		if (l < lbound) lbound = l;
		if (u > ubound) ubound = u;
	    }

	    pq.pop();
	    if (cur->next()) {
		pq.push(cur);
	    } else {
		delete cur;
	    }
	}

	if (freq) {
	    out->add(last_key, encode_valuestats(freq, lbound, ubound));
	}
    }

    // Merge valuestream chunks.
    while (!pq.empty()) {
	PostlistCursor * cur = pq.top();
	const string & key = cur->key;
	if (!is_valuechunk_key(key)) break;
	Assert(!is_user_metadata_key(key));
	out->add(key, cur->tag);
	pq.pop();
	if (cur->next()) {
	    pq.push(cur);
	} else {
	    delete cur;
	}
    }

    Xapian::termcount tf = 0, cf = 0; // Initialise to avoid warnings.
    vector<pair<Xapian::docid, string> > tags;
    while (true) {
	PostlistCursor * cur = NULL;
	if (!pq.empty()) {
	    cur = pq.top();
	    pq.pop();
	}
	Assert(cur == NULL || !is_user_metadata_key(cur->key));
	if (cur == NULL || cur->key != last_key) {
	    if (!tags.empty()) {
		string first_tag = F_pack_uint(tf);
		first_tag += F_pack_uint(cf);
		first_tag += F_pack_uint(tags[0].first - 1);
		string tag = tags[0].second;
		tag[0] = (tags.size() == 1) ? '1' : '0';
		first_tag += tag;
		out->add(last_key, first_tag);
		vector<pair<Xapian::docid, string> >::const_iterator i;
		i = tags.begin();
		while (++i != tags.end()) {
		    string new_key = last_key;
		    new_key += F_pack_uint_preserving_sort(i->first);
		    tag = i->second;
		    tag[0] = (i + 1 == tags.end()) ? '1' : '0';
		    out->add(new_key, tag);
		}
	    }
	    tags.clear();
	    if (cur == NULL) break;
	    tf = cf = 0;
	    last_key = cur->key;
	}
	tf += cur->tf;
	cf += cur->cf;
	tags.push_back(make_pair(cur->firstdid, cur->tag));
	if (cur->next()) {
	    pq.push(cur);
	} else {
	    delete cur;
	}
    }
}

struct MergeCursor : public FlintCursor {
    MergeCursor(FlintTable *in) : FlintCursor(in) {
	find_entry("");
	next();
    }

    ~MergeCursor() {
	delete FlintCursor::get_table();
    }
};

struct CursorGt {
    /// Return true if and only if a's key is strictly greater than b's key.
    bool operator()(const FlintCursor *a, const FlintCursor *b) {
	if (b->after_end()) return false;
	if (a->after_end()) return true;
	return (a->current_key > b->current_key);
    }
};

#define MAGIC_XOR_VALUE 96

// FIXME: copied from backends/flint/flint_spelling.cc.
class PrefixCompressedStringItor {
    const unsigned char * p;
    size_t left;
    string current;

    PrefixCompressedStringItor(const unsigned char * p_, size_t left_,
			       const string &current_)
	: p(p_), left(left_), current(current_) { }

  public:
    PrefixCompressedStringItor(const std::string & s)
	: p(reinterpret_cast<const unsigned char *>(s.data())),
	  left(s.size()) {
	if (left) {
	    operator++();
	} else {
	    p = NULL;
	}
    }

    const string & operator*() const {
	return current;
    }

    PrefixCompressedStringItor operator++(int) {
	const unsigned char * old_p = p;
	size_t old_left = left;
	string old_current = current;
	operator++();
	return PrefixCompressedStringItor(old_p, old_left, old_current);
    }

    PrefixCompressedStringItor & operator++() {
	if (left == 0) {
	    p = NULL;
	} else {
	    if (!current.empty()) {
		current.resize(*p++ ^ MAGIC_XOR_VALUE);
		--left;
	    }
	    size_t add;
	    if (left == 0 || (add = *p ^ MAGIC_XOR_VALUE) >= left)
		throw Xapian::DatabaseCorruptError("Bad spelling data (too little left)");
	    current.append(reinterpret_cast<const char *>(p + 1), add);
	    p += add + 1;
	    left -= add + 1;
	}
	return *this;
    }

    bool at_end() const {
	return p == NULL;
    }
};

// FIXME: copied from backends/flint/flint_spelling.cc.
class PrefixCompressedStringWriter {
    string current;
    string & out;

  public:
    PrefixCompressedStringWriter(string & out_) : out(out_) { }

    void append(const string & word) {
	// If this isn't the first entry, see how much of the previous one
	// we can reuse.
	if (!current.empty()) {
	    size_t len = min(current.size(), word.size());
	    size_t i;
	    for (i = 0; i < len; ++i) {
		if (current[i] != word[i]) break;
	    }
	    out += char(i ^ MAGIC_XOR_VALUE);
	    out += char((word.size() - i) ^ MAGIC_XOR_VALUE);
	    out.append(word.data() + i, word.size() - i);
	} else {
	    out += char(word.size() ^ MAGIC_XOR_VALUE);
	    out += word;
	}
	current = word;
    }
};

struct PrefixCompressedStringItorGt {
    /// Return true if and only if a's string is strictly greater than b's.
    bool operator()(const PrefixCompressedStringItor *a,
		    const PrefixCompressedStringItor *b) {
	return (**a > **b);
    }
};

static void
merge_spellings(FlintTable * out,
		vector<string>::const_iterator b,
		vector<string>::const_iterator e)
{
    priority_queue<MergeCursor *, vector<MergeCursor *>, CursorGt> pq;
    for ( ; b != e; ++b) {
	FlintTable *in = new FlintTable("spelling", *b, true, DONT_COMPRESS, true);
	in->open();
	if (in->get_entry_count()) {
	    // The MergeCursor takes ownership of FlintTable in and is
	    // responsible for deleting it.
	    pq.push(new MergeCursor(in));
	} else {
	    delete in;
	}
    }

    while (!pq.empty()) {
	MergeCursor * cur = pq.top();
	pq.pop();

	string key = cur->current_key;
	if (pq.empty() || pq.top()->current_key > key) {
	    // No need to merge the tags, just copy the (possibly compressed)
	    // tag value.
	    bool compressed = cur->read_tag(true);
	    out->add(key, cur->current_tag, compressed);
	    if (cur->next()) {
		pq.push(cur);
	    } else {
		delete cur;
	    }
	    continue;
	}

	// Merge tag values with the same key:
	string tag;
	if (key[0] != 'W') {
	    // We just want the union of words, so copy over the first instance
	    // and skip any identical ones.
	    priority_queue<PrefixCompressedStringItor *,
			   vector<PrefixCompressedStringItor *>,
			   PrefixCompressedStringItorGt> pqtag;
	    // Stick all the MergeCursor pointers in a vector because their
	    // current_tag members must remain valid while we're merging their
	    // tags, but we need to call next() on them all afterwards.
	    vector<MergeCursor *> vec;
	    vec.reserve(pq.size());

	    while (true) {
		cur->read_tag();
		pqtag.push(new PrefixCompressedStringItor(cur->current_tag));
		vec.push_back(cur);
		if (pq.empty() || pq.top()->current_key != key) break;
		cur = pq.top();
		pq.pop();
	    }

	    PrefixCompressedStringWriter wr(tag);
	    string lastword;
	    while (!pqtag.empty()) {
		PrefixCompressedStringItor * it = pqtag.top();
		string word = **it;
		if (word != lastword) {
		    lastword = word;
		    wr.append(lastword);
		}
		++*it;
		pqtag.pop();
		if (!it->at_end()) {
		    pqtag.push(it);
		} else {
		    delete it;
		}
	    }

	    vector<MergeCursor *>::const_iterator i;
	    for (i = vec.begin(); i != vec.end(); ++i) {
		cur = *i;
		if (cur->next()) {
		    pq.push(cur);
		} else {
		    delete cur;
		}
	    }
	} else {
	    // We want to sum the frequencies from tags for the same key.
	    Xapian::termcount tot_freq = 0;
	    while (true) {
		cur->read_tag();
		Xapian::termcount freq;
		const char * p = cur->current_tag.data();
		const char * end = p + cur->current_tag.size();
		if (!F_unpack_uint_last(&p, end, &freq) || freq == 0) {
		    throw Xapian::DatabaseCorruptError("Bad spelling word freq");
		}
		tot_freq += freq;
		if (cur->next()) {
		    pq.push(cur);
		} else {
		    delete cur;
		}
		if (pq.empty() || pq.top()->current_key != key) break;
		cur = pq.top();
		pq.pop();
	    }
	    tag = F_pack_uint_last(tot_freq);
	}
	out->add(key, tag);
    }
}

class ByteLengthPrefixedStringItor {
    const unsigned char * p;
    size_t left;

    ByteLengthPrefixedStringItor(const unsigned char * p_, size_t left_)
	: p(p_), left(left_) { }

  public:
    ByteLengthPrefixedStringItor(const std::string & s)
	: p(reinterpret_cast<const unsigned char *>(s.data())),
	  left(s.size()) { }

    string operator*() const {
	size_t len = *p ^ MAGIC_XOR_VALUE;
	return string(reinterpret_cast<const char *>(p + 1), len);
    }

    ByteLengthPrefixedStringItor operator++(int) {
	const unsigned char * old_p = p;
	size_t old_left = left;
	operator++();
	return ByteLengthPrefixedStringItor(old_p, old_left);
    }

    ByteLengthPrefixedStringItor & operator++() {
	if (!left) {
	    throw Xapian::DatabaseCorruptError("Bad synonym data (none left)");
	}
	size_t add = (*p ^ MAGIC_XOR_VALUE) + 1;
	if (left < add) {
	    throw Xapian::DatabaseCorruptError("Bad synonym data (too little left)");
	}
	p += add;
	left -= add;
	return *this;
    }

    bool at_end() const {
	return left == 0;
    }
};

struct ByteLengthPrefixedStringItorGt {
    /// Return true if and only if a's string is strictly greater than b's.
    bool operator()(const ByteLengthPrefixedStringItor *a,
		    const ByteLengthPrefixedStringItor *b) {
	return (**a > **b);
    }
};

static void
merge_synonyms(FlintTable * out,
	       vector<string>::const_iterator b,
	       vector<string>::const_iterator e)
{
    priority_queue<MergeCursor *, vector<MergeCursor *>, CursorGt> pq;
    for ( ; b != e; ++b) {
	FlintTable *in = new FlintTable("synonym", *b, true, DONT_COMPRESS, true);
	in->open();
	if (in->get_entry_count()) {
	    // The MergeCursor takes ownership of FlintTable in and is
	    // responsible for deleting it.
	    pq.push(new MergeCursor(in));
	} else {
	    delete in;
	}
    }

    while (!pq.empty()) {
	MergeCursor * cur = pq.top();
	pq.pop();

	string key = cur->current_key;
	if (pq.top()->current_key > key) {
	    // No need to merge the tags, just copy the (possibly compressed)
	    // tag value.
	    bool compressed = cur->read_tag(true);
	    out->add(key, cur->current_tag, compressed);
	    if (cur->next()) {
		pq.push(cur);
	    } else {
		delete cur;
	    }
	    continue;
	}

	// Merge tag values with the same key:
	string tag;

	// We just want the union of words, so copy over the first instance
	// and skip any identical ones.
	priority_queue<ByteLengthPrefixedStringItor *,
		       vector<ByteLengthPrefixedStringItor *>,
		       ByteLengthPrefixedStringItorGt> pqtag;
	vector<MergeCursor *> vec;

	while (true) {
	    cur->read_tag();
	    pqtag.push(new ByteLengthPrefixedStringItor(cur->current_tag));
	    vec.push_back(cur);
	    if (pq.empty() || pq.top()->current_key != key) break;
	    cur = pq.top();
	    pq.pop();
	}

	string lastword;
	while (!pqtag.empty()) {
	    ByteLengthPrefixedStringItor * it = pqtag.top();
	    if (**it != lastword) {
		lastword = **it;
		tag += byte(lastword.size() ^ MAGIC_XOR_VALUE);
		tag += lastword;
	    }
	    ++*it;
	    pqtag.pop();
	    if (!it->at_end()) {
		pqtag.push(it);
	    } else {
		delete it;
	    }
	}

	vector<MergeCursor *>::const_iterator i;
	for (i = vec.begin(); i != vec.end(); ++i) {
	    cur = *i;
	    if (cur->next()) {
		pq.push(cur);
	    } else {
		delete cur;
	    }
	}

	out->add(key, tag);
    }
}

static void
multimerge_postlists(FlintTable * out, const char * tmpdir,
		     Xapian::docid tot_off,
		     vector<string> tmp, vector<Xapian::docid> off,
		     bool is_chert)
{
    unsigned int c = 0;
    while (tmp.size() > 3) {
	vector<string> tmpout;
	tmpout.reserve(tmp.size() / 2);
	vector<Xapian::docid> newoff;
	newoff.resize(tmp.size() / 2);
	for (unsigned int i = 0, j; i < tmp.size(); i = j) {
	    j = i + 2;
	    if (j == tmp.size() - 1) ++j;

	    string dest = tmpdir;
	    char buf[64];
	    sprintf(buf, "/tmp%u_%u.", c, i / 2);
	    dest += buf;

	    // Don't compress temporary tables, even if the final table would
	    // be.
	    FlintTable tmptab("postlist", dest, false);
	    // Use maximum blocksize for temporary tables.
	    tmptab.create_and_open(65536);

	    merge_postlists(&tmptab, off.begin() + i, tmp.begin() + i, tmp.begin() + j, 0, is_chert);
	    if (c > 0) {
		for (unsigned int k = i; k < j; ++k) {
		    unlink((tmp[k] + "DB").c_str());
		    unlink((tmp[k] + "baseA").c_str());
		    unlink((tmp[k] + "baseB").c_str());
		}
	    }
	    tmpout.push_back(dest);
	    tmptab.flush_db();
	    tmptab.commit(1);
	}
	swap(tmp, tmpout);
	swap(off, newoff);
	++c;
    }
    merge_postlists(out, off.begin(), tmp.begin(), tmp.end(), tot_off, is_chert);
    if (c > 0) {
	for (size_t k = 0; k < tmp.size(); ++k) {
	    unlink((tmp[k] + "DB").c_str());
	    unlink((tmp[k] + "baseA").c_str());
	    unlink((tmp[k] + "baseB").c_str());
	}
    }
}

static void
merge_docid_keyed(const char * tablename,
		  FlintTable *out, const vector<string> & inputs,
		  const vector<Xapian::docid> & offset, bool lazy)
{
    for (size_t i = 0; i < inputs.size(); ++i) {
	Xapian::docid off = offset[i];

	FlintTable in(tablename, inputs[i], true, DONT_COMPRESS, lazy);
	in.open();
	if (in.get_entry_count() == 0) continue;

	FlintCursor cur(&in);
	cur.find_entry("");

	string key;
	while (cur.next()) {
	    // Adjust the key if this isn't the first database.
	    if (off) {
		Xapian::docid did;
		const char * d = cur.current_key.data();
		const char * e = d + cur.current_key.size();
		if (!F_unpack_uint_preserving_sort(&d, e, &did)) {
		    string msg = "Bad key in ";
		    msg += inputs[i];
		    throw Xapian::DatabaseCorruptError(msg);
		}
		did += off;
		key = F_pack_uint_preserving_sort(did);
		if (d != e) {
		    // Copy over the termname for the position table.
		    key.append(d, e - d);
		}
	    } else {
		key = cur.current_key;
	    }
	    bool compressed = cur.read_tag(true);
	    out->add(key, cur.current_tag, compressed);
	}
    }
}

int
main(int argc, char **argv)
{
    const char * opts = "b:nFm";
    const struct option long_opts[] = {
	{"fuller",	no_argument, 0, 'F'},
	{"no-full",	no_argument, 0, 'n'},
	{"multipass",	no_argument, 0, 'm'},
	{"blocksize",	required_argument, 0, 'b'},
	{"no-renumber", no_argument, 0, OPT_NO_RENUMBER},
	{"help",	no_argument, 0, OPT_HELP},
	{"version",	no_argument, 0, OPT_VERSION},
	{NULL,		0, 0, 0}
    };

    enum { STANDARD, FULL, FULLER } compaction = FULL;
    size_t block_size = 8192;
    bool multipass = false;
    bool renumber = true;

    int c;
    while ((c = gnu_getopt_long(argc, argv, opts, long_opts, 0)) != -1) {
	switch (c) {
	    case 'b': {
		char *p;
		block_size = strtoul(optarg, &p, 10);
		if (block_size <= 64 && (*p == 'K' || *p == 'k')) {
		    ++p;
		    block_size *= 1024;
		}
		if (*p || block_size < 2048 || block_size > 65536 ||
		    (block_size & (block_size - 1)) != 0) {
		    cerr << PROG_NAME": Bad value '" << optarg
			 << "' passed for blocksize, must be a power of 2 between 2K and 64K"
			 << endl;
		    exit(1);
		}
		break;
	    }
	    case 'n':
		compaction = STANDARD;
		break;
	    case 'F':
		compaction = FULLER;
		break;
	    case 'm':
		multipass = true;
		break;
	    case OPT_NO_RENUMBER:
		renumber = false;
		break;
	    case OPT_HELP:
		cout << PROG_NAME" - "PROG_DESC"\n\n";
		show_usage();
		exit(0);
	    case OPT_VERSION:
		cout << PROG_NAME" - "PACKAGE_STRING << endl;
		exit(0);
	    default:
		show_usage();
		exit(1);
	}
    }

    if (argc - optind < 2) {
	show_usage();
	exit(1);
    }

    if (!renumber && argc - optind > 2) {
	cout << argv[0]
	     << ": --no-renumber isn't currently supported when merging databases."
	     << endl;
	exit(1);
    }

    // Path to the database to create.
    const char *destdir = argv[argc - 1];

    try {
	vector<string> sources;
	vector<Xapian::docid> offset;
	sources.reserve(argc - 1 - optind);
	offset.reserve(argc - 1 - optind);
	Xapian::docid tot_off = 0;
	enum { UNKNOWN, FLINT, CHERT } backend = UNKNOWN;
	const char * backend_names[] = { NULL, "flint", "chert" };
	for (int i = optind; i < argc - 1; ++i) {
	    const char *srcdir = argv[i];
	    // Check destdir isn't the same as any source directory...
	    if (strcmp(srcdir, destdir) == 0) {
		cout << argv[0]
		     << ": destination may not be the same as any source directory."
		     << endl;
		exit(1);
	    }

	    struct stat sb;
	    if (stat(string(srcdir) + "/iamflint", &sb) == 0) {
		if (backend == UNKNOWN) {
		    backend = FLINT;
		} else if (backend != FLINT) {
		    cout << argv[0] << ": All databases must be the same type.\n";
		    cout << argv[0] << ": '" << argv[optind] << "' is "
			 << backend_names[backend] << ", but "
			 "'" << srcdir << "' is flint." << endl;
		    exit(1);
		}
	    } else if (stat(string(srcdir) + "/iamchert", &sb) == 0) {
		if (backend == UNKNOWN) {
		    backend = CHERT;
		} else if (backend != CHERT) {
		    cout << argv[0] << ": All databases must be the same type.\n";
		    cout << argv[0] << ": '" << argv[optind] << "' is "
			 << backend_names[backend] << ", but "
			 "'" << srcdir << "' is chert." << endl;
		    exit(1);
		}
	    } else {
		cout << argv[0] << ": '" << srcdir
		     << "' is not a flint or chert database directory" << endl;
		exit(1);
	    }

	    Xapian::Database db(srcdir);
	    Xapian::docid last = 0;

	    // "Empty" databases might have spelling or synonym data so can't
	    // just be completely ignored.
	    if (db.get_doccount() != 0) {
		last = db.get_lastdocid();

		if (renumber) {
		    // Prune any unused docids off the start of this source
		    // database.
		    Xapian::PostingIterator it = db.postlist_begin("");
		    // This test should never fail, since db.get_doccount() is
		    // non-zero!
		    if (it != db.postlist_end("")) {
			// tot_off could wrap here, but it's unsigned, so
			// that's OK.
			tot_off -= (*it - 1);
		    }

		    // FIXME: get_lastdocid() returns a "high water mark" - we
		    // should prune unused docids off the end of each source
		    // database as well as off the start.
		}
	    }
	    offset.push_back(tot_off);
	    tot_off += last;

	    sources.push_back(string(srcdir) + '/');
	}

	// If the destination database directory doesn't exist, create it.
	if (mkdir(destdir, 0755) < 0) {
	    // Check why mkdir failed.  It's ok if the directory already
	    // exists, but we also get EEXIST if there's an existing file with
	    // that name.
	    if (errno == EEXIST) {
		struct stat sb;
		if (stat(destdir, &sb) == 0 && S_ISDIR(sb.st_mode))
		    errno = 0;
		else
		    errno = EEXIST; // stat might have changed it
	    }
	    if (errno) {
		cerr << argv[0] << ": cannot create directory '"
		     << destdir << "': " << strerror(errno) << endl;
		exit(1);
	    }
	}

	enum table_type {
	    POSTLIST, RECORD, TERMLIST, POSITION, VALUE, SPELLING, SYNONYM
	};
	struct table_list {
	    // The "base name" of the table.
	    const char * name;
	    // The type.
	    table_type type;
	    // zlib compression strategy to use on tags.
	    int compress_strategy;
	    // Create tables after position lazily.
	    bool lazy;
	};

	static const table_list tables[] = {
	    // name	    type	compress_strategy	lazy
	    { "postlist",   POSTLIST,	DONT_COMPRESS,		false },
	    { "record",	    RECORD,	Z_DEFAULT_STRATEGY,	false },
	    { "termlist",   TERMLIST,	Z_DEFAULT_STRATEGY,	false },
	    { "position",   POSITION,	DONT_COMPRESS,		true },
	    { "value",	    VALUE,	DONT_COMPRESS,		true },
	    { "spelling",   SPELLING,	Z_DEFAULT_STRATEGY,	true },
	    { "synonym",    SYNONYM,	Z_DEFAULT_STRATEGY,	true }
	};
	const table_list * tables_end = tables +
	    (sizeof(tables) / sizeof(tables[0]));

	for (const table_list * t = tables; t < tables_end; ++t) {
	    // Chert doesn't have a value table.
	    if (backend == CHERT && t->type == VALUE) continue;

	    // The postlist table requires an N-way merge, adjusting the
	    // headers of various blocks.  The spelling and synonym tables also
	    // need special handling.  The other tables have keys sorted in
	    // docid order, so we can merge them by simply copying all the keys
	    // from each source table in turn.
	    cout << t->name << " ..." << flush;

	    string dest = destdir;
	    dest += '/';
	    dest += t->name;
	    dest += '.';

	    bool output_will_exist = !t->lazy;

	    // Sometimes stat can fail for benign reasons (e.g. >= 2GB file
	    // on certain systems).
	    bool bad_stat = false;

	    off_t in_size = 0;

	    vector<string> inputs;
	    inputs.reserve(sources.size());
	    size_t inputs_present = 0;
	    for (vector<string>::const_iterator src = sources.begin();
		 src != sources.end(); ++src) {
		string s(*src);
		s += t->name;
		s += '.';

		struct stat sb;
		if (stat(s + "DB", &sb) == 0) {
		    in_size += sb.st_size / 1024;
		    output_will_exist = true;
		    ++inputs_present;
		} else if (errno != ENOENT) {
		    // We get ENOENT for an optional table.
		    bad_stat = true;
		    output_will_exist = true;
		    ++inputs_present;
		}
		inputs.push_back(s);
	    }

	    if (backend == CHERT && t->type == TERMLIST) {
		if (inputs_present != sources.size()) {
		    if (inputs_present != 0) {
			cout << '\r' << t->name << ": " << inputs_present
			     << " of " << sources.size() << " inputs present "
				"so suppressing output" << endl;
			continue;
		    }
		    output_will_exist = false;
		}
	    }

	    if (!output_will_exist) {
		cout << '\r' << t->name << ": doesn't exist" << endl;
		continue;
	    }

	    FlintTable out(t->name, dest, false, t->compress_strategy, t->lazy);
	    if (!t->lazy) {
		out.create_and_open(block_size);
	    } else {
		out.erase();
		out.set_block_size(block_size);
	    }

	    out.set_full_compaction(compaction != STANDARD);
	    if (compaction == FULLER) out.set_max_item_size(1);

	    switch (t->type) {
		case POSTLIST:
		    if (multipass && inputs.size() > 3) {
			multimerge_postlists(&out, destdir, tot_off,
					     inputs, offset, backend == CHERT);
		    } else {
			merge_postlists(&out, offset.begin(),
					inputs.begin(), inputs.end(),
					tot_off, backend == CHERT);
		    }
		    break;
		case SPELLING:
		    merge_spellings(&out, inputs.begin(), inputs.end());
		    break;
		case SYNONYM:
		    merge_synonyms(&out, inputs.begin(), inputs.end());
		    break;
		default:
		    // Position, Record, Termlist, Value (Value only for flint).
		    if (t->type == VALUE) {
			Assert(backend != CHERT);
		    }
		    merge_docid_keyed(t->name, &out, inputs, offset, t->lazy);
		    break;
	    }

	    // Commit as revision 1.
	    out.flush_db();
	    out.commit(1);

	    cout << '\r' << t->name << ": ";
	    off_t out_size = 0;
	    if (!bad_stat) {
		struct stat sb;
		if (stat(dest + "DB", &sb) == 0) {
		    out_size = sb.st_size / 1024;
		} else {
		    bad_stat = (errno != ENOENT);
		}
	    }
	    if (bad_stat) {
		cout << "Done (couldn't stat all the DB files)";
	    } else {
		if (out_size == in_size) {
		    cout << "Size unchanged (";
		} else if (out_size < in_size) {
		    cout << "Reduced by "
			 << 100 * double(in_size - out_size) / in_size << "% "
			 << in_size - out_size << "K (" << in_size << "K -> ";
		} else {
		    cout << "INCREASED by "
			 << 100 * double(out_size - in_size) / in_size << "% "
			 << out_size - in_size << "K (" << in_size << "K -> ";
		}
		cout << out_size << "K)";
	    }
	    cout << endl;
	}

	// Copy over the version file ("iamflint" or "iamchert").
	// FIXME: We may need to do something smarter that just copying an
	// arbitrary version file if the version file format changes...
	string dest = destdir;
	dest += "/iam";
	dest += backend_names[backend];
	dest += ".tmp";

	string src(argv[optind]);
	src += "/iam";
	src += backend_names[backend];

	ifstream input(src.c_str());
	char buf[1024];
	input.read(buf, sizeof(buf));
	if (!input.eof()) {
	    if (!input) {
		cerr << argv[0] << ": error reading '" << src << "': "
		     << strerror(errno) << endl;
		exit(1);
	    }
	    // Version file should be about 12 bytes, not > 1024!
	    cerr << argv[0] << ": version file '" << src << "' too large!"
		 << endl;
	    exit(1);
	}
	ofstream output(dest.c_str());
	if (!output.write(buf, input.gcount())) {
	    cerr << argv[0] << ": error writing '" << dest << "': "
		 << strerror(errno) << endl;
	    exit(1);
	}
	output.close();

	string version = destdir;
	version += "/iam";
	version += backend_names[backend];
	if (rename(dest.c_str(), version.c_str()) == -1) {
	    cerr << argv[0] << ": cannot rename '" << dest << "' to '"
		 << version << "': " << strerror(errno) << endl;
	    exit(1);
	}
    } catch (const Xapian::Error &error) {
	cerr << argv[0] << ": " << error.get_description() << endl;
	exit(1);
    }
}
