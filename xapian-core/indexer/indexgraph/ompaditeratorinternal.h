/* ompaditeratorinternal.h
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

#ifndef OM_HGUARD_OMPADITERATORINTERNAL_H
#define OM_HGUARD_OMPADITERATORINTERNAL_H

#include "om/ompaditerator.h"
#include "omnodedescriptorinternal.h"
#include "omnodepad.h"
#include "refcnt.h"
#include "autoptr.h"

class OmPadIterator::Internal {
    private:
	friend class OmPadIterator; // allow access to positionlist
        friend bool operator==(const OmPadIterator &a, const OmPadIterator &b);

	RefCntPtr<OmNodeDescriptor::Internal::Data> nodedesc;
	std::vector<OmNodePad>::const_iterator it;
	std::vector<OmNodePad>::const_iterator end;
    
    public:
        Internal(RefCntPtr<OmNodeDescriptor::Internal::Data> nodedesc_,
		 std::vector<OmNodePad>::const_iterator it_,
		 std::vector<OmNodePad>::const_iterator end_)
		: nodedesc(nodedesc_),
		  it(it_),
		  end(end_)
	{
	}

	Internal(const Internal &other)
		: nodedesc(other.nodedesc),
		  it(other.it),
		  end(other.end)
	{ }
};

#endif /* OM_HGUARD_OMPADITERATORINTERNAL_H */
