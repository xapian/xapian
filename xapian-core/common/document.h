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
#include "omlocks.h"
#include <vector>

class OmKey;
class OmData;

/// A document in the database - holds keys and records
class LeafDocument : public OmRefCntBase {
    private:
	/// Copies are not allowed.
	LeafDocument(const LeafDocument &);

	/// Assignment is not allowed.
	void operator=(const LeafDocument &);

	OmLock mutex;
	
	/// The virtual implementation of get_key().
	virtual OmKey do_get_key(om_keyno keyid) const = 0;
	/// The virtual implementation of get_key().
	virtual vector<OmKey> do_get_all_keys() const = 0;
	/// The virtual implementation of get_data().
	virtual OmData do_get_data() const = 0;     
    public:
	/** Get key by key number.
	 *
	 *  Keys are quickly accessible fields, for use during the match
	 *  operation.  Each document may have a set of keys, each of which
	 *  having a different keyid.  Duplicate keys with the same keyid are
	 *  not supported in a single document.
	 *
	 *  Key numbers are any integer >= 0, but particular database types may
	 *  impose a more restrictive range than that.
	 *
	 *  @param keyid  The key number requested.
	 *
	 *  @return       An OmKey object containing the specified key.  If the
	 *  key is not present in this document, the key's value will be a zero
	 *  length string
	 */
	OmKey get_key(om_keyno keyid) const;
	
	/** Get all keys for this document
	 *
	 *  Keys are quickly accessible fields, for use during the match
	 *  operation.  Each document may have a set of keys, each of which
	 *  having a different keyid.  Duplicate keys with the same keyid are
	 *  not supported in a single document.
	 *
	 *  @return       An vector of OmKey objects containing the specified
	 *  keys.  If any key is not present in this document, the key's value
	 *  will be a zero length string.  The vector will be at least big
	 *  enough to hold all the keys for the document.
	 */
	vector<OmKey> get_all_keys() const;

	/** Get data stored in document.
	 *
	 *  This is a general piece of data associated with a document, and
	 *  will typically be used to store such information as text to be
	 *  displayed in the result list, and a pointer in some form
	 *  (eg, URL) to the full text of the document.
	 *
	 *  This operation can be expensive, and shouldn't normally be used
	 *  during the match operation (such as in a match decider functor):
	 *  use a key instead, if at all possible.
	 *
	 *  @return       An OmData object containing the data for this
	 *  document.
	 */
	OmData get_data() const;     

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

#endif  // OM_HGUARD_DOCUMENT_H
