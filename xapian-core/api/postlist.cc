/** @file postlist.cc
 * @brief Abstract base class for postlists.
 */
/* Copyright (C) 2007 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include <xapian/error.h>

#include "postlist.h"

using namespace std;

namespace Xapian {

PostingIterator::Internal::~Internal() { }

Xapian::termcount
PostingIterator::Internal::get_wdf() const
{
    throw Xapian::UnimplementedError("PostingIterator::Internal::get_wdf() unimplemented");
}

const string *
PostingIterator::Internal::get_collapse_key() const
{
    return NULL;
}

}
