/** @file brass_valuelist.h
 * @brief Brass class for value streams.
 */
/* Copyright (C) 2007,2008,2009 Olly Betts
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

#ifndef XAPIAN_INCLUDED_BRASS_VALUELIST_H
#define XAPIAN_INCLUDED_BRASS_VALUELIST_H

#include "valuelist.h"
#include "brass_values.h"

class BrassCursor;
class BrassDatabase;

/// Brass class for value streams.
class BrassValueList : public Xapian::ValueIterator::Internal {
    /// Don't allow assignment.
    void operator=(const BrassValueList &);

    /// Don't allow copying.
    BrassValueList(const BrassValueList &);

    BrassCursor * cursor;

    Brass::ValueChunkReader reader;

    Xapian::valueno slot;

    Xapian::Internal::RefCntPtr<const BrassDatabase> db;

    /// Update @a reader to use the chunk currently pointed to by @a cursor.
    bool update_reader();

  public:
    BrassValueList(Xapian::valueno slot_,
		   Xapian::Internal::RefCntPtr<const BrassDatabase> db_)
	: cursor(NULL), slot(slot_), db(db_) { }

    ~BrassValueList();

    Xapian::docid get_docid() const;

    Xapian::valueno get_valueno() const;

    std::string get_value() const;

    bool at_end() const;

    void next();

    void skip_to(Xapian::docid);

    bool check(Xapian::docid did);

    std::string get_description() const;
};

#endif // XAPIAN_INCLUDED_BRASS_VALUELIST_H
