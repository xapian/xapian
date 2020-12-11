/** @file
 * @brief Access to metadata for a chert database.
 */
/* Copyright (C) 2005,2007,2008,2011 Olly Betts
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

#ifndef XAPIAN_INCLUDED_CHERT_METADATA_H
#define XAPIAN_INCLUDED_CHERT_METADATA_H

#include "xapian/intrusive_ptr.h"
#include <xapian/database.h>
#include <xapian/types.h>

#include "backends/alltermslist.h"
#include "chert_table.h"
#include "api/termlist.h"

#include <string>

class ChertCursor;

class ChertMetadataTermList : public AllTermsList {
    /// Copying is not allowed.
    ChertMetadataTermList(const ChertMetadataTermList &);

    /// Assignment is not allowed.
    void operator=(const ChertMetadataTermList &);

    /// Keep a reference to our database to stop it being deleted.
    Xapian::Internal::intrusive_ptr<const Xapian::Database::Internal> database;

    /** A cursor which runs through the postlist table reading metadata keys.
     */
    ChertCursor * cursor;

    /** The prefix that all returned keys must have.
     */
    std::string prefix;

  public:
    ChertMetadataTermList(Xapian::Internal::intrusive_ptr<const Xapian::Database::Internal> database_,
			  ChertCursor * cursor_, const std::string &prefix_);

    ~ChertMetadataTermList();

    /** Returns the current termname.
     *
     *  Either next() or skip_to() must have been called before this
     *  method can be called.
     */
    std::string get_termname() const;

    /** Return the term frequency for the term at the current position.
     *
     *  Not meaningful for a ChertMetadataTermList.
     */
    Xapian::doccount get_termfreq() const;

    /// Advance to the next term in the list.
    TermList * next();

    /// Advance to the first key which is >= @a key.
    TermList * skip_to(const std::string &key);

    /// True if we're off the end of the list
    bool at_end() const;
};

#endif // XAPIAN_INCLUDED_CHERT_METADATA_H
