/** @file
 * @brief Subclass of GlassTable which holds document data.
 */
/* Copyright (C) 2007,2008,2009,2010,2014,2016 Olly Betts
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

#ifndef XAPIAN_INCLUDED_GLASS_DOCDATA_H
#define XAPIAN_INCLUDED_GLASS_DOCDATA_H

#include <xapian/types.h>

#include "glass_lazytable.h"
#include "pack.h"

#include <string>

class GlassDocDataTable : public GlassLazyTable {
  public:
    static std::string make_key(Xapian::docid did) {
	std::string key;
	pack_uint_preserving_sort(key, did);
	return key;
    }

    /** Create a new GlassDocDataTable object.
     *
     *  This method does not create or open the table on disk - you
     *  must call the create() or open() methods respectively!
     *
     *  @param dbdir	    The directory the glass database is stored in.
     *  @param readonly     true if we're opening read-only, else false.
     */
    GlassDocDataTable(const std::string & dbdir, bool readonly)
	: GlassLazyTable("docdata", dbdir + "/docdata.", readonly) { }

    GlassDocDataTable(int fd, off_t offset_, bool readonly)
	: GlassLazyTable("docdata", fd, offset_, readonly) { }

    /** Get the document data for document @a did.
     *
     *  If the document doesn't exist, the empty string is returned.
     *
     *  @param did	The docid to set the document data for.
     */
    std::string get_document_data(Xapian::docid did) const {
	// We don't store the document data if it is empty.
	std::string data;
	(void)get_exact_entry(make_key(did), data);
	return data;
    }

    /** Set the document data for document @a did.
     *
     *  If the document might already exist, use replace_document_data()
     *  instead.
     *
     *  @param did	The docid to set the document data for.
     *  @param data	The document data to set.
     */
    void add_document_data(Xapian::docid did, const std::string & data) {
	// We don't store the document data if it is empty.
	if (!data.empty())
	    add(make_key(did), data);
    }

    /** Replace the document data for document @a did.
     *
     *  Any existing data is replaced.
     *
     *  @param did	The docid to replace the document data for.
     *  @param data	The document data to set.
     */
    void replace_document_data(Xapian::docid did, const std::string & data) {
	if (data.empty()) {
	    // We don't store the document data if it is empty.
	    delete_document_data(did);
	    return;
	}
	add(make_key(did), data);
    }

    /** Delete the document data for document @a did.
     *
     *  @param did	The docid to delete the document data for.
     *
     *  @return true if document data was actually removed (false means either
     *		     there's no such document, or the document has no data).
     */
    bool delete_document_data(Xapian::docid did) { return del(make_key(did)); }

    void readahead_for_document(Xapian::docid did) const {
	readahead_key(make_key(did));
    }
};

#endif // XAPIAN_INCLUDED_GLASS_DOCDATA_H
