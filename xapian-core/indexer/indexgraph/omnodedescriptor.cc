/* omnodedescriptor.cc: An object describing a node type.
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

#include "om/omnodedescriptor.h"
#include "omnodedescriptorinternal.h"

OmNodeDescriptor::OmNodeDescriptor(const std::string &nodename_,
				   OmNodeCreator creator_)
	: internal(new Internal())
{
    internal->nodename = nodename_;
    internal->creator = creator_;
}

void
OmNodeDescriptor::add_input(const std::string &name,
			    const std::string &type,
			    OmIndexerMessageType phys_type)
{
    internal->inputs.push_back(OmNodeConnection(name, type, phys_type));
}

void
OmNodeDescriptor::add_output(const std::string &name,
			    const std::string &type,
			    OmIndexerMessageType phys_type)
{
    internal->outputs.push_back(OmNodeConnection(name, type, phys_type));
}

OmNodeDescriptor::~OmNodeDescriptor()
{
    delete internal;
}
