/* omkeylistiteratorinternal.h
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
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

#ifndef OM_HGUARD_OMKEYLISTITERATORINTERNAL_H
#define OM_HGUARD_OMKEYLISTITERATORINTERNAL_H

#include "om/omkeylistiterator.h"
#include "om/omdocument.h"
#include <map>

class OmKeyListIterator::Internal {
    private:
	friend class OmKeyListIterator; // allow access to it
        friend bool operator==(const OmKeyListIterator &a, const OmKeyListIterator &b);

	std::map<om_keyno, OmKey>::const_iterator it;
    
    public:
        Internal(std::map<om_keyno, OmKey>::const_iterator it_) : it(it_)
	{ }

        Internal(const Internal &other) : it(other.it)
	{ }
};

#endif /* OM_HGUARD_OMKEYLISTITERATOR_H */
