/** @file termlist.cc
 * @brief Abstract base class for termlists.
 */
/* Copyright (C) 2007,2010,2013 Olly Betts
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

#include "termlist.h"

#include "omassert.h"

using namespace std;

namespace Xapian {

TermIterator::Internal::~Internal() { }

void
TermIterator::Internal::accumulate_stats(Xapian::Internal::ExpandStats &) const
{
    // accumulate_stats should never get called for some subclasses.
    Assert(false);
}

// Default implementation for when the positions aren't in vector<termpos>.
const std::vector<Xapian::termpos> *
TermIterator::Internal::get_vector_termpos() const
{
    return NULL;
}

}
