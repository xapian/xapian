/* omdocumentparams.h: parameters for creating an OmDocument
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

#ifndef OM_HGUARD_OMDOCUMENTPARAMS_H
#define OM_HGUARD_OMDOCUMENTPARAMS_H

#include "document.h"
#include "refcnt.h"
/// Parameters used to create an OmDocument
class OmDocumentParams {
    public:
	/** Create an OmDocumentParams from a leaf document pointer.
	 *
	 *  @param ld A pointer to a LeafDocument.  The object pointed
	 *            to is claimed by the OmDocumentParams, so the
	 *            caller should not delete it: it will be deleted
	 *            when no further references to it exist.
	 */
	OmDocumentParams(LeafDocument *ld)
		: ld_ptr(ld) {}

	OmDocumentParams(RefCntPtr<LeafDocument> ld_ptr_)
		: ld_ptr(ld_ptr_) {}

	/// The reference counted pointer to the LeafDocument.
	RefCntPtr<LeafDocument> ld_ptr;
};

#endif  // OM_HGUARD_OMDOCUMENTPARAMS_H
