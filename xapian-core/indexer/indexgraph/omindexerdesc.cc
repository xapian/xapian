/* omindexerdesc.cc: An intermediate form for the graph description
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

#include <string>
#include <algorithm> /* swap() */
#include "om/omindexerdesc.h"
#include "omindexerdescinternal.h"

OmIndexerDesc::OmIndexerDesc()
	: internal(new Internal)
{
}

OmIndexerDesc::OmIndexerDesc(const OmIndexerDesc &other)
	: internal(new Internal(*other.internal))
{
}

void
OmIndexerDesc::operator=(const OmIndexerDesc &other)
{
    OmIndexerDesc temp(other);
    std::swap(internal, temp.internal);
}

OmIndexerDesc::~OmIndexerDesc()
{
    delete internal;
}

OmIndexerDesc::OmIndexerDesc(Internal *internal_)
	: internal(internal_)
{
}
