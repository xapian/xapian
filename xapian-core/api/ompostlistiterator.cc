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
#include <xapian/postlistiterator.h>
#include <xapian/positionlistiterator.h>
#include "postlist.h"
#include "omdebug.h"

using namespace std;

Xapian::PostListIterator::PostListIterator(Internal *internal_)
	: internal(internal_)
{
    if (internal.get()) {
	// A PostList starts before the start, iterators start at the start
	Internal *p = internal->next();
	if (p) internal = p; // handle prune
	if (internal->at_end()) internal = 0;
    }
}

Xapian::PostListIterator::~PostListIterator() {
    DEBUGAPICALL(void, "Xapian::PostListIterator::~PostListIterator", "");
}

Xapian::PostListIterator::PostListIterator(const Xapian::PostListIterator &other)
    : internal(other.internal)
{
    DEBUGAPICALL(void, "Xapian::PostListIterator::Xapian::PostListIterator", other);
}

void
Xapian::PostListIterator::operator=(const Xapian::PostListIterator &other)
{
    DEBUGAPICALL(void, "Xapian::PostListIterator::operator=", other);
    internal = other.internal;
}

Xapian::docid
Xapian::PostListIterator::operator *() const
{
    DEBUGAPICALL(Xapian::docid, "Xapian::PostListIterator::operator*", "");
    Assert(internal.get());
    Assert(!internal->at_end());
    RETURN(internal->get_docid());
}

Xapian::PostListIterator &
Xapian::PostListIterator::operator++()
{
    DEBUGAPICALL(Xapian::PostListIterator &, "Xapian::PostListIterator::operator++", "");
    Assert(internal.get());
    Assert(!internal->at_end());
    Internal *p = internal->next();
    if (p) internal = p; // handle prune
    if (internal->at_end()) internal = 0;
    RETURN(*this);
}

void
Xapian::PostListIterator::operator++(int)
{
    DEBUGAPICALL(void, "Xapian::PostListIterator::operator++", "int");
    Assert(internal.get());
    Assert(!internal->at_end());
    Internal *p = internal->next();
    if (p) internal = p; // handle prune
    if (internal->at_end()) internal = 0;
}

// extra method, not required to be an input_iterator
void
Xapian::PostListIterator::skip_to(Xapian::docid did)
{
    DEBUGAPICALL(void, "Xapian::PostListIterator::skip_to", did);
    Assert(internal.get());
    Assert(!internal->at_end());
    PostList *p = internal->skip_to(did, 0);
    if (p) internal = p; // handle prune
    if (internal->at_end()) internal = 0;
}    

// need to set Xapian::Weight object for this to work
//Xapian::weight
//Xapian::PostListIterator::get_weight() const
//{
//    DEBUGAPICALL(Xapian::weight, "Xapian::PostListIterator::get_weight", "");
//    RETURN(internal->get_weight());
//}
    
Xapian::doclength
Xapian::PostListIterator::get_doclength() const
{
    DEBUGAPICALL(Xapian::doclength, "Xapian::PostListIterator::get_doclength", "");
    Assert(internal.get());
    Assert(!internal->at_end());
    RETURN(internal->get_doclength());
}

Xapian::termcount
Xapian::PostListIterator::get_wdf() const
{
    DEBUGAPICALL(Xapian::termcount, "Xapian::PostListIterator::get_wdf", "");
    Assert(internal.get());
    Assert(!internal->at_end());
    RETURN(internal->get_wdf());
}

Xapian::PositionListIterator
Xapian::PostListIterator::positionlist_begin()
{
    DEBUGAPICALL(Xapian::PositionListIterator, "Xapian::PostListIterator::positionlist_begin", "");
    Assert(internal.get());
    Assert(!internal->at_end());
    RETURN(Xapian::PositionListIterator(internal->open_position_list()));
}

Xapian::PositionListIterator
Xapian::PostListIterator::positionlist_end()
{
    DEBUGAPICALL(Xapian::PositionListIterator, "Xapian::PostListIterator::positionlist_end", "");
    Assert(internal.get());
    Assert(!internal->at_end());
    RETURN(Xapian::PositionListIterator(NULL));
}

string
Xapian::PostListIterator::get_description() const
{
    DEBUGCALL(INTRO, string, "Xapian::PostListIterator::get_description", "");
    /// \todo display contents of the object
    string desc = "Xapian::PostListIterator([pos=";
    if (internal.get() == 0) {
	desc += "END";
    } else {
	desc += internal->get_description();
    }
    desc += "])";
    RETURN(desc);
}
