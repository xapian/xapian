/** @file slowvaluelist.h
 * @brief Slow implementation for backends which don't streamed values.
 */
/* Copyright (C) 2007,2008,2011,2014 Olly Betts
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

#ifndef XAPIAN_INCLUDED_SLOWVALUELIST_H
#define XAPIAN_INCLUDED_SLOWVALUELIST_H

#include "valuelist.h"

#include "database.h"

/** Slow implementation for backends which don't streamed values.
 *
 *  Used by inmemory and remote backends.
 */
class SlowValueList : public ValueList {
    /// Don't allow assignment.
    void operator=(const SlowValueList &);

    /// Don't allow copying.
    SlowValueList(const SlowValueList &);

    /// The subdatabase.
    Xapian::Internal::intrusive_ptr<const Xapian::Database::Internal> db;

    /// The last docid in the database, or 0 if we're at_end.
    Xapian::docid last_docid;

    /// The value slot we're iterating over.
    Xapian::valueno slot;

    /// The value at the current position.
    std::string current_value;

    /// The document id at the current position.
    Xapian::docid current_did;

  public:
    SlowValueList(const Xapian::Database::Internal * db_, Xapian::valueno slot_)
	: db(db_), last_docid(db_->get_lastdocid()), slot(slot_), current_did(0)
    { }

    Xapian::docid get_docid() const;

    std::string get_value() const;

    Xapian::valueno get_valueno() const;

    bool at_end() const;

    void next();

    void skip_to(Xapian::docid);

    bool check(Xapian::docid did);

    std::string get_description() const;
};

#endif // XAPIAN_INCLUDED_SLOWVALUELIST_H
