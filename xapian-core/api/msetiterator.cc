/** @file msetiterator.cc
 *  @brief Iterator over a Xapian::MSet.
 */
/* Copyright (C) 2017 Olly Betts
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

#include "xapian/mset.h"

#include "msetinternal.h"

#include "debuglog.h"
#include "omassert.h"
#include "str.h"

using namespace std;

namespace Xapian {

Xapian::docid
MSetIterator::operator*() const
{
    LOGCALL(API, Xapian::docid, "MSetIterator::operator*", NO_ARGS);
    Assert(off_from_end != 0);
    AssertRel(off_from_end, <=, mset.internal->items.size());
    RETURN((mset.internal->items.end() - off_from_end)->get_docid());
}

Xapian::Document
MSetIterator::get_document() const
{
    LOGCALL(API, Xapian::Document, "MSetIterator::get_document", NO_ARGS);
    auto size = mset.internal->items.size();
    Assert(off_from_end != 0);
    AssertRel(off_from_end, <=, size);
    RETURN(mset.internal->get_document(size - off_from_end));
}

double
MSetIterator::get_weight() const
{
    LOGCALL(API, double, "MSetIterator::get_weight", NO_ARGS);
    Assert(off_from_end != 0);
    AssertRel(off_from_end, <=, mset.internal->items.size());
    RETURN((mset.internal->items.end() - off_from_end)->get_weight());
}

string
MSetIterator::get_collapse_key() const
{
    LOGCALL(API, string, "MSetIterator::get_collapse_key", NO_ARGS);
    Assert(off_from_end != 0);
    AssertRel(off_from_end, <=, mset.internal->items.size());
    RETURN((mset.internal->items.end() - off_from_end)->get_collapse_key());
}

Xapian::doccount
MSetIterator::get_collapse_count() const
{
    LOGCALL(API, Xapian::doccount, "MSetIterator::get_collapse_count", NO_ARGS);
    Assert(off_from_end != 0);
    AssertRel(off_from_end, <=, mset.internal->items.size());
    RETURN((mset.internal->items.end() - off_from_end)->get_collapse_count());
}

std::string
MSetIterator::get_description() const
{
    string desc = "MSetIterator(";
    if (off_from_end == 0) {
	desc += "end";
    } else {
	desc += str(mset.internal->items.size() - off_from_end);
    }
    desc += ')';
    return desc;
}

}
