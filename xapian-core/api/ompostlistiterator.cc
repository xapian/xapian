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
#include "om/ompositionlistiterator.h"
#include "ompostlistiteratorinternal.h"
#include "ompositionlistiteratorinternal.h"
#include "postlist.h"
#include "omdebug.h"

OmPostListIterator::~OmPostListIterator() {
    DEBUGAPICALL("OmPostListIterator::~OmPostListIterator", "");
}

const om_docid
OmPostListIterator::operator *() {
    DEBUGAPICALL("OmPostListIterator::operator*", "");
    om_docid result = internal->postlist->get_docid();
    DEBUGAPIRETURN(result);
    return result;
}

OmPostListIterator &
OmPostListIterator::operator++() { 
    DEBUGAPICALL("OmPostListIterator::operator++", "");
    PostList *p = internal->postlist->next();
    if (p) internal->postlist = p; // handle prune
    DEBUGAPIRETURN(*this);
    return *this;
}

OmPostListIterator
OmPostListIterator::operator++(int) {
    DEBUGAPICALL("OmPostListIterator::operator++", "int");
    PostList *p = internal->postlist->next();
    if (p) internal->postlist = p; // handle prune
    DEBUGAPIRETURN(*this);
    return *this;
}

// extra method, not required to be an input_iterator
OmPostListIterator
OmPostListIterator::skip_to(om_docid did) {
    DEBUGAPICALL("OmPostListIterator::skip_to", did);
    PostList *p = internal->postlist->skip_to(did, 0);
    if (p) internal->postlist = p; // handle prune
    DEBUGAPIRETURN(*this);
    return *this;
}    

OmPositionListIterator
OmPostListIterator::positionlist_begin()
{
    DEBUGAPICALL("OmPostListIterator::positionlist_begin", "");
    DEBUGAPIRETURN("OmPositionListIterator");
    return OmPositionListIterator(new OmPositionListIterator::Internal(internal->postlist->get_position_list()));
}

OmPositionListIterator
OmPostListIterator::positionlist_end()
{
    DEBUGAPICALL("OmPostListIterator::positionlist_end", "");
    DEBUGAPIRETURN("OmPositionListIterator");
    return OmPositionListIterator(NULL);
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
    DEBUGAPICALL_STATIC("OmPostListIterator::operator==", a << ", " << b);
    bool result = (a.internal == b.internal ||
		   a.internal->postlist->at_end() &&
		   b.internal->postlist->at_end());
    DEBUGAPIRETURN(result);
    return result;
}

