/** @file positioniterator.cc
 *  @brief Class for iterating over term positions.
 */
/* Copyright (C) 2008,2009,2010,2011,2013 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include "xapian/positioniterator.h"

#include "debuglog.h"
#include "omassert.h"
#include "backends/positionlist.h"

using namespace std;

namespace Xapian {

void
PositionIterator::decref()
{
    Assert(internal);
    if (--internal->_refs == 0)
	delete internal;
}

PositionIterator::PositionIterator(Internal *internal_) : internal(internal_)
{
    LOGCALL_CTOR(API, "PositionIterator", internal_);
    Assert(internal);
    ++internal->_refs;
    try {
	if (!internal->next()) {
	    decref();
	    internal = NULL;
	}
    } catch (...) {
	// The destructor only runs if the constructor completes, so we have to
	// take care of cleaning up for ourselves here.
	decref();
	throw;
    }
}

PositionIterator::PositionIterator(const PositionIterator & o)
    : internal(o.internal)
{
    LOGCALL_CTOR(API, "PositionIterator", o);
    if (internal)
	++internal->_refs;
}

PositionIterator &
PositionIterator::operator=(const PositionIterator & o)
{
    LOGCALL(API, PositionIterator &, "PositionIterator::operator=", o);
    if (o.internal)
	++o.internal->_refs;
    if (internal)
	decref();
    internal = o.internal;
    RETURN(*this);
}

Xapian::termpos
PositionIterator::operator*() const
{
    LOGCALL(API, Xapian::termpos, "PositionIterator::operator*", NO_ARGS);
    Assert(internal);
    RETURN(internal->get_position());
}

PositionIterator &
PositionIterator::operator++()
{
    LOGCALL(API, PositionIterator &, "PositionIterator::operator++", NO_ARGS);
    Assert(internal);
    if (!internal->next()) {
	decref();
	internal = NULL;
    }
    RETURN(*this);
}

void
PositionIterator::skip_to(Xapian::termpos pos)
{
    LOGCALL_VOID(API, "PositionIterator::skip_to", pos);
    if (internal) {
	if (!internal->skip_to(pos)) {
	    decref();
	    internal = NULL;
	}
    }
}

std::string
PositionIterator::get_description() const
{
#if 0 // FIXME: Add PositionIterator::Internal::get_description() method.
    string desc = "PositionIterator(";
    if (internal)
	desc += internal->get_description();
    desc += ')';
    return desc;
#else
    return "PositionIterator()";
#endif
}

}
