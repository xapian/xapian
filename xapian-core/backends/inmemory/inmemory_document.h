/* inmemory_document.h: C++ class definition for accessing a inmemory document
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#ifndef OM_HGUARD_INMEMORY_DOCUMENT_H
#define OM_HGUARD_INMEMORY_DOCUMENT_H

#include "document.h"
#include <string>

class InMemoryDocument : public LeafDocument {
    friend class InMemoryDatabase;
    private:
	string doc;

	InMemoryDocument(const string & doc_);

	// Stop copying
	InMemoryDocument(const InMemoryDocument &);
	InMemoryDocument & operator = (const InMemoryDocument &);
    public:
	OmKey do_get_key(om_keyno keyid) const;
	vector<OmKey> do_get_all_keys() const;
	OmData do_get_data() const;
};

#endif /* OM_HGUARD_INMEMORY_DOCUMENT_H */
