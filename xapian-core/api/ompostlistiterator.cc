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
    DEBUGAPICALL(void, "OmPostListIterator::~OmPostListIterator", "");
    delete internal;
}

OmPostListIterator::OmPostListIterator(const OmPostListIterator &other)
    : internal(NULL)
{
    DEBUGAPICALL(void, "OmPostListIterator::OmPostListIterator", other);
    if (other.internal) internal = new Internal(*(other.internal));
}

void
OmPostListIterator::operator=(const OmPostListIterator &other)
{
    DEBUGAPICALL(void, "OmPostListIterator::operator=", other);
    if (this == &other) {
	DEBUGLINE(API, "OmPostListIterator assigned to itself");
	return;
    }

    Internal * newinternal = NULL;
    if (other.internal)
	newinternal = new Internal(*(other.internal));
    std::swap(internal, newinternal);
    delete newinternal;
}

const om_docid
OmPostListIterator::operator *() {
    DEBUGAPICALL(om_docid, "OmPostListIterator::operator*", "");
    om_docid result = internal->postlist->get_docid();
    RETURN(result);
}

OmPostListIterator &
OmPostListIterator::operator++() { 
    DEBUGAPICALL(OmPostListIterator &, "OmPostListIterator::operator++", "");
    PostList *p = internal->postlist->next();
    if (p) internal->postlist = p; // handle prune
    RETURN(*this);
}

void
OmPostListIterator::operator++(int) {
    DEBUGAPICALL(void, "OmPostListIterator::operator++", "int");
    PostList *p = internal->postlist->next();
    if (p) internal->postlist = p; // handle prune
}

// extra method, not required to be an input_iterator
void
OmPostListIterator::skip_to(om_docid did) {
    DEBUGAPICALL(void, "OmPostListIterator::skip_to", did);
    PostList *p = internal->postlist->skip_to(did, 0);
    if (p) internal->postlist = p; // handle prune
}    

OmPositionListIterator
OmPostListIterator::positionlist_begin()
{
    DEBUGAPICALL(OmPositionListIterator, "OmPostListIterator::positionlist_begin", "");
    RETURN(OmPositionListIterator(new OmPositionListIterator::Internal(internal->postlist->get_position_list())));
}

OmPositionListIterator
OmPostListIterator::positionlist_end()
{
    DEBUGAPICALL(OmPositionListIterator, "OmPostListIterator::positionlist_end", "");
    RETURN(OmPositionListIterator(NULL));
}

std::string
OmPostListIterator::get_description() const
{
    DEBUGAPICALL(std::string, "OmPostListIterator::get_description", "");
    /// \todo display contents of the object
    om_ostringstream desc;
    desc << "OmPostListIterator([pos=";
    if ((internal == 0) || internal->postlist->at_end()) {
	desc << "END";
    } else {
	desc << internal->postlist->get_docid();
    }
    desc << "])";
    RETURN(desc.str());
}

bool
operator==(const OmPostListIterator &a, const OmPostListIterator &b)
{
    if (a.internal == b.internal) return true;
    if (a.internal) {
	if (b.internal) return a.internal->postlist.get() == b.internal->postlist.get();
	return a.internal->postlist->at_end();
    }
    Assert(b.internal); // a.internal is NULL, so b.internal can't be
    return b.internal->postlist->at_end();
}
