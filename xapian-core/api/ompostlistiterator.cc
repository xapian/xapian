/* ompostlistiterator.cc
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

#include "om/ompostlistiterator.h"
#include "ompostlistiteratorinternal.h"
#include "postlist.h"
#include "omassert.h"

OmPostListIterator::~OmPostListIterator() { }

const om_docid
OmPostListIterator::operator *() {
    return internal->postlist->get_docid();
}

OmPostListIterator &
OmPostListIterator::operator++() { 
    PostList *p = internal->postlist->next(0);
    if (p) internal->postlist = p; // handle prune
    return *this;
}

OmPostListIterator
OmPostListIterator::operator++(int) {
    PostList *p = internal->postlist->next(0);
    if (p) internal->postlist = p; // handle prune
    return *this;
}

// extra method, not required to be an input_iterator
OmPostListIterator
OmPostListIterator::skip_to(om_docid did) {
    PostList *p = internal->postlist->skip_to(did, 0);
    if (p) internal->postlist = p; // handle prune
    return *this;
}    

std::string
OmPostListIterator::get_description() const
{
    DEBUGAPICALL("OmPostListIterator::get_description", "");
    /// \todo display contents of the object
    std::string description = "OmPostListIterator()";
    DEBUGAPIRETURN(description);
    return description;
}

bool
operator==(const OmPostListIterator &a, const OmPostListIterator &b)
{
    return (a.internal->postlist->at_end() && b.internal->postlist->at_end());
}

