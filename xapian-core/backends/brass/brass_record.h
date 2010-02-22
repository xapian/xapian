/** @file brass_record.h
 * @brief Subclass of BrassTable which holds document data.
 */
/* Copyright (C) 2007,2008,2009,2010 Olly Betts
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

#ifndef XAPIAN_INCLUDED_BRASS_RECORD_H
#define XAPIAN_INCLUDED_BRASS_RECORD_H

#include <xapian/types.h>

#include "brass_defs.h"
#include "brass_lazytable.h"
#include "pack.h"

#include <string>

class BrassRecordTable : public BrassLazyTable {
  public:
    static std::string make_key(Xapian::docid did) {
	std::string key;
	pack_uint_preserving_sort(key, did);
	return key;
    }

    /** Create a new BrassTermListTable object.
     *
     *  This method does not create or open the table on disk - you
     *  must call the create() or open() methods respectively!
     *
     *  @param dbdir	    The directory the brass database is stored in.
     *  @param readonly_    true if we're opening read-only, else false.
     */
    BrassRecordTable(const std::string & dbdir, bool readonly_)
	: BrassLazyTable("record", dbdir + "/record.", readonly_, COMPRESS) { }

    /** Get the document data for document @a did.
     *
     *  If document did doesn't exist, the empty string is returned.
     *
     *  @param did	The docid to set the document data for.
     */
    std::string get_document_data(Xapian::docid did) const {
	// We don't store the document data if it is empty.
	std::string data;
	(void)get(make_key(did), data);
	return data;
    }

    /** Set the document data for document @a did.
     *
     *  If document did may already exist, use replace_document_data() instead.
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
     */
    void delete_document_data(Xapian::docid did) { del(make_key(did)); }
};

#endif // XAPIAN_INCLUDED_BRASS_RECORD_H
