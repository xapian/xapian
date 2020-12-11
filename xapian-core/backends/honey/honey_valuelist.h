/** @file
 * @brief Honey class for value streams.
 */
/* Copyright (C) 2007,2008,2009,2011 Olly Betts
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

#ifndef XAPIAN_INCLUDED_HONEY_VALUELIST_H
#define XAPIAN_INCLUDED_HONEY_VALUELIST_H

#include "backends/valuelist.h"
#include "honey_values.h"

class HoneyCursor;
class HoneyDatabase;

/// Honey class for value streams.
class HoneyValueList : public Xapian::ValueIterator::Internal {
    /// Don't allow assignment.
    void operator=(const HoneyValueList&);

    /// Don't allow copying.
    HoneyValueList(const HoneyValueList&);

    HoneyCursor* cursor;

    Honey::ValueChunkReader reader;

    Xapian::valueno slot;

    Xapian::Internal::intrusive_ptr<const HoneyDatabase> db;

    /// Update @a reader to use the chunk currently pointed to by @a cursor.
    bool update_reader();

  public:
    HoneyValueList(Xapian::valueno slot_, const HoneyDatabase* db_)
	: cursor(NULL), slot(slot_), db(db_) { }

    ~HoneyValueList();

    Xapian::docid get_docid() const;

    Xapian::valueno get_valueno() const;

    std::string get_value() const;

    bool at_end() const;

    void next();

    void skip_to(Xapian::docid);

    std::string get_description() const;
};

#endif // XAPIAN_INCLUDED_HONEY_VALUELIST_H
