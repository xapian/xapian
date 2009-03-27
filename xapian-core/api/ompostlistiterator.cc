/* ompostlistiterator.cc
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003,2004,2005,2008,2009 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>
#include <xapian/postingiterator.h>
#include <xapian/positioniterator.h>
#include "postlist.h"
#include "omassert.h"
#include "omdebug.h"

using namespace std;

Xapian::PostingIterator::PostingIterator(Internal *internal_)
	: internal(internal_)
{
    if (internal.get()) {
	// A PostList starts before the start, iterators start at the start
	Internal *p = internal->next();
	if (p) internal = p; // handle prune
	if (internal->at_end()) internal = 0;
    }
}

Xapian::PostingIterator::PostingIterator() : internal(0) {
    DEBUGAPICALL(void, "Xapian::PostingIterator::PostingIterator", "");
}

Xapian::PostingIterator::~PostingIterator() {
    DEBUGAPICALL(void, "Xapian::PostingIterator::~PostingIterator", "");
}

Xapian::PostingIterator::PostingIterator(const Xapian::PostingIterator &other)
    : internal(other.internal)
{
    DEBUGAPICALL(void, "Xapian::PostingIterator::Xapian::PostingIterator", other);
}

void
Xapian::PostingIterator::operator=(const Xapian::PostingIterator &other)
{
    DEBUGAPICALL(void, "Xapian::PostingIterator::operator=", other);
    internal = other.internal;
}

Xapian::docid
Xapian::PostingIterator::operator *() const
{
    DEBUGAPICALL(Xapian::docid, "Xapian::PostingIterator::operator*", "");
    Assert(internal.get());
    Assert(!internal->at_end());
    RETURN(internal->get_docid());
}

Xapian::PostingIterator &
Xapian::PostingIterator::operator++()
{
    DEBUGAPICALL(void, "Xapian::PostingIterator::operator++", "");
    Assert(internal.get());
    Assert(!internal->at_end());
    Internal *p = internal->next();
    if (p) internal = p; // handle prune
    if (internal->at_end()) internal = 0;
    return *this;
}

// extra method, not required to be an input_iterator
void
Xapian::PostingIterator::skip_to(Xapian::docid did)
{
    DEBUGAPICALL(void, "Xapian::PostingIterator::skip_to", did);
    Assert(internal.get());
    Assert(!internal->at_end());
    PostList *p = internal->skip_to(did, 0);
    if (p) internal = p; // handle prune
    if (internal->at_end()) internal = 0;
}    

Xapian::termcount
Xapian::PostingIterator::get_doclength() const
{
    DEBUGAPICALL(Xapian::termcount, "Xapian::PostingIterator::get_doclength", "");
    Assert(internal.get());
    Assert(!internal->at_end());
    RETURN(internal->get_doclength());
}

Xapian::termcount
Xapian::PostingIterator::get_wdf() const
{
    DEBUGAPICALL(Xapian::termcount, "Xapian::PostingIterator::get_wdf", "");
    Assert(internal.get());
    Assert(!internal->at_end());
    RETURN(internal->get_wdf());
}

Xapian::PositionIterator
Xapian::PostingIterator::positionlist_begin() const
{
    DEBUGAPICALL(Xapian::PositionIterator, "Xapian::PostingIterator::positionlist_begin", "");
    Assert(internal.get());
    Assert(!internal->at_end());
    RETURN(Xapian::PositionIterator(internal->open_position_list()));
}

string
Xapian::PostingIterator::get_description() const
{
    string desc = "Xapian::PostingIterator(pos=";
    if (internal.get() == 0) {
	desc += "END";
    } else {
	desc += internal->get_description();
    }
    desc += ")";
    return desc;
}
