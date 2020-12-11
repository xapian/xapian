/** @file
 * @brief A document read from a GlassDatabase.
 */
/* Copyright (C) 2008,2009,2010,2011 Olly Betts
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

#ifndef XAPIAN_INCLUDED_GLASS_DOCUMENT_H
#define XAPIAN_INCLUDED_GLASS_DOCUMENT_H

#include "glass_docdata.h"
#include "glass_values.h"
#include "backends/database.h"
#include "backends/document.h"

/// A document read from a GlassDatabase.
class GlassDocument : public Xapian::Document::Internal {
    /// Don't allow assignment.
    void operator=(const GlassDocument &);

    /// Don't allow copying.
    GlassDocument(const GlassDocument &);

    /// Used for lazy access to document values.
    const GlassValueManager *value_manager;

    /// Used for lazy access to document data.
    const GlassDocDataTable *docdata_table;

    /// GlassDatabase::open_document() needs to call our private constructor.
    friend class GlassDatabase;

    /// Private constructor - only called by GlassDatabase::open_document().
    GlassDocument(Xapian::Internal::intrusive_ptr<const Xapian::Database::Internal> db,
		  Xapian::docid did_,
		  const GlassValueManager *value_manager_,
		  const GlassDocDataTable *docdata_table_)
	: Xapian::Document::Internal(db, did_),
	  value_manager(value_manager_), docdata_table(docdata_table_) { }

  public:
    /** Implementation of virtual methods @{ */
    string do_get_value(Xapian::valueno slot) const;
    void do_get_all_values(map<Xapian::valueno, string> & values_) const;
    string do_get_data() const;
    /** @} */
};

#endif // XAPIAN_INCLUDED_GLASS_DOCUMENT_H
