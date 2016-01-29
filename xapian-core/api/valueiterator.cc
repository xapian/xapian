/** @file valueiterator.cc
 *  @brief Class for iterating over document values.
 */
/* Copyright (C) 2008,2009,2011,2013 Olly Betts
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

#include "xapian/valueiterator.h"

#include "debuglog.h"
#include "omassert.h"
#include "backends/valuelist.h"

using namespace std;

namespace Xapian {

void
ValueIterator::decref()
{
    Assert(internal);
    if (--internal->_refs == 0)
	delete internal;
}

ValueIterator::ValueIterator(Internal *internal_) : internal(internal_)
{
    LOGCALL_CTOR(API, "ValueIterator", internal_);
    Assert(internal);
    ++internal->_refs;
    try {
	internal->next();
    } catch (...) {
	// The destructor only runs if the constructor completes, so we have to
	// take care of cleaning up for ourselves here.
	decref();
	throw;
    }
    if (internal->at_end()) {
	decref();
	internal = NULL;
    }
}

ValueIterator::ValueIterator(const ValueIterator & o)
    : internal(o.internal)
{
    LOGCALL_CTOR(API, "ValueIterator", o);
    if (internal)
	++internal->_refs;
}

ValueIterator &
ValueIterator::operator=(const ValueIterator & o)
{
    LOGCALL(API, ValueIterator &, "ValueIterator::operator=", o);
    if (o.internal)
	++o.internal->_refs;
    if (internal)
	decref();
    internal = o.internal;
    RETURN(*this);
}

string
ValueIterator::operator*() const
{
    LOGCALL(API, string, "ValueIterator::operator*", NO_ARGS);
    Assert(internal);
    RETURN(internal->get_value());
}

ValueIterator &
ValueIterator::operator++()
{
    LOGCALL(API, ValueIterator &, "ValueIterator::operator++", NO_ARGS);
    Assert(internal);
    internal->next();
    if (internal->at_end()) {
	decref();
	internal = NULL;
    }
    RETURN(*this);
}

Xapian::docid
ValueIterator::get_docid() const
{
    LOGCALL(API, Xapian::docid, "ValueIterator::get_docid", NO_ARGS);
    Assert(internal);
    RETURN(internal->get_docid());
}

Xapian::valueno
ValueIterator::get_valueno() const
{
    LOGCALL(API, Xapian::valueno, "ValueIterator::get_valueno", NO_ARGS);
    Assert(internal);
    RETURN(internal->get_valueno());
}

void
ValueIterator::skip_to(Xapian::docid docid_or_slot)
{
    LOGCALL_VOID(API, "ValueIterator::skip_to", docid_or_slot);
    if (internal) {
	internal->skip_to(docid_or_slot);
	if (internal->at_end()) {
	    decref();
	    internal = NULL;
	}
    }
}

bool
ValueIterator::check(Xapian::docid did)
{
    LOGCALL(API, bool, "ValueIterator::check", did);
    if (internal) {
	if (!internal->check(did)) RETURN(false);
	if (internal->at_end()) {
	    decref();
	    internal = NULL;
	}
    }
    RETURN(true);
}

std::string
ValueIterator::get_description() const
{
    string desc = "ValueIterator(";
    if (internal)
	desc += internal->get_description();
    desc += ')';
    return desc;
}

}
