/* omtermlistiterator.cc
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
#include <xapian/termiterator.h>
#include "termlist.h"
#include "positionlist.h"
#include "omdebug.h"

using namespace std;

Xapian::TermIterator::TermIterator(Internal *internal_)
	: internal(internal_)
{
    if (internal.get()) {
	// A TermList starts before the start, iterators start at the start
	internal->next();
	if (internal->at_end()) internal = 0;
    }
}

Xapian::TermIterator::TermIterator() : internal(0)
{
    DEBUGAPICALL(void, "Xapian::TermIterator::TermIterator", "");
}

Xapian::TermIterator::~TermIterator() {
    DEBUGAPICALL(void, "Xapian::TermIterator::~TermIterator", "");
}

Xapian::TermIterator::TermIterator(const Xapian::TermIterator &other)
    : internal(other.internal)
{
    DEBUGAPICALL(void, "Xapian::TermIterator::TermIterator", other);
}

void
Xapian::TermIterator::operator=(const Xapian::TermIterator &other)
{
    DEBUGAPICALL(void, "Xapian::TermIterator::operator=", other);
    internal = other.internal;
}

string
Xapian::TermIterator::operator *() const
{
    DEBUGAPICALL(string, "Xapian::TermIterator::operator*", "");
    Assert(internal.get());
    Assert(!internal->at_end());
    RETURN(internal->get_termname());
}

Xapian::termcount
Xapian::TermIterator::get_wdf() const
{
    DEBUGAPICALL(Xapian::termcount, "Xapian::TermIterator::get_wdf", "");
    Assert(internal.get());
    Assert(!internal->at_end());
    RETURN(internal->get_wdf());
}

Xapian::doccount
Xapian::TermIterator::get_termfreq() const
{
    DEBUGAPICALL(Xapian::doccount, "Xapian::TermIterator::get_termfreq", "");
    Assert(internal.get());
    Assert(!internal->at_end());
    RETURN(internal->get_termfreq());
}

Xapian::TermIterator &
Xapian::TermIterator::operator++()
{
    DEBUGAPICALL(Xapian::TermIterator &, "Xapian::TermIterator::operator++", "");
    Assert(internal.get());
    Assert(!internal->at_end());
    internal->next();
    if (internal->at_end()) internal = 0;
    RETURN(*this);
}

void
Xapian::TermIterator::operator++(int)
{
    DEBUGAPICALL(void, "Xapian::TermIterator::operator++(int)", "");
    Assert(internal.get());
    Assert(!internal->at_end());
    internal->next();
    if (internal->at_end()) internal = 0;
}

// extra method, not required to be an input_iterator
void
Xapian::TermIterator::skip_to(const string & tname)
{
    DEBUGAPICALL(void, "Xapian::TermIterator::skip_to", tname);
    if (internal.get()) {
	Assert(!internal->at_end());
	internal->skip_to(tname);
	if (internal->at_end()) internal = 0;
    }
}

Xapian::PositionIterator
Xapian::TermIterator::positionlist_begin()
{
    DEBUGAPICALL(Xapian::PositionIterator, "Xapian::TermIterator::positionlist_begin", "");
    Assert(internal.get());
    Assert(!internal->at_end());
    RETURN(internal->positionlist_begin());
}

Xapian::PositionIterator
Xapian::TermIterator::positionlist_end()
{
    DEBUGAPICALL(Xapian::PositionIterator, "Xapian::TermIterator::positionlist_end", "");
    Assert(internal.get());
    Assert(!internal->at_end());
    RETURN(Xapian::PositionIterator(NULL));
}

std::string
Xapian::TermIterator::get_description() const
{
    DEBUGCALL(INTRO, std::string, "Xapian::TermIterator::get_description", "");
    /// \todo display contents of the object
    RETURN("Xapian::TermIterator()");
}
