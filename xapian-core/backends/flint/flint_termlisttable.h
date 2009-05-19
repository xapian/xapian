/** @file flint_termlisttable.h
 * @brief Subclass of FlintTable which holds termlists.
 */
/* Copyright (C) 2007 Olly Betts
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

#ifndef XAPIAN_INCLUDED_FLINT_TERMLISTTABLE_H
#define XAPIAN_INCLUDED_FLINT_TERMLISTTABLE_H

#include <xapian/types.h>

#include "flint_table.h"
#include "flint_utils.h"

#include <string>

namespace Xapian {
class Document;
}

class FlintTermListTable : public FlintTable {
  public:
    /** Create a new FlintTermListTable object.
     *
     *  This method does not create or open the table on disk - you
     *  must call the create() or open() methods respectively!
     *
     *  @param dbdir	    The directory the flint database is stored in.
     *  @param readonly	    true if we're opening read-only, else false.
     */
    FlintTermListTable(const std::string & dbdir, bool readonly)
	: FlintTable("termlist", dbdir + "/termlist.", readonly, Z_DEFAULT_STRATEGY) { }

    /** Set the termlist data for document @a did.
     *
     *  Any existing data is replaced.
     *
     *  @param did	The docid to set the termlist data for.
     *  @param doc	The Xapian::Document object to read term data from.
     *  @param doclen	The document length.
     */
    void set_termlist(Xapian::docid did, const Xapian::Document & doc,
		      flint_doclen_t doclen);

    /** Delete the termlist data for document @a did.
     *
     *  @param did  The docid to delete the termlist data for.
     */
    void delete_termlist(Xapian::docid did) { del(flint_docid_to_key(did)); }

    /// Returns the document length for document @a did.
    flint_doclen_t get_doclength(Xapian::docid did) const;
};

#endif // XAPIAN_INCLUDED_FLINT_TERMLISTTABLE_H
