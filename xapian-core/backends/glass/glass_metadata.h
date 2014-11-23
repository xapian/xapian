/** @file glass_metadata.h
 * @brief Access to metadata for a glass database.
 */
/* Copyright (C) 2005,2007,2008,2009,2011 Olly Betts
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

#ifndef XAPIAN_INCLUDED_GLASS_METADATA_H
#define XAPIAN_INCLUDED_GLASS_METADATA_H

#include <xapian/intrusive_ptr.h>
#include <xapian/database.h>
#include <xapian/types.h>

#include "backends/alltermslist.h"
#include "glass_table.h"
#include "api/termlist.h"

#include <string>

class GlassCursor;

class GlassMetadataTermList : public AllTermsList {
    /// Copying is not allowed.
    GlassMetadataTermList(const GlassMetadataTermList &);

    /// Assignment is not allowed.
    void operator=(const GlassMetadataTermList &);

    /// Keep a reference to our database to stop it being deleted.
    Xapian::Internal::intrusive_ptr<const Xapian::Database::Internal> database;

    /** A cursor which runs through the postlist table reading metadata keys.
     */
    GlassCursor * cursor;

    /** The prefix that all returned keys must have.
     */
    std::string prefix;

  public:
    GlassMetadataTermList(Xapian::Internal::intrusive_ptr<const Xapian::Database::Internal> database_,
			  GlassCursor * cursor_, const std::string &prefix_);

    ~GlassMetadataTermList();

    /** Returns the current termname.
     *
     *  Either next() or skip_to() must have been called before this
     *  method can be called.
     */
    std::string get_termname() const;

    /** Return the term frequency for the term at the current position.
     *
     *  Not meaningful for a GlassMetadataTermList.
     */
    Xapian::doccount get_termfreq() const;

    /** Return the collection frequency for the term at the current position.
     *
     *  Not meaningful for a GlassMetadataTermList.
     */
    Xapian::termcount get_collection_freq() const;

    /// Advance to the next term in the list.
    TermList * next();

    /// Advance to the first key which is >= @a key.
    TermList * skip_to(const std::string &key);

    /// True if we're off the end of the list
    bool at_end() const;
};

#endif // XAPIAN_INCLUDED_GLASS_METADATA_H
