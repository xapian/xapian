/** @file
 * @brief Compact a honey database, or merge and compact several.
 */
/* Copyright (C) 2004,2005,2006,2007,2008,2009,2010,2011,2012,2013,2014,2015,2018,2019 Olly Betts
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

#include "xapian/compactor.h"
#include "xapian/constants.h"
#include "xapian/error.h"
#include "xapian/types.h"

#include <algorithm>
#include <memory>
#include <queue>
#include <type_traits>

#include <cerrno>
#include <cstdio>

#include "backends/flint_lock.h"
#include "compression_stream.h"
#include "honey_cursor.h"
#include "honey_database.h"
#include "honey_defs.h"
#include "honey_postlist_encodings.h"
#include "honey_table.h"
#include "honey_values.h"
#include "honey_version.h"
#include "filetests.h"
#include "internaltypes.h"
#include "pack.h"
#include "backends/valuestats.h"
#include "wordaccess.h"

#include "../byte_length_strings.h"
#include "../prefix_compressed_strings.h"

#ifdef XAPIAN_HAS_GLASS_BACKEND
# include "../glass/glass_database.h"
# include "../glass/glass_table.h"
# include "../glass/glass_values.h"
#endif

using namespace std;
using Honey::encode_valuestats;

[[noreturn]]
static void
throw_database_corrupt(const char* item, const char* pos)
{
    string message;
    if (pos != NULL) {
	message = "Value overflow unpacking termlist: ";
    } else {
	message = "Out of data unpacking termlist: ";
    }
    message += item;
    throw Xapian::DatabaseCorruptError(message);
}

#ifdef XAPIAN_HAS_GLASS_BACKEND
namespace GlassCompact {

static inline bool
is_user_metadata_key(const string& key)
{
    return key.size() > 1 && key[0] == '\0' && key[1] == '\xc0';
}

static inline bool
is_valuestats_key(const string& key)
{
    return key.size() > 1 && key[0] == '\0' && key[1] == '\xd0';
}

static inline bool
is_valuechunk_key(const string& key)
{
    return key.size() > 1 && key[0] == '\0' && key[1] == '\xd8';
}

static inline bool
is_doclenchunk_key(const string& key)
{
    return key.size() > 1 && key[0] == '\0' && key[1] == '\xe0';
}

}

static inline bool
termlist_key_is_values_used(const string& key)
{
    const char* p = key.data();
    const char* e = p + key.size();
    Xapian::docid did;
    if (unpack_uint_preserving_sort(&p, e, &did)) {
	if (p == e)
	    return false;
	if (e - p == 1 && *p == '\0')
	    return true;
    }
    throw Xapian::DatabaseCorruptError("termlist key format");
}
#endif

// Put all the helpers in a namespace to avoid symbols colliding with those of
// the same name in other flint-derived backends.
namespace HoneyCompact {

/// Return a Honey::KEY_* constant, or a different value for an invalid key.
static inline int
key_type(const string& key)
{
    if (key[0] != '\0')
	return Honey::KEY_POSTING_CHUNK;

    if (key.size() <= 1)
	return -1;

    unsigned char ch = key[1];
    if (ch >= Honey::KEY_VALUE_STATS && ch <= Honey::KEY_VALUE_STATS_HI)
	return Honey::KEY_VALUE_STATS;
    if (ch >= Honey::KEY_VALUE_CHUNK && ch <= Honey::KEY_VALUE_CHUNK_HI)
	return Honey::KEY_VALUE_CHUNK;
    if (ch >= Honey::KEY_DOCLEN_CHUNK && ch <= Honey::KEY_DOCLEN_CHUNK_HI)
	return Honey::KEY_DOCLEN_CHUNK;

    // Handle Honey::KEY_USER_METADATA, Honey::KEY_POSTING_CHUNK, and currently
    // unused values.
    return ch;
}

template<typename T> class PostlistCursor;

#ifdef XAPIAN_HAS_GLASS_BACKEND
// Convert glass to honey.
template<>
class PostlistCursor<const GlassTable&> : private GlassCursor {
    Xapian::docid offset;

  public:
    string key, tag;
    Xapian::docid firstdid;
    Xapian::docid chunk_lastdid;
    Xapian::termcount tf, cf;
    Xapian::termcount first_wdf;
    Xapian::termcount wdf_max;
    bool have_wdfs;

    PostlistCursor(const GlassTable* in, Xapian::docid offset_)
	: GlassCursor(in), offset(offset_), firstdid(0)
    {
	rewind();
    }

    bool next() {
	if (!GlassCursor::next()) return false;
	// We put all chunks into the non-initial chunk form here, then fix up
	// the first chunk for each term in the merged database as we merge.
	read_tag();
	key = current_key;
	tag = current_tag;
	tf = cf = 0;
	if (GlassCompact::is_user_metadata_key(key)) {
	    key[1] = Honey::KEY_USER_METADATA;
	    return true;
	}
	if (GlassCompact::is_valuestats_key(key)) {
	    // Adjust key.
	    const char* p = key.data();
	    const char* end = p + key.length();
	    p += 2;
	    Xapian::valueno slot;
	    if (!unpack_uint_last(&p, end, &slot))
		throw Xapian::DatabaseCorruptError("bad value stats key");
	    // FIXME: pack_uint_last() is not a good encoding for keys, as the
	    // encoded values do not in general sort the same way as the
	    // numeric values.  The first 256 values will be in order at least.
	    //
	    // We could just buffer up the stats for all the slots - it's
	    // unlikely there are going to be very many.  Another option is
	    // to use multiple cursors or seek a single cursor around.
	    AssertRel(slot, <=, 256);
	    key = Honey::make_valuestats_key(slot);
	    return true;
	}
	if (GlassCompact::is_valuechunk_key(key)) {
	    const char* p = key.data();
	    const char* end = p + key.length();
	    p += 2;
	    Xapian::valueno slot;
	    if (!unpack_uint(&p, end, &slot))
		throw Xapian::DatabaseCorruptError("bad value key");
	    // FIXME: pack_uint() is not a good encoding for keys, as the
	    // encoded values do not in general sort the same way as the
	    // numeric values.  The first 128 values will be in order at least.
	    //
	    // Buffering up this data for all slots is potentially prohibitively
	    // costly.  We probably need to use multiple cursors or seek a
	    // single cursor around.
	    AssertRel(slot, <=, 128);
	    Xapian::docid first_did;
	    if (!unpack_uint_preserving_sort(&p, end, &first_did))
		throw Xapian::DatabaseCorruptError("bad value key");
	    first_did += offset;

	    Glass::ValueChunkReader reader(tag.data(), tag.size(), first_did);
	    Xapian::docid last_did = first_did;
	    while (reader.next(), !reader.at_end()) {
		last_did = reader.get_docid();
	    }

	    key = Honey::make_valuechunk_key(slot, last_did);

	    // Add the docid delta across the chunk to the start of the tag.
	    string newtag;
	    pack_uint(newtag, last_did - first_did);
	    tag.insert(0, newtag);

	    return true;
	}

	if (GlassCompact::is_doclenchunk_key(key)) {
	    const char* d = key.data();
	    const char* e = d + key.size();
	    d += 2;

	    if (d == e) {
		// This is an initial chunk, so adjust tag header.
		d = tag.data();
		e = d + tag.size();
		if (!unpack_uint(&d, e, &tf) ||
		    !unpack_uint(&d, e, &cf) ||
		    !unpack_uint(&d, e, &firstdid)) {
		    throw Xapian::DatabaseCorruptError("Bad postlist key");
		}
		++firstdid;
		tag.erase(0, d - tag.data());
	    } else {
		// Not an initial chunk, just unpack firstdid.
		if (!unpack_uint_preserving_sort(&d, e, &firstdid) || d != e)
		    throw Xapian::DatabaseCorruptError("Bad postlist key");
	    }
	    firstdid += offset;

	    // Set key to placeholder value which just indicates that this is a
	    // doclen chunk.
	    static const char doclen_key_prefix[2] = {
		0, char(Honey::KEY_DOCLEN_CHUNK)
	    };
	    key.assign(doclen_key_prefix, 2);

	    d = tag.data();
	    e = d + tag.size();

	    // Convert doclen chunk to honey format.
	    string newtag;

	    // Skip the "last chunk" flag and increase_to_last.
	    if (d == e)
		throw Xapian::DatabaseCorruptError("No last chunk flag in "
						   "glass docdata chunk");
	    ++d;
	    Xapian::docid increase_to_last;
	    if (!unpack_uint(&d, e, &increase_to_last))
		throw Xapian::DatabaseCorruptError("Decoding last docid delta "
						   "in glass docdata chunk");

	    Xapian::termcount doclen_max = 0;
	    while (true) {
		Xapian::termcount doclen;
		if (!unpack_uint(&d, e, &doclen))
		    throw Xapian::DatabaseCorruptError("Decoding doclen in "
						       "glass docdata chunk");
		if (doclen > doclen_max)
		    doclen_max = doclen;
		unsigned char buf[4];
		unaligned_write4(buf, doclen);
		newtag.append(reinterpret_cast<char*>(buf), 4);
		if (d == e)
		    break;
		Xapian::docid gap_size;
		if (!unpack_uint(&d, e, &gap_size))
		    throw Xapian::DatabaseCorruptError("Decoding docid "
						       "gap_size in glass "
						       "docdata chunk");
		// FIXME: Split chunk if the gap_size is at all large.
		newtag.append(4 * gap_size, '\xff');
	    }

	    Assert(!startswith(newtag, "\xff\xff\xff\xff"));
	    Assert(!endswith(newtag, "\xff\xff\xff\xff"));

	    AssertEq(newtag.size() % 4, 0);
	    chunk_lastdid = firstdid - 1 + newtag.size() / 4;

	    // Only encode document lengths using a whole number of bytes for
	    // now.  We could allow arbitrary bit widths, but it complicates
	    // encoding and decoding so we should consider if the fairly small
	    // additional saving is worth it.
	    if (doclen_max >= 0xffff) {
		if (doclen_max >= 0xffffff) {
		    newtag.insert(0, 1, char(32));
		    swap(tag, newtag);
		} else if (doclen_max >= 0xffffffff) {
		    // FIXME: Handle these.
		    const char* m = "Document length values >= 0xffffffff not "
				    "currently handled";
		    throw Xapian::FeatureUnavailableError(m);
		} else {
		    tag.assign(1, char(24));
		    for (size_t i = 1; i < newtag.size(); i += 4)
			tag.append(newtag, i, 3);
		}
	    } else {
		if (doclen_max >= 0xff) {
		    tag.assign(1, char(16));
		    for (size_t i = 2; i < newtag.size(); i += 4)
			tag.append(newtag, i, 2);
		} else {
		    tag.assign(1, char(8));
		    for (size_t i = 3; i < newtag.size(); i += 4)
			tag.append(newtag, i, 1);
		}
	    }

	    return true;
	}

	// Adjust key if this is *NOT* an initial chunk.
	// key is: pack_string_preserving_sort(key, tname)
	// plus optionally: pack_uint_preserving_sort(key, did)
	const char* d = key.data();
	const char* e = d + key.size();
	string tname;
	if (!unpack_string_preserving_sort(&d, e, tname))
	    throw Xapian::DatabaseCorruptError("Bad postlist key");

	if (d == e) {
	    // This is an initial chunk for a term, so adjust tag header.
	    d = tag.data();
	    e = d + tag.size();
	    if (!unpack_uint(&d, e, &tf) ||
		!unpack_uint(&d, e, &cf) ||
		!unpack_uint(&d, e, &firstdid)) {
		throw Xapian::DatabaseCorruptError("Bad postlist key");
	    }
	    ++firstdid;
	    have_wdfs = (cf != 0);
	    tag.erase(0, d - tag.data());
	    wdf_max = 0;
	} else {
	    // Not an initial chunk, so adjust key.
	    size_t tmp = d - key.data();
	    if (!unpack_uint_preserving_sort(&d, e, &firstdid) || d != e)
		throw Xapian::DatabaseCorruptError("Bad postlist key");
	    key.erase(tmp - 1);
	}
	firstdid += offset;

	d = tag.data();
	e = d + tag.size();

	// Convert posting chunk to honey format, but without any header.
	string newtag;

	// Skip the "last chunk" flag; decode increase_to_last.
	if (d == e)
	    throw Xapian::DatabaseCorruptError("No last chunk flag in glass "
					       "posting chunk");
	++d;
	Xapian::docid increase_to_last;
	if (!unpack_uint(&d, e, &increase_to_last))
	    throw Xapian::DatabaseCorruptError("Decoding last docid delta in "
					       "glass posting chunk");
	chunk_lastdid = firstdid + increase_to_last;
	if (!unpack_uint(&d, e, &first_wdf))
	    throw Xapian::DatabaseCorruptError("Decoding first wdf in glass "
					       "posting chunk");
	if ((first_wdf != 0) != have_wdfs) {
	    // FIXME: Also need to adjust document length, total document
	    // length.

	    // Convert wdf=0 to 1 when the term has non-zero wdf elsewhere.
	    // first_wdf = 1;
	    // ++cf;
	    throw Xapian::DatabaseError("Honey does not support a term having "
					"both zero and non-zero wdf");
	}
	wdf_max = max(wdf_max, first_wdf);

	while (d != e) {
	    Xapian::docid delta;
	    if (!unpack_uint(&d, e, &delta))
		throw Xapian::DatabaseCorruptError("Decoding docid delta in "
						   "glass posting chunk");
	    pack_uint(newtag, delta);
	    Xapian::termcount wdf;
	    if (!unpack_uint(&d, e, &wdf))
		throw Xapian::DatabaseCorruptError("Decoding wdf in glass "
						   "posting chunk");
	    if ((wdf != 0) != have_wdfs) {
		// FIXME: Also need to adjust document length, total document
		// length.

		// Convert wdf=0 to 1 when the term has non-zero wdf elsewhere.
		// wdf = 1;
		// ++cf;
		throw Xapian::DatabaseError("Honey does not support a term "
					    "having both zero and non-zero "
					    "wdf");
	    }
	    if (have_wdfs) {
		pack_uint(newtag, wdf);
		wdf_max = max(wdf_max, wdf);
	    }
	}

	swap(tag, newtag);

	return true;
    }
};
#endif

template<>
class PostlistCursor<const HoneyTable&> : private HoneyCursor {
    Xapian::docid offset;

  public:
    string key, tag;
    Xapian::docid firstdid;
    Xapian::docid chunk_lastdid;
    Xapian::doccount tf;
    Xapian::termcount cf;
    Xapian::termcount first_wdf;
    Xapian::termcount wdf_max;
    bool have_wdfs;

    PostlistCursor(const HoneyTable* in, Xapian::docid offset_)
	: HoneyCursor(in), offset(offset_), firstdid(0)
    {
	rewind();
    }

    bool next() {
	if (!HoneyCursor::next()) return false;
	// We put all chunks into the non-initial chunk form here, then fix up
	// the first chunk for each term in the merged database as we merge.
	read_tag();
	key = current_key;
	tag = current_tag;
	tf = 0;
	switch (key_type(key)) {
	    case Honey::KEY_USER_METADATA:
	    case Honey::KEY_VALUE_STATS:
		return true;
	    case Honey::KEY_VALUE_CHUNK: {
		const char* p = key.data();
		const char* end = p + key.length();
		p += 2;
		Xapian::valueno slot;
		if (p[-1] != char(Honey::KEY_VALUE_CHUNK_HI)) {
		    slot = p[-1] - Honey::KEY_VALUE_CHUNK;
		} else {
		    if (!unpack_uint_preserving_sort(&p, end, &slot))
			throw Xapian::DatabaseCorruptError("bad value key");
		}
		Xapian::docid did;
		if (!unpack_uint_preserving_sort(&p, end, &did))
		    throw Xapian::DatabaseCorruptError("bad value key");
		key = Honey::make_valuechunk_key(slot, did + offset);
		return true;
	    }
	    case Honey::KEY_DOCLEN_CHUNK: {
		Xapian::docid did = Honey::docid_from_key(key);
		if (did == 0)
		    throw Xapian::DatabaseCorruptError("Bad doclen key");
		chunk_lastdid = did + offset;
		firstdid = chunk_lastdid - (tag.size() - 2) / (tag[0] / 8);
		key.resize(2);
		return true;
	    }
	    case Honey::KEY_POSTING_CHUNK:
		break;
	    default:
		throw Xapian::DatabaseCorruptError("Bad postlist table key "
						   "type");
	}

	// Adjust key if this is *NOT* an initial chunk.
	// key is: pack_string_preserving_sort(key, term)
	// plus optionally: pack_uint_preserving_sort(key, did)
	const char* d = key.data();
	const char* e = d + key.size();
	string term;
	if (!unpack_string_preserving_sort(&d, e, term))
	    throw Xapian::DatabaseCorruptError("Bad postlist key");

	if (d == e) {
	    // This is an initial chunk for a term, so remove tag header.
	    d = tag.data();
	    e = d + tag.size();

	    Xapian::docid lastdid;
	    if (!decode_initial_chunk_header(&d, e, tf, cf,
					     firstdid, lastdid, chunk_lastdid,
					     first_wdf, wdf_max)) {
		throw Xapian::DatabaseCorruptError("Bad postlist initial "
						   "chunk header");
	    }
	    // Ignore lastdid - we'll need to recalculate it (at least when
	    // merging, and for simplicity we always do).
	    (void)lastdid;
	    tag.erase(0, d - tag.data());

	    if (tf <= 2) {
		have_wdfs = false;
		Assert(tag.empty());
	    } else {
		have_wdfs = (cf != 0) && (cf - first_wdf != tf - 1);
		if (have_wdfs) {
		    Xapian::termcount remaining_cf_for_flat_wdf =
			(tf - 1) * wdf_max;
		    // Check this matches and that it isn't a false match due
		    // to overflow of the multiplication above.
		    if (cf - first_wdf == remaining_cf_for_flat_wdf &&
			usual(remaining_cf_for_flat_wdf / wdf_max == tf - 1)) {
			// The wdf is flat so we don't need to store it.
			have_wdfs = false;
		    }
		}
	    }
	} else {
	    if (cf > 0) {
		// The cf we report should only be non-zero for initial chunks
		// (of which there may be several for the same term from
		// different instances of this class when merging databases)
		// and gets summed to give the total cf for the term, so we
		// need to zero it here, after potentially using it to
		// calculate first_wdf.
		//
		// If cf is zero for a term, first_wdf will be zero for all
		// chunks so doesn't need setting specially here.
		if (!have_wdfs)
		    first_wdf = (cf - first_wdf) / (tf - 1);
		cf = 0;
	    }

	    // Not an initial chunk, so adjust key and remove tag header.
	    size_t tmp = d - key.data();
	    if (!unpack_uint_preserving_sort(&d, e, &chunk_lastdid) ||
		d != e) {
		throw Xapian::DatabaseCorruptError("Bad postlist key");
	    }
	    // -1 to remove the terminating zero byte too.
	    key.erase(tmp - 1);

	    d = tag.data();
	    e = d + tag.size();

	    if (have_wdfs) {
		if (!decode_delta_chunk_header(&d, e, chunk_lastdid, firstdid,
					       first_wdf)) {
		    throw Xapian::DatabaseCorruptError("Bad postlist delta "
						       "chunk header");
		}
	    } else {
		if (!decode_delta_chunk_header_no_wdf(&d, e, chunk_lastdid,
						      firstdid)) {
		    throw Xapian::DatabaseCorruptError("Bad postlist delta "
						       "chunk header");
		}
	    }
	    tag.erase(0, d - tag.data());
	}
	firstdid += offset;
	chunk_lastdid += offset;
	return true;
    }
};

template<>
class PostlistCursor<HoneyTable&> : public PostlistCursor<const HoneyTable&> {
  public:
    PostlistCursor(HoneyTable* in, Xapian::docid offset_)
	: PostlistCursor<const HoneyTable&>(in, offset_) {}
};

template<typename T>
class PostlistCursorGt {
  public:
    /** Return true if and only if a's key is strictly greater than b's key.
     */
    bool operator()(const T* a, const T* b) const {
	if (a->key > b->key) return true;
	if (a->key != b->key) return false;
	return (a->firstdid > b->firstdid);
    }
};

