/** @file glass_termlisttable.h
 * @brief Subclass of GlassTable which holds termlists.
 */
/* Copyright (C) 2007,2008,2009,2010,2013,2014 Olly Betts
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

#ifndef XAPIAN_INCLUDED_GLASS_TERMLISTTABLE_H
#define XAPIAN_INCLUDED_GLASS_TERMLISTTABLE_H

#include <xapian/constants.h>
#include <xapian/types.h>

#include "glass_lazytable.h"
#include "pack.h"

#include <string>

namespace Xapian {
class Document;
}

class GlassTermListTable : public GlassLazyTable {
  public:
    static std::string make_key(Xapian::docid did) {
	std::string key;
	pack_uint_preserving_sort(key, did);
	return key;
    }

    /** Create a new GlassTermListTable object.
     *
     *  This method does not create or open the table on disk - you
     *  must call the create() or open() methods respectively!
     *
     *  @param dbdir	    The directory the glass database is stored in.
     *  @param readonly	    true if we're opening read-only, else false.
     */
    GlassTermListTable(const std::string & dbdir, bool readonly)
	: GlassLazyTable("termlist", dbdir + "/termlist.", readonly,
			 Z_DEFAULT_STRATEGY) { }

    /** Set the termlist data for document @a did.
     *
     *  Any existing data is replaced.
     *
     *  @param did	The docid to set the termlist data for.
     *  @param doc	The Xapian::Document object to read term data from.
     *  @param doclen	The document length.
     */
    void set_termlist(Xapian::docid did, const Xapian::Document & doc,
		      Xapian::termcount doclen);

    /** Delete the termlist data for document @a did.
     *
     *  @param did  The docid to delete the termlist data for.
     */
    void delete_termlist(Xapian::docid did) { del(make_key(did)); }

    /** Conditionally lazy override of GlassLazyTable::create_and_open().
     *
     *  Only create lazily if Xapian::DB_NO_TERMLIST is set in flags_.
     *
     *  This method isn't virtual, but we never call it such that it needs to
     *  be.
     */
    void create_and_open(int flags_, unsigned blocksize_) {
	if (flags_ & Xapian::DB_NO_TERMLIST) {
	    GlassLazyTable::create_and_open(flags_, blocksize_);
	} else {
	    GlassTable::create_and_open(flags_, blocksize_);
	}
    }
};

#endif // XAPIAN_INCLUDED_GLASS_TERMLISTTABLE_H
