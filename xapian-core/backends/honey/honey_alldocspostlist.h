/** @file
 * @brief A PostList which iterates over all documents in a HoneyDatabase.
 */
/* Copyright (C) 2006,2007,2008,2009,2017,2018 Olly Betts
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

#ifndef PACKAGE
# error config.h must be included first in each C++ source file
#endif

#include "backends/leafpostlist.h"
#include "honey_defs.h"
#include "pack.h"
#include "wordaccess.h"

#include <string>

class HoneyCursor;
class HoneyDatabase;

namespace Honey {

/** Generate a key for a doclen chunk. */
inline std::string
make_doclenchunk_key(Xapian::docid last_did)
{
    std::string key(1, '\0');
    Assert(last_did != 0);
#ifdef DO_CLZ
    int width = (do_clz(last_did) >> 3) + 1;
#else
    int width = 0;
    for (auto v = last_did; v; v >>= 8) {
	++width;
    }
#endif
    key += char(Honey::KEY_DOCLEN_CHUNK + width - 1);
    Xapian::docid v = last_did;
#ifndef WORDS_BIGENDIAN
    v = do_bswap(v);
#endif
    key.append(reinterpret_cast<const char*>(&v) + (sizeof(v) - width), width);
    return key;
}

inline Xapian::docid
docid_from_key(const std::string& key)
{
    const char* p = key.data();
    const char* end = p + key.length();
    if (end - p < 3 || *p++ != '\0') {
	// Not a doclen chunk key.
	return 0;
    }
    unsigned char code = *p++;
    if (code < Honey::KEY_DOCLEN_CHUNK || code > Honey::KEY_DOCLEN_CHUNK_HI) {
	// Also not a doclen chunk key.
	return 0;
    }

    size_t width = (code - Honey::KEY_DOCLEN_CHUNK) + 1;
    AssertEq(width, size_t(end - p));
    Xapian::docid v = 0;
    memcpy(reinterpret_cast<char*>(&v) + (sizeof(v) - width), p, width);
#ifndef WORDS_BIGENDIAN
    v = do_bswap(v);
#endif
    Assert(v != 0);
    return v;
}

class DocLenChunkReader {
    unsigned const char* p;
    unsigned const char* end;

    Xapian::docid did;

    Xapian::termcount doclen;

    unsigned width;

    bool read_doclen(const unsigned char* q);

  public:
    /// Create a DocLenChunkReader which is already at_end().
    DocLenChunkReader() : p(NULL) { }

    /// Update to use the chunk currently pointed to by @a cursor.
    bool update(HoneyCursor* cursor);

    bool at_end() const { return p == NULL; }

    Xapian::docid get_docid() const { return did; }

    Xapian::termcount get_doclength() const { return doclen; }

    bool next();

    bool skip_to(Xapian::docid target);

    /** Searches the whole chunk (skip_to() only advances).
     *
     *  Don't call this method and any of next()/skip_to()/at_end() on the same
     *  object (unless there's an intervening call to update()).
     *
     *  Return false if this isn't the right chunk.
     */
    bool find_doclength(Xapian::docid target);

    /// Return the last document length in this chunk.
    Xapian::termcount back() {
	(void)read_doclen(end - width);
	return doclen;
    }
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

    Xapian::termcount get_wdf_upper_bound() const;

    std::string get_description() const;
};

#endif // XAPIAN_INCLUDED_HONEY_ALLDOCSPOSTLIST_H
