/** @file honey_alldocspostlist.h
 * @brief A PostList which iterates over all documents in a HoneyDatabase.
 */
/* Copyright (C) 2006,2007,2008,2009,2017 Olly Betts
 * Copyright (C) 2008 Lemur Consulting Ltd
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

#ifndef XAPIAN_INCLUDED_HONEY_ALLDOCSPOSTLIST_H
#define XAPIAN_INCLUDED_HONEY_ALLDOCSPOSTLIST_H

#include <string>

#include "api/leafpostlist.h"
#include "pack.h"

class HoneyCursor;
class HoneyDatabase;

namespace Honey {

/** Generate a key for a doc len chunk. */
inline std::string
make_doclenchunk_key(Xapian::docid did)
{
    std::string key("\0\xe0", 2);
    if (did > 1) pack_uint_preserving_sort(key, did);
    return key;
}

inline Xapian::docid
docid_from_key(const std::string& key)
{
    const char * p = key.data();
    const char * end = p + key.length();
    // Fail if not a doclen chunk key.
    if (end - p < 2 || *p++ != '\0' || *p++ != '\xe0') return 0;
    if (p == end)
	return 1;
    Xapian::docid did;
    if (!unpack_uint_preserving_sort(&p, end, &did))
	throw Xapian::DatabaseCorruptError("bad value key");
    return did;
}

class DocLenChunkReader {
    unsigned const char *p;
    unsigned const char *end;

    Xapian::docid did;

    Xapian::termcount doclen;

  public:
    /// Create a DocLenChunkReader which is already at_end().
    DocLenChunkReader() : p(NULL) { }

    DocLenChunkReader(const char * p_, size_t len, Xapian::docid did_) {
	assign(p_, len, did_);
    }

    void assign(const char * p_, size_t len, Xapian::docid did_);

    bool at_end() const { return p == NULL; }

    Xapian::docid get_docid() const { return did; }

    Xapian::termcount get_doclength() const { return doclen; }

    void next();

    void skip_to(Xapian::docid target);
};

}

class HoneyAllDocsPostList : public LeafPostList {
    /// Don't allow assignment.
    HoneyAllDocsPostList& operator=(const HoneyAllDocsPostList&) = delete;

    /// Don't allow copying.
    HoneyAllDocsPostList(const HoneyAllDocsPostList&) = delete;

    /// Cursor on the postlist table.
    HoneyCursor* cursor;

    Honey::DocLenChunkReader reader;

    /// The number of documents in the database.
    Xapian::doccount doccount;

    /// Update @a reader to use the chunk currently pointed to by @a cursor.
    bool update_reader();

  public:
    HoneyAllDocsPostList(const HoneyDatabase* db_, Xapian::doccount doccount_);

    ~HoneyAllDocsPostList();

    Xapian::doccount get_termfreq() const;

    Xapian::termcount get_doclength() const;

    Xapian::docid get_docid() const;

    Xapian::termcount get_wdf() const;

    bool at_end() const;

    PostList* next(double w_min);

    PostList* skip_to(Xapian::docid did, double w_min);

    PostList* check(Xapian::docid did, double w_min, bool& valid);

    std::string get_description() const;
};

#endif // XAPIAN_INCLUDED_HONEY_ALLDOCSPOSTLIST_H
