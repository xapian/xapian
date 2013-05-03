/** @file chert_valuelist.h
 * @brief Chert class for value streams.
 */
/* Copyright (C) 2007,2008,2011 Olly Betts
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

#ifndef XAPIAN_INCLUDED_CHERT_VALUELIST_H
#define XAPIAN_INCLUDED_CHERT_VALUELIST_H

#include "backends/valuelist.h"
#include "chert_values.h"

class ChertCursor;
class ChertDatabase;

/// Chert class for value streams.
class ChertValueList : public Xapian::ValueIterator::Internal {
    /// Don't allow assignment.
    void operator=(const ChertValueList &);

    /// Don't allow copying.
    ChertValueList(const ChertValueList &);

    ChertCursor * cursor;

    ValueChunkReader reader;

    Xapian::valueno slot;

    Xapian::Internal::intrusive_ptr<const ChertDatabase> db;

    /// Update @a reader to use the chunk currently pointed to by @a cursor.
    bool update_reader();

  public:
    ChertValueList(Xapian::valueno slot_,
		   Xapian::Internal::intrusive_ptr<const ChertDatabase> db_)
	: cursor(NULL), slot(slot_), db(db_) { }

    ~ChertValueList();

    Xapian::docid get_docid() const;

    Xapian::valueno get_valueno() const;

    std::string get_value() const;

    bool at_end() const;

    void next();

    void skip_to(Xapian::docid);

    bool check(Xapian::docid did);

    std::string get_description() const;
};

#endif // XAPIAN_INCLUDED_CHERT_VALUELIST_H
