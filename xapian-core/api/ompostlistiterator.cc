/* ompostlistiterator.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003 Olly Betts
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
#include "om/ompostlistiterator.h"
#include "xapian/positionlistiterator.h"
#include "ompostlistiteratorinternal.h"
#include "postlist.h"
#include "omdebug.h"

using namespace std;

OmPostListIterator::OmPostListIterator()
	: internal(0)
{
}

OmPostListIterator::OmPostListIterator(Internal *internal_)
	: internal(internal_)
{
    if (internal && internal->postlist->at_end()) {
	delete internal;
	internal = 0;
    }
}

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
    swap(internal, newinternal);
    delete newinternal;
}

om_docid
OmPostListIterator::operator *() const
{
    DEBUGAPICALL(om_docid, "OmPostListIterator::operator*", "");
    Assert(internal);
    Assert(!internal->postlist->at_end());
    om_docid result = internal->postlist->get_docid();
    RETURN(result);
}

OmPostListIterator &
OmPostListIterator::operator++()
{
    DEBUGAPICALL(OmPostListIterator &, "OmPostListIterator::operator++", "");
    Assert(internal);
    Assert(!internal->postlist->at_end());
    PostList *p = internal->postlist->next();
    if (p) internal->postlist = p; // handle prune
    if (internal->postlist->at_end()) {
	delete internal;
	internal = 0;
    }
    RETURN(*this);
}

void
OmPostListIterator::operator++(int)
{
    DEBUGAPICALL(void, "OmPostListIterator::operator++", "int");
    Assert(internal);
    Assert(!internal->postlist->at_end());
    PostList *p = internal->postlist->next();
    if (p) internal->postlist = p; // handle prune
    if (internal->postlist->at_end()) {
	delete internal;
	internal = 0;
    }
}

// extra method, not required to be an input_iterator
void
OmPostListIterator::skip_to(om_docid did)
{
    DEBUGAPICALL(void, "OmPostListIterator::skip_to", did);
    Assert(internal);
    Assert(!internal->postlist->at_end());
    PostList *p = internal->postlist->skip_to(did, 0);
    if (p) internal->postlist = p; // handle prune
    if (internal->postlist->at_end()) {
	delete internal;
	internal = 0;
    }
}    

// need to set OmWeight object for this to work
//om_weight
//OmPostListIterator::get_weight() const
//{
//    DEBUGAPICALL(om_weight, "OmPostListIterator::get_weight", "");
//    RETURN(internal->postlist->get_weight());
//}
    
om_doclength
OmPostListIterator::get_doclength() const
{
    DEBUGAPICALL(om_doclength, "OmPostListIterator::get_doclength", "");
    Assert(internal);
    Assert(!internal->postlist->at_end());
    RETURN(internal->postlist->get_doclength());
}

om_termcount
OmPostListIterator::get_wdf() const
{
    DEBUGAPICALL(om_termcount, "OmPostListIterator::get_wdf", "");
    Assert(internal);
    Assert(!internal->postlist->at_end());
    RETURN(internal->postlist->get_wdf());
}

Xapian::PositionListIterator
OmPostListIterator::positionlist_begin()
{
    DEBUGAPICALL(Xapian::PositionListIterator, "OmPostListIterator::positionlist_begin", "");
    Assert(internal);
    Assert(!internal->postlist->at_end());
    RETURN(Xapian::PositionListIterator(internal->postlist->open_position_list()));
}

Xapian::PositionListIterator
OmPostListIterator::positionlist_end()
{
    DEBUGAPICALL(Xapian::PositionListIterator, "OmPostListIterator::positionlist_end", "");
    Assert(internal);
    Assert(!internal->postlist->at_end());
    RETURN(Xapian::PositionListIterator(NULL));
}

string
OmPostListIterator::get_description() const
{
    DEBUGCALL(INTRO, string, "OmPostListIterator::get_description", "");
    /// \todo display contents of the object
    string desc = "OmPostListIterator([pos=";
    if (internal == 0) {
	desc += "END";
    } else {
	desc += internal->postlist->get_description();
    }
    desc += "])";
    RETURN(desc);
}

bool
operator==(const OmPostListIterator &a, const OmPostListIterator &b)
{
    if (a.internal == b.internal) return true;
    if (a.internal == 0 || b.internal == 0) return false;
    return (a.internal->postlist.get() == b.internal->postlist.get());
}
