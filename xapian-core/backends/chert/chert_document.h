/* chert_document.h: Document from a Chert Database
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2008 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef OM_HGUARD_CHERT_DOCUMENT_H
#define OM_HGUARD_CHERT_DOCUMENT_H

#include <xapian/base.h>
#include "document.h"

class ChertDatabase;

/// A document from a Chert format database
class ChertDocument : public Xapian::Document::Internal {
    friend class ChertDatabase;
    friend class ChertWritableDatabase;
    private:
	Xapian::Internal::RefCntPtr<const ChertDatabase> database;

	const ChertValueManager *value_manager;
	const ChertRecordTable *record_table;

	ChertDocument(Xapian::Internal::RefCntPtr<const ChertDatabase> database_,
		       const ChertValueManager *value_manager_,
		       const ChertRecordTable *record_table_,
		       Xapian::docid did_, bool lazy);

	// Prevent copying
	ChertDocument(const ChertDocument &);
	ChertDocument & operator = (const ChertDocument &);
    public:
	string do_get_value(Xapian::valueno valueid) const;
	void do_get_all_values(map<Xapian::valueno, string> & values_) const;
	string do_get_data() const;
};

#endif /* OM_HGUARD_CHERT_DOCUMENT_H */
