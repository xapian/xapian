/* irdocument.h : A document
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

#ifndef _irdocument_h_
#define _irdocument_h_

#include "omtypes.h"

// A key in a document
class IRKey {
    public:
	unsigned int value;  // FIXME TEMPORARY
	bool operator < (const IRKey &k) const { return(value < k.value); }
};

// The data in a document
class IRData {
    public:
	string value;
};

// A document in the database - holds keys and records
class IRDocument {
    private:
    public:
	virtual IRKey get_key(om_keyno) const = 0;  // Get key by number (>= 0)
	virtual IRData get_data() const = 0;     // Get data stored in document
};

#endif /* _irdocument_h_ */
