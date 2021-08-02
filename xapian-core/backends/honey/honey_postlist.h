/** @file
 * @brief PostList in a honey database.
 */
/* Copyright (C) 2007,2009,2011,2013,2015,2016,2017,2018 Olly Betts
 * Copyright (C) 2009 Lemur Consulting Ltd
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_HONEY_POSTLIST_H
#define XAPIAN_INCLUDED_HONEY_POSTLIST_H

#include "backends/leafpostlist.h"
#include "honey_positionlist.h"
#include "pack.h"

#include <string>

class HoneyCursor;
class HoneyDatabase;

namespace Honey {

/** Generate a key for a posting initial chunk. */
inline std::string
make_postingchunk_key(const std::string& term)
{
    std::string key;
    pack_string_preserving_sort(key, term, true);
    return key;
}

/** Generate a key for a posting continuation chunk. */
inline std::string
make_postingchunk_key(const std::string& term, Xapian::docid did)
{
    std::string key;
    pack_string_preserving_sort(key, term);
    pack_uint_preserving_sort(key, did);
    return key;
}

inline Xapian::docid
docid_from_key(const std::string& term, const std::string& key)
{
    if (key.size() < term.size()) {
	// A key can't be shorter than the term it contains.
	return false;
    }
    const char* p = key.data();
    const char* end = p + key.size();
    // Most terms don't contain zero bytes, so we could optimise this.
    std::string term_in_key;
    // FIXME: the next key might not be for a postlist chunk...
    if (!unpack_string_preserving_sort(&p, end, term_in_key))
	throw Xapian::DatabaseCorruptError("bad postlist key");
    if (term_in_key != term)
	return false;
    Xapian::docid did;
    if (!unpack_uint_preserving_sort(&p, end, &did))
	throw Xapian::DatabaseCorruptError("bad postlist key");
    return did;
}

class PostingChunkReader {
    const char* p;
    const char* end;

    Xapian::docid did;

    Xapian::termcount wdf;

    /// The last docid in this chunk.
    Xapian::docid last_did;

    Xapian::doccount termfreq;

    /** Value "to do with" collection frequency.
     *
     *  In order to keep this class small in size the meaning of the value of
     *  collfreq_info depends on the context (which can be determined from the
     *  values of other members):
     *
     *  * if (collfreq == 0 || tf <= 2) collfreq_info = collfreq;
     *
     *  * otherwise:
     *
     *    + if wdf is wdf_same for all documents (with the possible exception
     *      of the first) then to start with:
     *
     *      collfreq_info = wdf_same | TOP_BIT_SET
     *
     *      and once we've moved onto the second entry:
     *
     *      collfreq_info = 0
     *
     *    + else collfreq_info = 1
     */
    Xapian::termcount collfreq_info;

  public:
    /// Create an uninitialised PostingChunkReader.
    PostingChunkReader() : p(NULL) { }

    /// Initialise already at_end().
    void init() {
	p = NULL;
	termfreq = 0;
    }

    /// Initialise.
    void init(Xapian::doccount tf, Xapian::termcount cf_info) {
	p = NULL;
	termfreq = tf;
	collfreq_info = cf_info;
    }

    void assign(const char* p_, size_t len, Xapian::docid did);

    void assign(const char* p_, size_t len, Xapian::docid did_,
		Xapian::docid last_did_in_chunk,
		Xapian::termcount wdf_);

    bool at_end() const { return p == NULL; }

    Xapian::doccount get_termfreq() const { return termfreq; }

    Xapian::docid get_docid() const { return did; }

    Xapian::termcount get_wdf() const { return wdf; }

    /// Advance, returning false if we've run out of data.
    bool next();

    /// Skip ahead, returning false if we've run out of data.
    bool skip_to(Xapian::docid target);
};

}

/** PostList in a honey database. */
class HoneyPostList : public LeafPostList {
    /// Don't allow assignment.
    HoneyPostList& operator=(const HoneyPostList&) = delete;

    /// Don't allow copying.
    HoneyPostList(const HoneyPostList&) = delete;

    /// Cursor on the postlist table.
    HoneyCursor* cursor;

    Honey::PostingChunkReader reader;

    /// The highest document id in this posting list.
    Xapian::docid last_did;

    /// HoneyDatabase to get position table object from.
    const HoneyDatabase* db;

    /** Maximum wdf for this postlist.
     *
     *  We store this for honey, so it's exact - not just an upper bound.
     */
    Xapian::termcount wdf_max;

    /** Needed so that first next() does nothing.
     *
     *  FIXME: Can we arrange not to need this?
     */
    bool started = false;

    /// Update @a reader to use the chunk currently pointed to by @a cursor.
    bool update_reader();

  public:
    /// Create HoneyPostList from already positioned @a cursor_.
    HoneyPostList(const HoneyDatabase* db_,
		  const std::string& term_,
		  HoneyCursor* cursor_);

    ~HoneyPostList();

    Xapian::doccount get_termfreq() const;

    LeafPostList* open_nearby_postlist(const std::string& term_,
				       bool need_read_pos) const;

    Xapian::docid get_docid() const;

    Xapian::termcount get_wdf() const;

    bool at_end() const;

    PositionList* open_position_list() const;

    PostList* next(double w_min);

    PostList* skip_to(Xapian::docid did, double w_min);

    Xapian::termcount get_wdf_upper_bound() const;

    std::string get_description() const;
};

/** PostList in a honey database with positions.
 *
 *  Use a special subclass to avoid the size cost for the common case where we
 *  don't want positional data.
 */
class HoneyPosPostList : public HoneyPostList {
    /** PositionList object to reuse for OP_NEAR and OP_PHRASE.
     *
     *  This saves the overhead of creating objects for every document
     *  considered.
     */
    HoneyRePositionList position_list;

  public:
    HoneyPosPostList(const HoneyDatabase* db_,
		     const std::string& term_,
		     HoneyCursor* cursor_);

    PositionList* read_position_list();

    std::string get_description() const;
};

#endif // XAPIAN_INCLUDED_HONEY_POSTLIST_H
