/* multialltermslist.cc
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

#include "multialltermslist.h"

MultiAllTermsList::MultiAllTermsList(const std::vector<RefCntPtr<AllTermsList> > &lists_)
	: lists(lists_)
{
}

MultiAllTermsList::~MultiAllTermsList()
{
}

const om_termname
MultiAllTermsList::get_termname() const
{
    throw OmUnimplementedError("not implemented");
}

om_doccount
MultiAllTermsList::get_termfreq() const
{
    throw OmUnimplementedError("not implemented");
}

om_termcount
MultiAllTermsList::get_collection_freq() const
{
    throw OmUnimplementedError("not implemented");
}

bool
MultiAllTermsList::skip_to(const om_termname &tname)
{
    throw OmUnimplementedError("not implemented");
}

bool
MultiAllTermsList::next()
{
    throw OmUnimplementedError("not implemented");
}

bool
MultiAllTermsList::at_end() const
{
    throw OmUnimplementedError("not implemented");
}
