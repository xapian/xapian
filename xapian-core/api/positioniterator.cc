/** @file positioniterator.cc
 *  @brief Class for iterating over term positions.
 */
/* Copyright (C) 2008,2009,2010 Olly Betts
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
#include "positionlist.h"

using namespace std;

namespace Xapian {

PositionIterator::PositionIterator() : internal(NULL) { }

PositionIterator::~PositionIterator() { }

PositionIterator::PositionIterator(Internal *internal_) : internal(internal_)
{
    Assert(internal_);
    internal->next();
    if (internal->at_end()) internal = NULL;
}

PositionIterator::PositionIterator(const PositionIterator & o)
    : internal(o.internal) { }

PositionIterator &
PositionIterator::operator=(const PositionIterator & o)
{
    internal = o.internal;
    return *this;
}

Xapian::termpos
PositionIterator::operator*() const
{
    LOGCALL(API, Xapian::termpos, "PositionIterator::operator*", NO_ARGS);
    Assert(internal.get());
    RETURN(internal->get_position());
}

PositionIterator &
PositionIterator::operator++()
{
    LOGCALL(API, PositionIterator &, "PositionIterator::operator++", NO_ARGS);
    Assert(internal.get());
    internal->next();
    if (internal->at_end()) internal = NULL;
    RETURN(*this);
}

void
PositionIterator::skip_to(Xapian::termpos termpos)
{
    LOGCALL_VOID(API, "PositionIterator::skip_to", termpos);
    Assert(internal.get());
    internal->skip_to(termpos);
    if (internal->at_end()) internal = NULL;
}

std::string
PositionIterator::get_description() const
{
#if 0 // FIXME: Add PositionIterator::Internal::get_description() method.
    string desc = "PositionIterator(";
    if (internal.get()) desc += internal->get_description();
    desc += ')';
    return desc;
#else
    return "PositionIterator()";
#endif
}

}
