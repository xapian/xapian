/* document.h: class with document data
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

#ifndef OM_HGUARD_DOCUMENT_H
#define OM_HGUARD_DOCUMENT_H

#include "om/omtypes.h"
#include "omrefcnt.h"

class OmKey;
class OmData;

/// A document in the database - holds keys and records
class LeafDocument : public OmRefCntBase {
    private:
	/// Copies are not allowed.
	LeafDocument(const LeafDocument &);

	/// Assignment is not allowed.
	void operator=(const LeafDocument &);
    public:
	/** Get key by number (>= 0).
	 *  Keys are designed to be quickly accessible, for use during the
	 *  match operation.
	 */
	virtual OmKey get_key(om_keyno) const = 0;
	
	/** Get data stored in document.
	 *  This can be expensive, and shouldn't normally be used
	 *  during the match operation (such as in a match decider functor):
	 *  use a key instead, if at all possible.
	 */
	virtual OmData get_data() const = 0;     

	/** Constructor.  In derived classes, this will typically be a
	 *  private method, and only be called by database objects of the
	 *  corresponding type.
	 */
	LeafDocument() {};

	/** Destructor.  Note that the database object which created this
	 *  document must still exist at the time this is called.
	 */
	virtual ~LeafDocument() {}
};

#endif  // OM_HGUARD_LeafDocument_H