// U : vector<HoneyTable*>::const_iterator
template<typename T, typename U> void
merge_postlists(Xapian::Compactor* compactor,
		T* out, vector<Xapian::docid>::const_iterator offset,
		U b, U e)
{
    typedef decltype(**b) table_type; // E.g. HoneyTable
    typedef PostlistCursor<table_type> cursor_type;
    typedef PostlistCursorGt<cursor_type> gt_type;
    priority_queue<cursor_type*, vector<cursor_type*>, gt_type> pq;
    for ( ; b != e; ++b, ++offset) {
	auto in = *b;
	auto cursor = new cursor_type(in, *offset);
	if (cursor->next()) {
	    pq.push(cursor);
	} else {
	    // Skip empty tables.
	    delete cursor;
	}
    }

    string last_key;
    {
	// Merge user metadata.
	vector<string> tags;
	while (!pq.empty()) {
	    cursor_type* cur = pq.top();
	    const string& key = cur->key;
	    if (key_type(key) != Honey::KEY_USER_METADATA) break;

	    if (key != last_key) {
		if (!tags.empty()) {
		    if (tags.size() > 1 && compactor) {
			Assert(!last_key.empty());
			// FIXME: It would be better to merge all duplicates
			// for a key in one call, but currently we don't in
			// multipass mode.
			const string& resolved_tag =
			    compactor->resolve_duplicate_metadata(last_key,
								  tags.size(),
								  &tags[0]);
			if (!resolved_tag.empty())
			    out->add(last_key, resolved_tag);
		    } else {
			Assert(!last_key.empty());
			out->add(last_key, tags[0]);
		    }
		    tags.resize(0);
		}
		last_key = key;
	    }
	    tags.push_back(cur->tag);

	    pq.pop();
	    if (cur->next()) {
		pq.push(cur);
	    } else {
		delete cur;
	    }
	}
	if (!tags.empty()) {
	    if (tags.size() > 1 && compactor) {
		Assert(!last_key.empty());
		const string& resolved_tag =
		    compactor->resolve_duplicate_metadata(last_key,
							  tags.size(),
							  &tags[0]);
		if (!resolved_tag.empty())
		    out->add(last_key, resolved_tag);
	    } else {
		Assert(!last_key.empty());
		out->add(last_key, tags[0]);
	    }
	}
    }

    {
	// Merge valuestats.
	Xapian::doccount freq = 0;
	string lbound, ubound;

	while (!pq.empty()) {
	    cursor_type* cur = pq.top();
	    const string& key = cur->key;
	    if (key_type(key) != Honey::KEY_VALUE_STATS) break;
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

	    const string& tag = cur->tag;

	    const char* pos = tag.data();
	    const char* end = pos + tag.size();

	    Xapian::doccount f;
	    string l, u;
	    if (!unpack_uint(&pos, end, &f)) {
		if (*pos == 0)
		    throw Xapian::DatabaseCorruptError("Incomplete stats item "
						       "in value table");
		throw Xapian::RangeError("Frequency statistic in value table "
					 "is too large");
	    }
	    if (!unpack_string(&pos, end, l)) {
		if (*pos == 0)
		    throw Xapian::DatabaseCorruptError("Incomplete stats item "
						       "in value table");
		throw Xapian::RangeError("Lower bound in value table is too "
					 "large");
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
	cursor_type* cur = pq.top();
	const string& key = cur->key;
	if (key_type(key) != Honey::KEY_VALUE_CHUNK) break;
	out->add(key, cur->tag);
	pq.pop();
	if (cur->next()) {
	    pq.push(cur);
	} else {
	    delete cur;
	}
    }

    // Merge doclen chunks.
    while (!pq.empty()) {
	cursor_type* cur = pq.top();
	if (key_type(cur->key) != Honey::KEY_DOCLEN_CHUNK) break;
	string tag = std::move(cur->tag);
	auto chunk_lastdid = cur->chunk_lastdid;
	pq.pop();
	if (cur->next()) {
	    pq.push(cur);
	} else {
	    delete cur;
	}
	while (!pq.empty()) {
	    cur = pq.top();
	    if (key_type(cur->key) != Honey::KEY_DOCLEN_CHUNK)
		break;
	    if (tag[0] != cur->tag[0]) {
		// Different width values in the two tags, so punt for now.
		// FIXME: We would ideally optimise the total size here.
		break;
	    }
	    size_t byte_width = tag[0] / 8;
	    auto new_size = tag.size();
	    Xapian::docid gap_size = cur->firstdid - chunk_lastdid - 1;
	    new_size += gap_size * byte_width;
	    if (new_size >= HONEY_DOCLEN_CHUNK_MAX) {
		// The gap spans beyond HONEY_DOCLEN_CHUNK_MAX.
		break;
	    }
	    new_size += cur->tag.size() - 1;
	    auto full_new_size = new_size;
	    if (new_size > HONEY_DOCLEN_CHUNK_MAX) {
		if (byte_width > 1) {
		    // HONEY_DOCLEN_CHUNK_MAX should be one more than a
		    // multiple of 12 so for widths 1,2,3,4 we can fix the
		    // initial byte which indicates the width for the chunk
		    // plus an exact number of entries.
		    auto m = (new_size - HONEY_DOCLEN_CHUNK_MAX) % byte_width;
		    (void)m;
		    AssertEq(m, 0);
		}
		new_size = HONEY_DOCLEN_CHUNK_MAX;
	    }
	    tag.reserve(new_size);
	    tag.append(byte_width * gap_size, '\xff');
	    if (new_size != full_new_size) {
		// Partial copy.
		auto copy_size = new_size - tag.size();
		tag.append(cur->tag, 1, copy_size);
		cur->tag.erase(1, copy_size);
		copy_size /= byte_width;
		cur->firstdid += copy_size;
		chunk_lastdid += gap_size;
		chunk_lastdid += copy_size;
		break;
	    }

	    tag.append(cur->tag, 1, string::npos);
	    chunk_lastdid = cur->chunk_lastdid;

	    pq.pop();
	    if (cur->next()) {
		pq.push(cur);
	    } else {
		delete cur;
	    }
	}
	out->add(Honey::make_doclenchunk_key(chunk_lastdid), tag);
    }

    struct HoneyPostListChunk {
	Xapian::docid first, last;
	Xapian::termcount first_wdf;
	Xapian::termcount wdf_max;

      private:
	Xapian::doccount tf;
	Xapian::termcount cf;
	bool have_wdfs;
	string data;

      public:
	HoneyPostListChunk(Xapian::docid first_,
			   Xapian::docid last_,
			   Xapian::termcount first_wdf_,
			   Xapian::termcount wdf_max_,
			   Xapian::doccount tf_,
			   Xapian::termcount cf_,
			   bool have_wdfs_,
			   string&& data_)
	    : first(first_),
	      last(last_),
	      first_wdf(first_wdf_),
	      wdf_max(wdf_max_),
	      tf(tf_),
	      cf(cf_),
	      have_wdfs(have_wdfs_),
	      data(data_) {}

	/** Estimate the size of data once spliced.
	 *
	 *  @param want_wdfs  With explicitly encoded wdfs?
	 */
	size_t data_size(bool want_wdfs) const {
	    if (data.empty()) {
		// Allow one byte for each docid delta, and another byte for
		// each wdf (if explicitly stored).
		return tf * (1u + size_t(want_wdfs));
	    }

	    if (have_wdfs == want_wdfs) {
		// Allow 1 for the explicit wdf once spliced.
		return data.size() + size_t(want_wdfs);
	    }

	    if (have_wdfs) {
		// Assume stripping wdfs will halve the size.
		return (data.size() + 1u) / 2u;
	    }

	    // Assume adding wdfs will double the size.
	    return data.size() * 2u;
	}

	/// Append postings to tag, which should only contain the chunk header.
	void append_postings_to(string& tag, bool want_wdfs) {
	    if (data.empty()) {
		if (tf == 1) {
		    return;
		}
		AssertEq(tf, 2);
		pack_uint(tag, last - first - 1);
		if (want_wdfs) {
		    pack_uint(tag, cf - first_wdf);
		}
		return;
	    }

	    if (have_wdfs == want_wdfs) {
		// We can just copy the encoded data.
		tag += data;
	    } else if (want_wdfs) {
		// Need to add wdfs which were implicit.
		auto wdf = (cf - first_wdf) / (tf - 1);
		const char* pos = data.data();
		const char* pos_end = pos + data.size();
		while (pos != pos_end) {
		    Xapian::docid delta;
		    if (!unpack_uint(&pos, pos_end, &delta))
			throw_database_corrupt("Decoding docid delta", pos);
		    pack_uint(tag, delta);
		    pack_uint(tag, wdf);
		}
	    } else {
		// Need to remove wdfs which are now implicit.
		const char* pos = data.data();
		const char* pos_end = pos + data.size();
		while (pos != pos_end) {
		    Xapian::docid delta;
		    if (!unpack_uint(&pos, pos_end, &delta))
			throw_database_corrupt("Decoding docid delta", pos);
		    pack_uint(tag, delta);
		    Xapian::termcount wdf;
		    if (!unpack_uint(&pos, pos_end, &wdf))
			throw_database_corrupt("Decoding wdf", pos);
		    (void)wdf;
		}
	    }
	}

	/// Append postings to tag, which should already contain postings.
	void append_postings_to(string& tag, bool want_wdfs,
				Xapian::docid splice_did) {
	    pack_uint(tag, first - splice_did - 1);
	    if (want_wdfs) {
		pack_uint(tag, first_wdf);
	    }
	    append_postings_to(tag, want_wdfs);
	}
    };
    vector<HoneyPostListChunk> tags;

    Xapian::termcount tf = 0, cf = 0; // Initialise to avoid warnings.

    while (true) {
	cursor_type* cur = NULL;
	if (!pq.empty()) {
	    cur = pq.top();
	    pq.pop();
	}
	if (cur) {
	    AssertEq(key_type(cur->key), Honey::KEY_POSTING_CHUNK);
	}
	if (cur == NULL || cur->key != last_key) {
	    if (!tags.empty()) {
		Xapian::termcount first_wdf = tags[0].first_wdf;
		Xapian::docid chunk_lastdid = tags[0].last;
		Xapian::docid last_did = tags.back().last;
		Xapian::termcount wdf_max =
		    max_element(tags.begin(), tags.end(),
				[](const HoneyPostListChunk& x,
				   const HoneyPostListChunk& y) {
				    return x.wdf_max < y.wdf_max;
				})->wdf_max;

		bool have_wdfs = true;
		if (cf == 0) {
		    // wdf must always be zero.
		    have_wdfs = false;
		} else if (tf <= 2) {
		    // We only need to store cf and first_wdf to know all wdfs.
		    have_wdfs = false;
		} else if (cf == tf - 1 + first_wdf) {
		    // wdf must be 1 for second and subsequent entries.
		    have_wdfs = false;
		} else {
		    Xapian::termcount remaining_cf_for_flat_wdf =
			(tf - 1) * wdf_max;
		    // Check this matches and that it isn't a false match due
		    // to overflow of the multiplication above.
		    if (cf - first_wdf == remaining_cf_for_flat_wdf &&
			usual(remaining_cf_for_flat_wdf / wdf_max == tf - 1)) {
			// The wdf is flat so we don't need to store it.
			have_wdfs = false;
		    }
		}

		// Decide whether to concatenate chunks, and if so how many.
		// We need to know the final docid in the chunk in order to
		// encode the header.  We merge [0,j) here.
		size_t j = 1;
		if (tags.size() > 1) {
		    if (tf <= HONEY_POSTLIST_CHUNK_MAX / (2 * (64 / 7 + 1))) {
			// The worst-case size if we merged all the chunks is
			// small enough to just have one chunk.
			//
			// Each posting needs a docid delta and wdf value, both
			// of which might be 64 bit and the encoding used needs
			// a byte for every 7 bits up to the msb set bit.
			j = tags.size();
		    } else {
			size_t est = tags[0].data_size(have_wdfs);
			while (j < tags.size()) {
			    est += tags[j].data_size(have_wdfs);
			    if (est > HONEY_POSTLIST_CHUNK_MAX) break;
			    ++j;
			}
		    }
		}

		chunk_lastdid = tags[j - 1].last;

		string first_tag;
		encode_initial_chunk_header(tf, cf, tags[0].first, last_did,
					    chunk_lastdid,
					    first_wdf, wdf_max, first_tag);

		if (tf > 2) {
		    // If tf <= 2 there's no explicit posting data.
		    tags[0].append_postings_to(first_tag, have_wdfs);
		    for (size_t chunk = 1; chunk != j; ++chunk) {
			tags[chunk].append_postings_to(first_tag, have_wdfs,
						       tags[chunk - 1].last);
		    }
		}
		out->add(last_key, first_tag);

		if (j != tags.size()) {
		    // Output continuation chunk(s).
		    string term;
		    const char* p = last_key.data();
		    const char* end = p + last_key.size();
		    if (!unpack_string_preserving_sort(&p, end, term) ||
			p != end) {
			throw Xapian::DatabaseCorruptError("Bad postlist "
							   "chunk key");
		    }

		    while (j < tags.size()) {
			// Here we merge tags [i,j) to a continuation chunk.
			size_t i = j;
			size_t est = tags[j].data_size(have_wdfs);
			while (++j < tags.size()) {
			    est += tags[j].data_size(have_wdfs);
			    if (est > HONEY_POSTLIST_CHUNK_MAX) break;
			}

			last_did = tags[j - 1].last;
			string tag;
			if (have_wdfs) {
			    encode_delta_chunk_header(tags[i].first,
						      last_did,
						      tags[i].first_wdf,
						      tag);
			} else {
			    encode_delta_chunk_header_no_wdf(tags[i].first,
							     last_did,
							     tag);
			}

			tags[i].append_postings_to(tag, have_wdfs);
			while (++i != j) {
			    tags[i].append_postings_to(tag, have_wdfs,
						       tags[i - 1].last);
			}

			out->add(pack_honey_postlist_key(term, last_did), tag);
		    }
		}
	    }
	    tags.clear();
	    if (cur == NULL) break;
	    tf = cf = 0;
	    last_key = cur->key;
	}

	if (tf && cur->tf && (cf == 0) != (cur->cf == 0)) {
	    // FIXME: Also need to adjust document length, total document
	    // length.
	    // Convert wdf=0 to 1 when the term has non-zero wdf elsewhere.
	    throw Xapian::DatabaseError("Honey does not support a term having "
					"both zero and non-zero wdf");
	}
	tf += cur->tf;
	cf += cur->cf;

	tags.push_back(HoneyPostListChunk(cur->firstdid,
					  cur->chunk_lastdid,
					  cur->first_wdf,
					  cur->wdf_max,
					  cur->tf,
					  cur->cf,
					  cur->have_wdfs,
					  std::move(cur->tag)));
	if (cur->next()) {
	    pq.push(cur);
	} else {
	    delete cur;
	}
    }
}

template<typename T> struct MergeCursor;

#ifdef XAPIAN_HAS_GLASS_BACKEND
template<>
struct MergeCursor<const GlassTable&> : public GlassCursor {
    explicit MergeCursor(const GlassTable* in) : GlassCursor(in) {
	rewind();
    }
};
#endif

template<>
struct MergeCursor<const HoneyTable&> : public HoneyCursor {
    explicit MergeCursor(const HoneyTable* in) : HoneyCursor(in) {
	rewind();
    }
};

template<typename T>
struct CursorGt {
    /// Return true if and only if a's key is strictly greater than b's key.
    bool operator()(const T* a, const T* b) const {
	if (b->after_end()) return false;
	if (a->after_end()) return true;
	return (a->current_key > b->current_key);
    }
};

#ifdef XAPIAN_HAS_GLASS_BACKEND
// Convert glass to honey.
static void
merge_spellings(HoneyTable* out,
		vector<const GlassTable*>::const_iterator b,
		vector<const GlassTable*>::const_iterator e)
{
    typedef MergeCursor<const GlassTable&> cursor_type;
    typedef CursorGt<cursor_type> gt_type;
    priority_queue<cursor_type*, vector<cursor_type*>, gt_type> pq;
    for ( ; b != e; ++b) {
	auto in = *b;
	auto cursor = new cursor_type(in);
	if (cursor->next()) {
	    pq.push(cursor);
	} else {
	    // Skip empty tables.
	    delete cursor;
	}
    }

    while (!pq.empty()) {
	cursor_type* cur = pq.top();
	pq.pop();

	// Glass vs honey spelling key prefix map:
	//
	//  Type	Glass	    Honey
	//
	//  bookend	Bbd	    \x00bd
	//  head	Hhe	    \x01he
	//  middle	Mddl	    \x02ddl
	//  tail	Til	    \x03il
	//  word	Wword	    word
	//
	// In honey, if the word's first byte is <= '\x04', we add a prefix
	// of '\x04' (which is unlikely in real world use but ensures that
	// we can handle arbitrary strings).
	//
	// The prefixes for honey are chosen such that we save a byte for the
	// key of all real-world words, and so that more first bytes are used
	// so that a top-level array index is more effective, especially for
	// random-access lookup of word entries (which we do to check the
	// frequency of potential spelling candidates).
	//
	// The other prefixes are chosen such that we can do look up in key
	// order at search time, which is more efficient for a cursor which can
	// only efficiently move forwards.
	//
	// Note that the key ordering is the same for glass and honey, which
	// makes translating during compaction simpler.
	string key = cur->current_key;
	switch (key[0]) {
	    case 'B':
		key[0] = Honey::KEY_PREFIX_BOOKEND;
		break;
	    case 'H':
		key[0] = Honey::KEY_PREFIX_HEAD;
		break;
	    case 'M':
		key[0] = Honey::KEY_PREFIX_MIDDLE;
		break;
	    case 'T':
		key[0] = Honey::KEY_PREFIX_TAIL;
		break;
	    case 'W':
		if (static_cast<unsigned char>(key[1]) > Honey::KEY_PREFIX_WORD)
		    key.erase(0, 1);
		else
		    key[0] = Honey::KEY_PREFIX_WORD;
		break;
	    default: {
		string m = "Bad spelling key prefix: ";
		m += static_cast<unsigned char>(key[0]);
		throw Xapian::DatabaseCorruptError(m);
	    }
	}

	if (pq.empty() || pq.top()->current_key > key) {
	    // No merging to do for this key so just copy the tag value,
	    // adjusting if necessary.  If we don't need to adjust it, just
	    // copy the compressed value.
	    bool compressed;
	    switch (key[0]) {
		case Honey::KEY_PREFIX_HEAD: {
		    // Honey omits the first two bytes from the first entry
		    // since we know what those most be from the key
		    // (subsequent entries won't include those anyway, since
		    // they'll be part of the reused prefix).
		    compressed = cur->read_tag(false);
		    unsigned char len = cur->current_tag[0] ^ MAGIC_XOR_VALUE;
		    cur->current_tag[0] = (len - 2) ^ MAGIC_XOR_VALUE;
		    AssertEq(cur->current_tag[1], key[1]);
		    AssertEq(cur->current_tag[2], key[2]);
		    cur->current_tag.erase(1, 2);
		    break;
		}
		case Honey::KEY_PREFIX_TAIL:
		case Honey::KEY_PREFIX_BOOKEND: {
		    // Need to repack because honey omits the last 2 or 1 bytes
		    // from each entry (2 for tail, 1 for bookend) since we
		    // know what those must be from the key.
		    compressed = cur->read_tag(false);
		    PrefixCompressedStringItor spell_in(cur->current_tag);
		    string new_tag;
		    PrefixCompressedStringWriter spell_out(new_tag, key);
		    while (!spell_in.at_end()) {
			spell_out.append(*spell_in);
			++spell_in;
		    }
		    cur->current_tag = std::move(new_tag);
		    break;
		}
		default:
		    compressed = cur->read_tag(true);
		    break;
	    }
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
	if (static_cast<unsigned char>(key[0]) < Honey::KEY_PREFIX_WORD) {
	    // We just want the union of words, so copy over the first instance
	    // and skip any identical ones.
	    priority_queue<PrefixCompressedStringItor*,
			   vector<PrefixCompressedStringItor*>,
			   PrefixCompressedStringItorGt> pqtag;
	    // Stick all the cursor_type pointers in a vector because their
	    // current_tag members must remain valid while we're merging their
	    // tags, but we need to call next() on them all afterwards.
	    vector<cursor_type*> vec;
	    vec.reserve(pq.size());

	    while (true) {
		cur->read_tag();
		pqtag.push(new PrefixCompressedStringItor(cur->current_tag));
		vec.push_back(cur);
		if (pq.empty() || pq.top()->current_key != key) break;
		cur = pq.top();
		pq.pop();
	    }

	    PrefixCompressedStringWriter wr(tag, key);
	    string lastword;
	    while (!pqtag.empty()) {
		PrefixCompressedStringItor* it = pqtag.top();
		pqtag.pop();
		string word = **it;
		if (word != lastword) {
		    lastword = word;
		    wr.append(lastword);
		}
		++*it;
		if (!it->at_end()) {
		    pqtag.push(it);
		} else {
		    delete it;
		}
	    }

	    for (auto i : vec) {
		cur = i;
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
		const char* p = cur->current_tag.data();
		const char* end = p + cur->current_tag.size();
		if (!unpack_uint_last(&p, end, &freq) || freq == 0) {
		    throw_database_corrupt("Bad spelling word freq", p);
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
	    tag.resize(0);
	    pack_uint_last(tag, tot_freq);
	}
	out->add(key, tag);
    }
}
#endif

static void
merge_spellings(HoneyTable* out,
		vector<const HoneyTable*>::const_iterator b,
		vector<const HoneyTable*>::const_iterator e)
{
    typedef MergeCursor<const HoneyTable&> cursor_type;
    typedef CursorGt<cursor_type> gt_type;
    priority_queue<cursor_type*, vector<cursor_type*>, gt_type> pq;
    for ( ; b != e; ++b) {
	auto in = *b;
	auto cursor = new cursor_type(in);
	if (cursor->next()) {
	    pq.push(cursor);
	} else {
	    // Skip empty tables.
	    delete cursor;
	}
    }

    while (!pq.empty()) {
	cursor_type* cur = pq.top();
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
	if (static_cast<unsigned char>(key[0]) < Honey::KEY_PREFIX_WORD) {
	    // We just want the union of words, so copy over the first instance
	    // and skip any identical ones.
	    priority_queue<PrefixCompressedStringItor*,
			   vector<PrefixCompressedStringItor*>,
			   PrefixCompressedStringItorGt> pqtag;
	    // Stick all the cursor_type pointers in a vector because their
	    // current_tag members must remain valid while we're merging their
	    // tags, but we need to call next() on them all afterwards.
	    vector<cursor_type*> vec;
	    vec.reserve(pq.size());

	    while (true) {
		cur->read_tag();
		pqtag.push(new PrefixCompressedStringItor(cur->current_tag,
							  key));
		vec.push_back(cur);
		if (pq.empty() || pq.top()->current_key != key) break;
		cur = pq.top();
		pq.pop();
	    }

	    PrefixCompressedStringWriter wr(tag, key);
	    string lastword;
	    while (!pqtag.empty()) {
		PrefixCompressedStringItor* it = pqtag.top();
		pqtag.pop();
		string word = **it;
		if (word != lastword) {
		    lastword = word;
		    wr.append(lastword);
		}
		++*it;
		if (!it->at_end()) {
		    pqtag.push(it);
		} else {
		    delete it;
		}
	    }

	    for (auto i : vec) {
		cur = i;
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
		const char* p = cur->current_tag.data();
		const char* end = p + cur->current_tag.size();
		if (!unpack_uint_last(&p, end, &freq) || freq == 0) {
		    throw_database_corrupt("Bad spelling word freq", p);
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
	    tag.resize(0);
	    pack_uint_last(tag, tot_freq);
	}
	out->add(key, tag);
    }
}

// U : vector<HoneyTable*>::const_iterator
template<typename T, typename U> void
merge_synonyms(T* out, U b, U e)
{
    typedef decltype(**b) table_type; // E.g. HoneyTable
    typedef MergeCursor<table_type> cursor_type;
    typedef CursorGt<cursor_type> gt_type;
    priority_queue<cursor_type*, vector<cursor_type*>, gt_type> pq;
    for ( ; b != e; ++b) {
	auto in = *b;
	auto cursor = new cursor_type(in);
	if (cursor->next()) {
	    pq.push(cursor);
	} else {
	    // Skip empty tables.
	    delete cursor;
	}
    }

    while (!pq.empty()) {
	cursor_type* cur = pq.top();
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

	// We just want the union of words, so copy over the first instance
	// and skip any identical ones.
	priority_queue<ByteLengthPrefixedStringItor*,
		       vector<ByteLengthPrefixedStringItor*>,
		       ByteLengthPrefixedStringItorGt> pqtag;
	vector<cursor_type*> vec;

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
	    ByteLengthPrefixedStringItor* it = pqtag.top();
	    pqtag.pop();
	    if (**it != lastword) {
		lastword = **it;
		tag += uint8_t(lastword.size() ^ MAGIC_XOR_VALUE);
		tag += lastword;
	    }
	    ++*it;
	    if (!it->at_end()) {
		pqtag.push(it);
	    } else {
		delete it;
	    }
	}

	for (auto i : vec) {
	    cur = i;
	    if (cur->next()) {
		pq.push(cur);
	    } else {
		delete cur;
	    }
	}

	out->add(key, tag);
    }
}

template<typename T, typename U> void
multimerge_postlists(Xapian::Compactor* compactor,
		     T* out, const char* tmpdir,
		     const vector<U*>& in,
		     vector<Xapian::docid> off)
{
    if (in.size() <= 3) {
	merge_postlists(compactor, out, off.begin(), in.begin(), in.end());
	return;
    }
    unsigned int c = 0;
    vector<HoneyTable*> tmp;
    tmp.reserve(in.size() / 2);
    {
	vector<Xapian::docid> newoff;
	newoff.resize(in.size() / 2);
	for (unsigned int i = 0, j; i < in.size(); i = j) {
	    j = i + 2;
	    if (j == in.size() - 1) ++j;

	    string dest = tmpdir;
	    char buf[64];
	    sprintf(buf, "/tmp%u_%u.", c, i / 2);
	    dest += buf;

	    HoneyTable* tmptab = new HoneyTable("postlist", dest, false);

	    // Don't compress entries in temporary tables, even if the final
	    // table would do so.  Any already compressed entries will get
	    // copied in compressed form.
	    Honey::RootInfo root_info;
	    root_info.init(0);
	    const int flags = Xapian::DB_DANGEROUS|Xapian::DB_NO_SYNC;
	    tmptab->create_and_open(flags, root_info);

	    merge_postlists(compactor, tmptab, off.begin() + i,
			    in.begin() + i, in.begin() + j);
	    tmp.push_back(tmptab);
	    tmptab->flush_db();
	    tmptab->commit(1, &root_info);
	}
	swap(off, newoff);
	++c;
    }

    while (tmp.size() > 3) {
	vector<HoneyTable*> tmpout;
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

	    HoneyTable* tmptab = new HoneyTable("postlist", dest, false);

	    // Don't compress entries in temporary tables, even if the final
	    // table would do so.  Any already compressed entries will get
	    // copied in compressed form.
	    Honey::RootInfo root_info;
	    root_info.init(0);
	    const int flags = Xapian::DB_DANGEROUS|Xapian::DB_NO_SYNC;
	    tmptab->create_and_open(flags, root_info);

	    merge_postlists(compactor, tmptab, off.begin() + i,
			    tmp.begin() + i, tmp.begin() + j);
	    if (c > 0) {
		for (unsigned int k = i; k < j; ++k) {
		    // FIXME: unlink(tmp[k]->get_path().c_str());
		    delete tmp[k];
		    tmp[k] = NULL;
		}
	    }
	    tmpout.push_back(tmptab);
	    tmptab->flush_db();
	    tmptab->commit(1, &root_info);
	}
	swap(tmp, tmpout);
	swap(off, newoff);
	++c;
    }
    merge_postlists(compactor, out, off.begin(), tmp.begin(), tmp.end());
    if (c > 0) {
	for (size_t k = 0; k < tmp.size(); ++k) {
	    // FIXME: unlink(tmp[k]->get_path().c_str());
	    delete tmp[k];
	    tmp[k] = NULL;
	}
    }
}

template<typename T> class PositionCursor;

#ifdef XAPIAN_HAS_GLASS_BACKEND
template<>
class PositionCursor<const GlassTable&> : private GlassCursor {
    Xapian::docid offset;

  public:
    string key;
    Xapian::docid firstdid;

    PositionCursor(const GlassTable* in, Xapian::docid offset_)
	: GlassCursor(in), offset(offset_), firstdid(0) {
	rewind();
    }

    bool next() {
	if (!GlassCursor::next()) return false;
	read_tag();
	const char* d = current_key.data();
	const char* e = d + current_key.size();
	string term;
	Xapian::docid did;
	if (!unpack_string_preserving_sort(&d, e, term) ||
	    !unpack_uint_preserving_sort(&d, e, &did) ||
	    d != e) {
	    throw Xapian::DatabaseCorruptError("Bad position key");
	}

	key.resize(0);
	pack_string_preserving_sort(key, term);
	pack_uint_preserving_sort(key, did + offset);
	return true;
    }

    const string& get_tag() const {
	return current_tag;
    }
};
#endif

template<>
class PositionCursor<const HoneyTable&> : private HoneyCursor {
    Xapian::docid offset;

  public:
    string key;
    Xapian::docid firstdid;

    PositionCursor(const HoneyTable* in, Xapian::docid offset_)
	: HoneyCursor(in), offset(offset_), firstdid(0) {
	rewind();
    }

    bool next() {
	if (!HoneyCursor::next()) return false;
	read_tag();
	const char* d = current_key.data();
	const char* e = d + current_key.size();
	string term;
	Xapian::docid did;
	if (!unpack_string_preserving_sort(&d, e, term) ||
	    !unpack_uint_preserving_sort(&d, e, &did) ||
	    d != e) {
	    throw Xapian::DatabaseCorruptError("Bad position key");
	}

	key.resize(0);
	pack_string_preserving_sort(key, term);
	pack_uint_preserving_sort(key, did + offset);
	return true;
    }

    const string& get_tag() const {
	return current_tag;
    }
};

template<typename T>
class PositionCursorGt {
  public:
    /** Return true if and only if a's key is strictly greater than b's key.
     */
    bool operator()(const T* a, const T* b) const {
	return a->key > b->key;
    }
};

template<typename T, typename U> void
merge_positions(T* out, const vector<U*>& inputs,
		const vector<Xapian::docid>& offset)
{
    typedef decltype(*inputs[0]) table_type; // E.g. HoneyTable
    typedef PositionCursor<table_type> cursor_type;
    typedef PositionCursorGt<cursor_type> gt_type;
    priority_queue<cursor_type*, vector<cursor_type*>, gt_type> pq;
    for (size_t i = 0; i < inputs.size(); ++i) {
	auto in = inputs[i];
	auto cursor = new cursor_type(in, offset[i]);
	if (cursor->next()) {
	    pq.push(cursor);
	} else {
	    // Skip empty tables.
	    delete cursor;
	}
    }

    while (!pq.empty()) {
	cursor_type* cur = pq.top();
	pq.pop();
	out->add(cur->key, cur->get_tag());
	if (cur->next()) {
	    pq.push(cur);
	} else {
	    delete cur;
	}
    }
}

template<typename T, typename U> void
merge_docid_keyed(T* out, const vector<U*>& inputs,
		  const vector<Xapian::docid>& offset,
		  int = 0)
{
    for (size_t i = 0; i < inputs.size(); ++i) {
	Xapian::docid off = offset[i];

	auto in = inputs[i];
	HoneyCursor cur(in);
	cur.rewind();

	string key;
	while (cur.next()) {
	    // Adjust the key if this isn't the first database.
	    if (off) {
		Xapian::docid did;
		const char* d = cur.current_key.data();
		const char* e = d + cur.current_key.size();
		if (!unpack_uint_preserving_sort(&d, e, &did)) {
		    string msg = "Bad key in ";
		    msg += inputs[i]->get_path();
		    throw Xapian::DatabaseCorruptError(msg);
		}
		did += off;
		key.resize(0);
		pack_uint_preserving_sort(key, did);
		if (d != e) {
		    // Copy over anything extra in the key (e.g. the zero byte
		    // at the end of "used value slots" in the termlist table).
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

#ifdef XAPIAN_HAS_GLASS_BACKEND
template<typename T> void
merge_docid_keyed(T* out, const vector<const GlassTable*>& inputs,
		  const vector<Xapian::docid>& offset,
		  Xapian::termcount& ut_lb, Xapian::termcount& ut_ub,
		  int table_type = 0)
{
    for (size_t i = 0; i < inputs.size(); ++i) {
	Xapian::docid off = offset[i];

	auto in = inputs[i];
	if (in->empty()) continue;

	GlassCursor cur(in);
	cur.rewind();

	string key;
	while (cur.next()) {
next_without_next:
	    // Adjust the key if this isn't the first database.
	    if (off) {
		Xapian::docid did;
		const char* d = cur.current_key.data();
		const char* e = d + cur.current_key.size();
		if (!unpack_uint_preserving_sort(&d, e, &did)) {
		    string msg = "Bad key in ";
		    msg += inputs[i]->get_path();
		    throw Xapian::DatabaseCorruptError(msg);
		}
		did += off;
		key.resize(0);
		pack_uint_preserving_sort(key, did);
		if (d != e) {
		    // Copy over anything extra in the key (e.g. the zero byte
		    // at the end of "used value slots" in the termlist table).
		    key.append(d, e - d);
		}
	    } else {
		key = cur.current_key;
	    }
	    if (table_type == Honey::TERMLIST) {
		if (termlist_key_is_values_used(key)) {
		    throw Xapian::DatabaseCorruptError("value slots used "
						       "entry without a "
						       "corresponding "
						       "termlist entry");
		}
		cur.read_tag();
		string tag = cur.current_tag;

		bool next_result = cur.next();
		bool next_already_done = true;
		unsigned bitmap_slots_used = 0;
		string encoded_slots_used;
		if (next_result &&
		    termlist_key_is_values_used(cur.current_key)) {
		    next_already_done = false;
		    cur.read_tag();
		    const string& valtag = cur.current_tag;

		    const char* p = valtag.data();
		    const char* end = p + valtag.size();

		    Xapian::VecCOW<Xapian::termpos> slots;

		    Xapian::valueno first_slot;
		    if (!unpack_uint(&p, end, &first_slot)) {
			throw_database_corrupt("Value slot encoding corrupt",
					       p);
		    }
		    slots.push_back(first_slot);
		    Xapian::valueno last_slot = first_slot;
		    while (p != end) {
			Xapian::valueno slot;
			if (!unpack_uint(&p, end, &slot)) {
			    throw_database_corrupt("Value slot encoding "
						   "corrupt", p);
			}
			slot += last_slot + 1;
			last_slot = slot;

			slots.push_back(slot);
		    }

		    if (slots.back() <= 6) {
			// Encode as a bitmap if only slots in the range 0-6
			// are used.
			for (auto slot : slots) {
			    bitmap_slots_used |= 1 << slot;
			}
		    } else {
			string enc;
			pack_uint(enc, last_slot);
			if (slots.size() > 1) {
			    BitWriter slots_used(enc);
			    slots_used.encode(first_slot, last_slot);
			    slots_used.encode(slots.size() - 2,
					      last_slot - first_slot);
			    slots_used.encode_interpolative(slots, 0,
							    slots.size() - 1);
			    encoded_slots_used = slots_used.freeze();
			} else {
			    encoded_slots_used = std::move(enc);
			}
		    }
		}

		const char* pos = tag.data();
		const char* end = pos + tag.size();

		string newtag;
		if (encoded_slots_used.empty()) {
		    newtag += char(bitmap_slots_used);
		} else {
		    auto size = encoded_slots_used.size();
		    if (size < 0x80) {
			newtag += char(0x80 | size);
		    } else {
			newtag += '\x80';
			pack_uint(newtag, size);
		    }
		    newtag += encoded_slots_used;
		}

		if (pos != end) {
		    Xapian::termcount doclen;
		    if (!unpack_uint(&pos, end, &doclen)) {
			throw_database_corrupt("doclen", pos);
		    }
		    Xapian::termcount termlist_size;
		    if (!unpack_uint(&pos, end, &termlist_size)) {
			throw_database_corrupt("termlist length", pos);
		    }

		    auto uniq_terms = min(termlist_size, doclen);
		    if (uniq_terms &&
			(ut_lb == 0 || uniq_terms < ut_lb)) {
			ut_lb = uniq_terms;
		    }
		    if (uniq_terms > ut_ub)
			ut_ub = uniq_terms;

		    pack_uint(newtag, termlist_size - 1);
		    pack_uint(newtag, doclen);

		    string current_term;
		    while (pos != end) {
			Xapian::termcount current_wdf = 0;

			if (!current_term.empty()) {
			    size_t reuse = static_cast<unsigned char>(*pos++);
			    newtag += char(reuse);

			    if (reuse > current_term.size()) {
				current_wdf = reuse / (current_term.size() + 1);
				reuse = reuse % (current_term.size() + 1);
			    }
			    current_term.resize(reuse);

			    if (pos == end)
				throw_database_corrupt("term", NULL);
			}

			size_t append = static_cast<unsigned char>(*pos++);
			if (size_t(end - pos) < append)
			    throw_database_corrupt("term", NULL);

			current_term.append(pos, append);
			pos += append;

			if (current_wdf) {
			    --current_wdf;
			} else {
			    if (!unpack_uint(&pos, end, &current_wdf)) {
				throw_database_corrupt("wdf", pos);
			    }
			    pack_uint(newtag, current_wdf);
			}

			newtag += char(append);
			newtag.append(current_term.end() - append,
				      current_term.end());
		    }
		}
		if (!newtag.empty())
		    out->add(key, newtag);
		if (!next_result) break;
		if (next_already_done) goto next_without_next;
	    } else {
		bool compressed = cur.read_tag(true);
		out->add(key, cur.current_tag, compressed);
	    }
	}
    }
}
#endif

}

using namespace HoneyCompact;

void
HoneyDatabase::compact(Xapian::Compactor* compactor,
		       const char* destdir,
		       int fd,
		       int source_backend,
		       const vector<const Xapian::Database::Internal*>& sources,
		       const vector<Xapian::docid>& offset,
		       Xapian::Compactor::compaction_level compaction,
		       unsigned flags,
		       Xapian::docid last_docid)
{
    // Currently unused for honey.
    (void)compaction;

    struct table_list {
	// The "base name" of the table.
	char name[9];
	// The type.
	Honey::table_type type;
	// Create tables after position lazily.
	bool lazy;
    };

    static const table_list tables[] = {
	// name		type			lazy
	{ "postlist",	Honey::POSTLIST,	false },
	{ "docdata",	Honey::DOCDATA,		true },
	{ "termlist",	Honey::TERMLIST,	false },
	{ "position",	Honey::POSITION,	true },
	{ "spelling",	Honey::SPELLING,	true },
	{ "synonym",	Honey::SYNONYM,		true }
    };
    const table_list* tables_end = tables +
	(sizeof(tables) / sizeof(tables[0]));

    const int FLAGS = Xapian::DB_DANGEROUS;

    bool single_file = (flags & Xapian::DBCOMPACT_SINGLE_FILE);
    bool multipass = (flags & Xapian::DBCOMPACT_MULTIPASS);
    if (single_file) {
	// FIXME: Support this combination - we need to put temporary files
	// somewhere.
	multipass = false;
    }

    if (single_file) {
	for (size_t i = 0; i != sources.size(); ++i) {
	    bool has_uncommitted_changes;
	    if (source_backend == Xapian::DB_BACKEND_GLASS) {
#ifdef XAPIAN_HAS_GLASS_BACKEND
		auto db = static_cast<const GlassDatabase*>(sources[i]);
		has_uncommitted_changes = db->has_uncommitted_changes();
#else
		throw Xapian::FeatureUnavailableError("Glass backend disabled");
#endif
	    } else {
		auto db = static_cast<const HoneyDatabase*>(sources[i]);
		has_uncommitted_changes = db->has_uncommitted_changes();
	    }
	    if (has_uncommitted_changes) {
		const char* m =
		    "Can't compact from a WritableDatabase with uncommitted "
		    "changes - either call commit() first, or create a new "
		    "Database object from the filename on disk";
		throw Xapian::InvalidOperationError(m);
	    }
	}
    }

    FlintLock lock(destdir ? destdir : "");
    if (!single_file) {
	string explanation;
	FlintLock::reason why = lock.lock(true, false, explanation);
	if (why != FlintLock::SUCCESS) {
	    lock.throw_databaselockerror(why, destdir, explanation);
	}
    }

    unique_ptr<HoneyVersion> version_file_out;
    if (single_file) {
	if (destdir) {
	    fd = open(destdir, O_RDWR|O_CREAT|O_TRUNC|O_BINARY|O_CLOEXEC, 0666);
	    if (fd < 0) {
		throw Xapian::DatabaseCreateError("open() failed", errno);
	    }
	}
	version_file_out.reset(new HoneyVersion(fd));
    } else {
	fd = -1;
	version_file_out.reset(new HoneyVersion(destdir));
    }

    // Set to true if stat() failed (which can happen if the files are > 2GB
    // and off_t is 32 bit) or one of the totals overflowed.
    bool bad_totals = false;
    off_t in_total = 0, out_total = 0;

    version_file_out->create();
    for (size_t i = 0; i != sources.size(); ++i) {
	bool source_single_file = false;
	if (source_backend == Xapian::DB_BACKEND_GLASS) {
#ifdef XAPIAN_HAS_GLASS_BACKEND
	    auto db = static_cast<const GlassDatabase*>(sources[i]);
	    auto& v_in = db->version_file;
	    auto& v_out = version_file_out;
	    // Glass backend doesn't track unique term bounds, hence setting
	    // them to 0. We calculate the unique term bounds as we convert the
	    // termlist table and fill in the correct values before we write out
	    // version_file_out.
	    v_out->merge_stats(v_in.get_doccount(),
			       v_in.get_doclength_lower_bound(),
			       v_in.get_doclength_upper_bound(),
			       v_in.get_wdf_upper_bound(),
			       v_in.get_total_doclen(),
			       v_in.get_spelling_wordfreq_upper_bound(),
			       0,
			       0);
	    source_single_file = db->single_file();
#else
	    Assert(false);
#endif
	} else {
	    auto db = static_cast<const HoneyDatabase*>(sources[i]);
	    version_file_out->merge_stats(db->version_file);
	    source_single_file = db->single_file();
	}
	if (source_single_file) {
	    // Add single file input DB sizes to in_total here.  For other
	    // databases, we sum per-table below.
	    string path;
	    sources[i]->get_backend_info(&path);
	    off_t db_size = file_size(path);
	    if (errno == 0) {
		// FIXME: check overflow and set bad_totals
		in_total += db_size;
	    } else {
		bad_totals = true;
	    }
	}
    }

    string fl_serialised;
#if 0
    if (single_file) {
	HoneyFreeList fl;
	fl.set_first_unused_block(1); // FIXME: Assumption?
	fl.pack(fl_serialised);
    }
#endif

    // FIXME: sort out indentation.
if (source_backend == Xapian::DB_BACKEND_GLASS) {
#ifndef XAPIAN_HAS_GLASS_BACKEND
    throw Xapian::FeatureUnavailableError("Glass backend disabled");
#else
    vector<HoneyTable*> tabs;
    tabs.reserve(tables_end - tables);
    off_t prev_size = 0;
    for (const table_list* t = tables; t < tables_end; ++t) {
	// The postlist table requires an N-way merge, adjusting the
	// headers of various blocks.  The spelling and synonym tables also
	// need special handling.  The other tables have keys sorted in
	// docid order, so we can merge them by simply copying all the keys
	// from each source table in turn.
	if (compactor)
	    compactor->set_status(t->name, string());

	string dest;
	if (!single_file) {
	    dest = destdir;
	    dest += '/';
	    dest += t->name;
	    dest += '.';
	}

	bool output_will_exist = !t->lazy;

	// Sometimes stat can fail for benign reasons (e.g. >= 2GB file
	// on certain systems).
	bool bad_stat = false;

	// We can't currently report input sizes if there's a single file DB
	// amongst the inputs.
	bool single_file_in = false;

	off_t in_size = 0;

	vector<const GlassTable*> inputs;
	inputs.reserve(sources.size());
	size_t inputs_present = 0;
	for (auto src : sources) {
	    auto db = static_cast<const GlassDatabase*>(src);
	    const GlassTable* table;
	    switch (t->type) {
		case Honey::POSTLIST:
		    table = &(db->postlist_table);
		    break;
		case Honey::DOCDATA:
		    table = &(db->docdata_table);
		    break;
		case Honey::TERMLIST:
		    table = &(db->termlist_table);
		    break;
		case Honey::POSITION:
		    table = &(db->position_table);
		    break;
		case Honey::SPELLING:
		    table = &(db->spelling_table);
		    break;
		case Honey::SYNONYM:
		    table = &(db->synonym_table);
		    break;
		default:
		    Assert(false);
		    return;
	    }

	    if (db->single_file()) {
		if (t->lazy && !GlassCursor(table).next()) {
		    // Lazy table with no entries, so essentially doesn't
		    // exist.
		} else {
		    // FIXME: Find actual size somehow?
		    // in_size += table->size() / 1024;
		    single_file_in = true;
		    output_will_exist = true;
		    ++inputs_present;
		}
	    } else {
		off_t db_size = file_size(table->get_path());
		if (errno == 0) {
		    // FIXME: check overflow and set bad_totals
		    in_total += db_size;
		    in_size += db_size / 1024;
		    output_will_exist = true;
		    ++inputs_present;
		} else if (errno != ENOENT) {
		    // We get ENOENT for an optional table.
		    bad_totals = bad_stat = true;
		    output_will_exist = true;
		    ++inputs_present;
		}
	    }
	    inputs.push_back(table);
	}

	// If any inputs lack a termlist table, suppress it in the output.
	if (t->type == Honey::TERMLIST && inputs_present != sources.size()) {
	    if (inputs_present != 0) {
		if (compactor) {
		    string m = str(inputs_present);
		    m += " of ";
		    m += str(sources.size());
		    m += " inputs present, so suppressing output";
		    compactor->set_status(t->name, m);
		}
		continue;
	    }
	    output_will_exist = false;
	}

	if (!output_will_exist) {
	    if (compactor)
		compactor->set_status(t->name, "doesn't exist");
	    continue;
	}

	HoneyTable* out;
	off_t table_start_offset = -1;
	if (single_file) {
	    if (t == tables) {
		// Start first table HONEY_VERSION_MAX_SIZE bytes in to allow
		// space for version file.  It's tricky to exactly know the
		// size of the version file beforehand.
		table_start_offset = lseek(fd, HONEY_VERSION_MAX_SIZE, SEEK_CUR);
		if (table_start_offset < 0)
		    throw Xapian::DatabaseError("lseek() failed", errno);
	    } else {
		table_start_offset = lseek(fd, 0, SEEK_CUR);
	    }
	    out = new HoneyTable(t->name, fd, version_file_out->get_offset(),
				 false, false);
	} else {
	    out = new HoneyTable(t->name, dest, false, t->lazy);
	}
	tabs.push_back(out);
	Honey::RootInfo* root_info = version_file_out->root_to_set(t->type);
	if (single_file) {
	    root_info->set_free_list(fl_serialised);
	    root_info->set_offset(table_start_offset);
	    out->open(FLAGS,
		      version_file_out->get_root(t->type),
		      version_file_out->get_revision());
	} else {
	    out->create_and_open(FLAGS, *root_info);
	}

	switch (t->type) {
	    case Honey::POSTLIST: {
		if (multipass && inputs.size() > 3) {
		    multimerge_postlists(compactor, out, destdir,
					 inputs, offset);
		} else {
		    merge_postlists(compactor, out, offset.begin(),
				    inputs.begin(), inputs.end());
		}
		break;
	    }
	    case Honey::SPELLING:
		merge_spellings(out, inputs.cbegin(), inputs.cend());
		break;
	    case Honey::SYNONYM:
		merge_synonyms(out, inputs.begin(), inputs.end());
		break;
	    case Honey::POSITION:
		merge_positions(out, inputs, offset);
		break;
	    default: {
		// DocData, Termlist
		auto& v_out = version_file_out;
		auto ut_lb = v_out->get_unique_terms_lower_bound();
		auto ut_ub = v_out->get_unique_terms_upper_bound();
		merge_docid_keyed(out, inputs, offset, ut_lb, ut_ub, t->type);
		version_file_out->set_unique_terms_lower_bound(ut_lb);
		version_file_out->set_unique_terms_upper_bound(ut_ub);
		break;
	    }
	}

	// Commit as revision 1.
	out->flush_db();
	out->commit(1, root_info);
	out->sync();
	if (single_file) fl_serialised = root_info->get_free_list();

	off_t out_size = 0;
	if (!bad_stat && !single_file_in) {
	    off_t db_size;
	    if (single_file) {
		db_size = file_size(fd);
	    } else {
		db_size = file_size(dest + HONEY_TABLE_EXTENSION);
	    }
	    if (errno == 0) {
		if (single_file) {
		    off_t old_prev_size = prev_size;
		    prev_size = db_size;
		    db_size -= old_prev_size;
		}
		// FIXME: check overflow and set bad_totals
		out_total += db_size;
		out_size = db_size / 1024;
	    } else if (errno != ENOENT) {
		bad_totals = bad_stat = true;
	    }
	}
	if (bad_stat) {
	    if (compactor)
		compactor->set_status(t->name,
				      "Done (couldn't stat all the DB files)");
	} else if (single_file_in) {
	    if (compactor)
		compactor->set_status(t->name,
				      "Done (table sizes unknown for single "
				      "file DB input)");
	} else {
	    string status;
	    if (out_size == in_size) {
		status = "Size unchanged (";
	    } else {
		off_t delta;
		if (out_size < in_size) {
		    delta = in_size - out_size;
		    status = "Reduced by ";
		} else {
		    delta = out_size - in_size;
		    status = "INCREASED by ";
		}
		if (in_size) {
		    status += str(100 * delta / in_size);
		    status += "% ";
		}
		status += str(delta);
		status += "K (";
		status += str(in_size);
		status += "K -> ";
	    }
	    status += str(out_size);
	    status += "K)";
	    if (compactor)
		compactor->set_status(t->name, status);
	}
    }

    // If compacting to a single file output and all the tables are empty, pad
    // the output so that it isn't mistaken for a stub database when we try to
    // open it.  For this it needs to at least HONEY_MIN_DB_SIZE in size.
    if (single_file && prev_size < HONEY_MIN_DB_SIZE) {
	out_total = HONEY_MIN_DB_SIZE;
#ifdef HAVE_FTRUNCATE
	if (ftruncate(fd, HONEY_MIN_DB_SIZE) < 0) {
	    throw Xapian::DatabaseError("Failed to set size of output "
					"database", errno);
	}
#else
	const off_t off = HONEY_MIN_DB_SIZE - 1;
	if (lseek(fd, off, SEEK_SET) != off || write(fd, "", 1) != 1) {
	    throw Xapian::DatabaseError("Failed to set size of output "
					"database", errno);
	}
#endif
    }

    if (single_file) {
	if (lseek(fd, version_file_out->get_offset(), SEEK_SET) == -1) {
	    throw Xapian::DatabaseError("lseek() failed", errno);
	}
    }
    version_file_out->set_last_docid(last_docid);
    string tmpfile = version_file_out->write(1, FLAGS);
    if (single_file) {
	off_t version_file_size = lseek(fd, 0, SEEK_CUR);
	if (version_file_size < 0) {
	    throw Xapian::DatabaseError("lseek() failed", errno);
	}
	if (version_file_size > HONEY_VERSION_MAX_SIZE) {
	    throw Xapian::DatabaseError("Didn't allow enough space for "
					"version file data");
	}
    }
    for (unsigned j = 0; j != tabs.size(); ++j) {
	tabs[j]->sync();
    }
    // Commit with revision 1.
    version_file_out->sync(tmpfile, 1, FLAGS);
    for (unsigned j = 0; j != tabs.size(); ++j) {
	delete tabs[j];
    }
#endif
} else {
    vector<HoneyTable*> tabs;
    tabs.reserve(tables_end - tables);
    off_t prev_size = HONEY_MIN_DB_SIZE;
    for (const table_list* t = tables; t < tables_end; ++t) {
	// The postlist table requires an N-way merge, adjusting the
	// headers of various blocks.  The spelling and synonym tables also
	// need special handling.  The other tables have keys sorted in
	// docid order, so we can merge them by simply copying all the keys
	// from each source table in turn.
	if (compactor)
	    compactor->set_status(t->name, string());

	string dest;
	if (!single_file) {
	    dest = destdir;
	    dest += '/';
	    dest += t->name;
	    dest += '.';
	}

	bool output_will_exist = !t->lazy;

	// Sometimes stat can fail for benign reasons (e.g. >= 2GB file
	// on certain systems).
	bool bad_stat = false;

	// We can't currently report input sizes if there's a single file DB
	// amongst the inputs.
	bool single_file_in = false;

	off_t in_size = 0;

	vector<const HoneyTable*> inputs;
	inputs.reserve(sources.size());
	size_t inputs_present = 0;
	for (auto src : sources) {
	    auto db = static_cast<const HoneyDatabase*>(src);
	    const HoneyTable* table;
	    switch (t->type) {
		case Honey::POSTLIST:
		    table = &(db->postlist_table);
		    break;
		case Honey::DOCDATA:
		    table = &(db->docdata_table);
		    break;
		case Honey::TERMLIST:
		    table = &(db->termlist_table);
		    break;
		case Honey::POSITION:
		    table = &(db->position_table);
		    break;
		case Honey::SPELLING:
		    table = &(db->spelling_table);
		    break;
		case Honey::SYNONYM:
		    table = &(db->synonym_table);
		    break;
		default:
		    Assert(false);
		    return;
	    }

	    if (db->single_file()) {
		if (t->lazy && !HoneyCursor(table).next()) {
		    // Lazy table with no entries, so essentially doesn't
		    // exist.
		} else {
		    // FIXME: Find actual size somehow?
		    // in_size += table->size() / 1024;
		    single_file_in = true;
		    output_will_exist = true;
		    ++inputs_present;
		}
	    } else {
		off_t db_size = file_size(table->get_path());
		if (errno == 0) {
		    // FIXME: check overflow and set bad_totals
		    in_total += db_size;
		    in_size += db_size / 1024;
		    output_will_exist = true;
		    ++inputs_present;
		} else if (errno != ENOENT) {
		    // We get ENOENT for an optional table.
		    bad_totals = bad_stat = true;
		    output_will_exist = true;
		    ++inputs_present;
		}
	    }
	    inputs.push_back(table);
	}

	// If any inputs lack a termlist table, suppress it in the output.
	if (t->type == Honey::TERMLIST && inputs_present != sources.size()) {
	    if (inputs_present != 0) {
		if (compactor) {
		    string m = str(inputs_present);
		    m += " of ";
		    m += str(sources.size());
		    m += " inputs present, so suppressing output";
		    compactor->set_status(t->name, m);
		}
		continue;
	    }
	    output_will_exist = false;
	}

	if (!output_will_exist) {
	    if (compactor)
		compactor->set_status(t->name, "doesn't exist");
	    continue;
	}

	HoneyTable* out;
	off_t table_start_offset = -1;
	if (single_file) {
	    if (t == tables) {
		// Start first table HONEY_VERSION_MAX_SIZE bytes in to allow
		// space for version file.  It's tricky to exactly know the
		// size of the version file beforehand.
		table_start_offset = lseek(fd, HONEY_VERSION_MAX_SIZE, SEEK_CUR);
		if (table_start_offset < 0)
		    throw Xapian::DatabaseError("lseek() failed", errno);
	    } else {
		table_start_offset = lseek(fd, 0, SEEK_CUR);
	    }
	    out = new HoneyTable(t->name, fd, version_file_out->get_offset(),
				 false, false);
	} else {
	    out = new HoneyTable(t->name, dest, false, t->lazy);
	}
	tabs.push_back(out);
	Honey::RootInfo* root_info = version_file_out->root_to_set(t->type);
	if (single_file) {
	    root_info->set_free_list(fl_serialised);
	    root_info->set_offset(table_start_offset);
	    out->open(FLAGS,
		      version_file_out->get_root(t->type),
		      version_file_out->get_revision());
	} else {
	    out->create_and_open(FLAGS, *root_info);
	}

	switch (t->type) {
	    case Honey::POSTLIST: {
		if (multipass && inputs.size() > 3) {
		    multimerge_postlists(compactor, out, destdir,
					 inputs, offset);
		} else {
		    merge_postlists(compactor, out, offset.begin(),
				    inputs.begin(), inputs.end());
		}
		break;
	    }
	    case Honey::SPELLING:
		merge_spellings(out, inputs.begin(), inputs.end());
		break;
	    case Honey::SYNONYM:
		merge_synonyms(out, inputs.begin(), inputs.end());
		break;
	    case Honey::POSITION:
		merge_positions(out, inputs, offset);
		break;
	    default:
		// DocData, Termlist
		merge_docid_keyed(out, inputs, offset);
		break;
	}

	// Commit as revision 1.
	out->flush_db();
	out->commit(1, root_info);
	out->sync();
	if (single_file) fl_serialised = root_info->get_free_list();

	off_t out_size = 0;
	if (!bad_stat && !single_file_in) {
	    off_t db_size;
	    if (single_file) {
		db_size = file_size(fd);
	    } else {
		db_size = file_size(dest + HONEY_TABLE_EXTENSION);
	    }
	    if (errno == 0) {
		if (single_file) {
		    off_t old_prev_size = prev_size;
		    prev_size = db_size;
		    db_size -= old_prev_size;
		}
		// FIXME: check overflow and set bad_totals
		out_total += db_size;
		out_size = db_size / 1024;
	    } else if (errno != ENOENT) {
		bad_totals = bad_stat = true;
	    }
	}
	if (bad_stat) {
	    if (compactor)
		compactor->set_status(t->name,
				      "Done (couldn't stat all the DB files)");
	} else if (single_file_in) {
	    if (compactor)
		compactor->set_status(t->name,
				      "Done (table sizes unknown for single "
				      "file DB input)");
	} else {
	    string status;
	    if (out_size == in_size) {
		status = "Size unchanged (";
	    } else {
		off_t delta;
		if (out_size < in_size) {
		    delta = in_size - out_size;
		    status = "Reduced by ";
		} else {
		    delta = out_size - in_size;
		    status = "INCREASED by ";
		}
		if (in_size) {
		    status += str(100 * delta / in_size);
		    status += "% ";
		}
		status += str(delta);
		status += "K (";
		status += str(in_size);
		status += "K -> ";
	    }
	    status += str(out_size);
	    status += "K)";
	    if (compactor)
		compactor->set_status(t->name, status);
	}
    }

    // If compacting to a single file output and all the tables are empty, pad
    // the output so that it isn't mistaken for a stub database when we try to
    // open it.  For this it needs to at least HONEY_MIN_DB_SIZE in size.
    if (single_file && prev_size < HONEY_MIN_DB_SIZE) {
	out_total = HONEY_MIN_DB_SIZE;
#ifdef HAVE_FTRUNCATE
	if (ftruncate(fd, HONEY_MIN_DB_SIZE) < 0) {
	    throw Xapian::DatabaseError("Failed to set size of output "
					"database", errno);
	}
#else
	const off_t off = HONEY_MIN_DB_SIZE - 1;
	if (lseek(fd, off, SEEK_SET) != off || write(fd, "", 1) != 1) {
	    throw Xapian::DatabaseError("Failed to set size of output "
					"database", errno);
	}
#endif
    }

    if (single_file) {
	if (lseek(fd, version_file_out->get_offset(), SEEK_SET) < 0) {
	    throw Xapian::DatabaseError("lseek() failed", errno);
	}
    }
    version_file_out->set_last_docid(last_docid);
    string tmpfile = version_file_out->write(1, FLAGS);
    for (unsigned j = 0; j != tabs.size(); ++j) {
	tabs[j]->sync();
    }
    // Commit with revision 1.
    version_file_out->sync(tmpfile, 1, FLAGS);
    for (unsigned j = 0; j != tabs.size(); ++j) {
	delete tabs[j];
    }
}

    if (!single_file) lock.release();

    if (!bad_totals && compactor) {
	string status;
	in_total /= 1024;
	out_total /= 1024;
	if (out_total == in_total) {
	    status = "Size unchanged (";
	} else {
	    off_t delta;
	    if (out_total < in_total) {
		delta = in_total - out_total;
		status = "Reduced by ";
	    } else {
		delta = out_total - in_total;
		status = "INCREASED by ";
	    }
	    if (in_total) {
		status += str(100 * delta / in_total);
		status += "% ";
	    }
	    status += str(delta);
	    status += "K (";
	    status += str(in_total);
	    status += "K -> ";
	}
	status += str(out_total);
	status += "K)";
	compactor->set_status("Total", status);
    }
}
