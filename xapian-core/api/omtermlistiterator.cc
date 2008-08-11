/* omtermlistiterator.cc
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003,2004,2005,2006,2007,2008 Olly Betts
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
#include <xapian/termiterator.h>
#include "termlist.h"
#include "positionlist.h"
#include "omassert.h"
#include "omdebug.h"

using namespace std;

// Helper function.
inline void
handle_prune(Xapian::Internal::RefCntPtr<TermList>& old, TermList * result)
{
    if (result) {
	old = result;
    }
}

Xapian::TermIterator::TermIterator(Internal *internal_)
	: internal(internal_)
{
    if (internal.get()) {
	// A TermList starts before the start, iterators start at the start
	handle_prune(internal, internal->next());
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
    DEBUGAPICALL(void, "Xapian::TermIterator::operator++", "");
    Assert(internal.get());
    Assert(!internal->at_end());
    handle_prune(internal, internal->next());
    if (internal->at_end()) internal = 0;
    return *this;
}

// extra method, not required to be an input_iterator
void
Xapian::TermIterator::skip_to(const string & tname)
{
    DEBUGAPICALL(void, "Xapian::TermIterator::skip_to", tname);
    if (internal.get()) {
	Assert(!internal->at_end());
	handle_prune(internal, internal->skip_to(tname));
	if (internal->at_end()) internal = 0;
    }
}

Xapian::termcount
Xapian::TermIterator::positionlist_count() const
{
    DEBUGAPICALL(Xapian::termcount, "Xapian::TermIterator::positionlist_count", "");
    Assert(internal.get());
    Assert(!internal->at_end());
    RETURN(internal->positionlist_count());
}

Xapian::PositionIterator
Xapian::TermIterator::positionlist_begin() const
{
    DEBUGAPICALL(Xapian::PositionIterator, "Xapian::TermIterator::positionlist_begin", "");
    Assert(internal.get());
    Assert(!internal->at_end());
    RETURN(internal->positionlist_begin());
}

std::string
Xapian::TermIterator::get_description() const
{
    /// \todo display contents of the object
    return "Xapian::TermIterator()";
}
