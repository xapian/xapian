/* omnodedescriptorinternal.h: internals of omnodedescriptor
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
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

#ifndef OM_HGUARD_OMNODEDESCRIPTORINTERNAL_H
#define OM_HGUARD_OMNODEDESCRIPTORINTERNAL_H

#include <string>
#include <vector>
#include "om/omnodedescriptor.h"
#include "omnodepad.h"
#include "refcnt.h"

/** A description of a new node type */
class OmNodeDescriptor::Internal {
    public:
	struct Data : public RefCntBase {
	    std::string nodename;
	    std::string type;
	    OmNodeCreator creator;
	    std::vector<OmNodePad> inputs;
	    std::vector<OmNodePad> outputs;
	};

	RefCntPtr<Data> data;

	/* constructors */
	Internal() : data(new Data()) {}
	Internal(const Internal &other) : data(other.data) {}

    private:
	/* assignment not allowed */
	void operator=(const Internal &other);
};

#endif /* OM_HGUARD_OMNODEDESCRIPTORINTERNAL_H */
