/** @file brass_document.h
 * @brief A document read from a BrassDatabase.
 */
/* Copyright (C) 2008,2009,2010 Olly Betts
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

#ifndef XAPIAN_INCLUDED_BRASS_DOCUMENT_H
#define XAPIAN_INCLUDED_BRASS_DOCUMENT_H

#include "brass_record.h"
#include "brass_values.h"
#include "database.h"
#include "document.h"

/// A document read from a BrassDatabase.
class BrassDocument : public Xapian::Document::Internal {
    /// Don't allow assignment.
    void operator=(const BrassDocument &);

    /// Don't allow copying.
    BrassDocument(const BrassDocument &);

    /// Used for lazy access to document values.
    const BrassValueManager *value_manager;

    /// Used for lazy access to document data.
    const BrassRecordTable *record_table;

    /// BrassDatabase::open_document() needs to call our private constructor.
    friend class BrassDatabase;

    /// Private constructor - only called by BrassDatabase::open_document().
    BrassDocument(Xapian::Internal::RefCntPtr<const Xapian::Database::Internal> db,
		  Xapian::docid did_,
		  const BrassValueManager *value_manager_,
		  const BrassRecordTable *record_table_)
	: Xapian::Document::Internal(db, did_),
	  value_manager(value_manager_), record_table(record_table_) { }

  public:
    /** Implementation of virtual methods @{ */
    string do_get_value(Xapian::valueno slot) const;
    void do_get_all_values(map<Xapian::valueno, string> & values_) const;
    string do_get_data() const;
    /** @} */
};

#endif // XAPIAN_INCLUDED_BRASS_DOCUMENT_H
