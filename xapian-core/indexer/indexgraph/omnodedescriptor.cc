/* omnodedescriptor.cc: An object describing a node type.
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

#include <config.h>
#include "om/omnodedescriptor.h"
#include "omnodedescriptorinternal.h"
#include "ompaditeratorinternal.h"
#include "autoptr.h"

OmNodeDescriptor::OmNodeDescriptor(const std::string &nodename_,
				   OmNodeCreator creator_)
	: internal(new Internal())
{
    internal->data->nodename = nodename_;
    internal->data->creator = creator_;
}

OmNodeDescriptor::OmNodeDescriptor(const OmNodeDescriptor &other)
	: internal(new Internal(*other.internal))
{
}

OmNodeDescriptor::OmNodeDescriptor(OmNodeDescriptor::Internal *internal_)
	: internal(internal_)
{
}

std::string
OmNodeDescriptor::get_type() const
{
    return internal->data->nodename;
}

OmPadIterator
OmNodeDescriptor::inputs_begin() const
{
    AutoPtr<OmPadIterator::Internal> padit(new OmPadIterator::Internal(
						internal->data,
						internal->data->inputs.begin(),
						internal->data->inputs.end()));
    OmPadIterator result(padit.get());
    padit.release();

    return result;
}

OmPadIterator
OmNodeDescriptor::inputs_end() const
{
    return OmPadIterator(0);
}

OmPadIterator
OmNodeDescriptor::outputs_begin() const
{
    AutoPtr<OmPadIterator::Internal> padit(new OmPadIterator::Internal(
						internal->data,
						internal->data->outputs.begin(),
						internal->data->outputs.end()));
    OmPadIterator result(padit.get());
    padit.release();

    return result;
}

OmPadIterator
OmNodeDescriptor::outputs_end() const
{
    return OmPadIterator(0);
}

void
OmNodeDescriptor::add_input(const std::string &name,
			    const std::string &type,
			    OmIndexerMessageType phys_type)
{
    internal->data->inputs.push_back(OmNodePad(name, type, phys_type));
}

void
OmNodeDescriptor::add_output(const std::string &name,
			    const std::string &type,
			    OmIndexerMessageType phys_type)
{
    internal->data->outputs.push_back(OmNodePad(name, type, phys_type));
}

OmNodeDescriptor::~OmNodeDescriptor()
{
    delete internal;
}
