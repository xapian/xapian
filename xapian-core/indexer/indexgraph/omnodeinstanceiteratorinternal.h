/* omnodeinstanceiteratorinternal.h
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

#ifndef OM_HGUARD_OMNODEINSTANCEITERATORINTERNAL_H
#define OM_HGUARD_OMNODEINSTANCEITERATORINTERNAL_H

#include "om/omnodeinstanceiterator.h"
#include "omindexerdescinternal.h"
#include "refcnt.h"
#include "autoptr.h"

class OmNodeInstanceIterator::Internal {
    private:
	friend class OmNodeInstanceIterator;
        friend bool operator==(const OmNodeInstanceIterator &a,
			       const OmNodeInstanceIterator &b);

	RefCntPtr<OmIndexerDesc::Internal::Data> indexerdesc;
	std::vector<OmIndexerDesc::Internal::NodeInstance>::const_iterator it;
	std::vector<OmIndexerDesc::Internal::NodeInstance>::const_iterator end;
    
    public:
        Internal(RefCntPtr<OmIndexerDesc::Internal::Data> indexerdesc_,
		 std::vector<OmIndexerDesc::Internal::NodeInstance>::const_iterator it_,
		 std::vector<OmIndexerDesc::Internal::NodeInstance>::const_iterator end_)
		: indexerdesc(indexerdesc_),
		  it(it_),
		  end(end_)
	{
	}

	Internal(const Internal &other)
		: indexerdesc(other.indexerdesc),
		  it(other.it),
		  end(other.end)
	{ }
};

#endif /* OM_HGUARD_OMNODEINSTANCEITERATORINTERNAL_H */
