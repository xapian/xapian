/* omdocumentinternal.h: internal class representing a document
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#ifndef OM_HGUARD_OMDOCUMENTINTERNAL_H
#define OM_HGUARD_OMDOCUMENTINTERNAL_H

/* omdocument.cc: class for performing a match
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#include <om/omdocument.h>
#include "om/omtypes.h"
#include "refcnt.h"
#include "document.h"

// A document in the database - holds keys and records
class OmDocument::Internal {
    public:
	/// The reference counted pointer to a LeafDocument instance
	RefCntPtr<LeafDocument> ptr;

	explicit Internal(LeafDocument *ld) : ptr(ld) {}

	explicit Internal(RefCntPtr<LeafDocument> ptr_) : ptr(ptr_) {}

	Internal(const Internal &other)
		: ptr(other.ptr) {}
};

#endif  // OM_HGUARD_OMDOCUMENTINTERNAL_H
