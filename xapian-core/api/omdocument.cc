/* omdocument.cc: class for performing a match
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

#include <om/omdocument.h>
#include "omdocumentinternal.h"

// implementation of OmDocument
OmDocument::OmDocument(const Internal *internal_)
	: internal(new Internal(*internal_)) {}

OmKey
OmDocument::get_key(om_keyno key) const
{
    OmLockSentry locksentry(internal->mutex);
    return internal->ptr->get_key(key);
}

OmData
OmDocument::get_data() const
{
    OmLockSentry locksentry(internal->mutex);
    return internal->ptr->get_data();
}

void
OmDocument::operator=(const OmDocument &other)
{
    OmLockSentry locksentry(internal->mutex);
    // pointers are reference counted.
    internal->ptr = other.internal->ptr;
}

OmDocument::OmDocument(const OmDocument &other)
	: internal(0)
{
    internal = new Internal(*other.internal);
}

OmDocument::~OmDocument()
{
    delete internal;
}
