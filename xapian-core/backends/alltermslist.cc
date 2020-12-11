/** @file
 * @brief Abstract base class for iterating all terms in a database.
 */
/* Copyright (C) 2007,2008 Olly Betts
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

#include "alltermslist.h"

#include <xapian/error.h>

#include "omassert.h"

using namespace std;

Xapian::termcount
AllTermsList::get_approx_size() const
{
    // We should never use get_approx_size() on AllTermsList subclasses.
    Assert(false);
    return 0;
}

Xapian::termcount
AllTermsList::get_wdf() const
{
    throw Xapian::InvalidOperationError("AllTermsList::get_wdf() isn't meaningful");
}

Xapian::termcount
AllTermsList::positionlist_count() const
{
    throw Xapian::InvalidOperationError("AllTermsList::positionlist_count() isn't meaningful");
}

Xapian::PositionIterator
AllTermsList::positionlist_begin() const
{
    throw Xapian::InvalidOperationError("AllTermsList::positionlist_begin() isn't meaningful");
}
