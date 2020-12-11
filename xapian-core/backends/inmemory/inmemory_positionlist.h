/** @file
 * @brief PositionList from an InMemory DB or a Document object
 */
/* Copyright 2017 Olly Betts
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

#ifndef XAPIAN_INCLUDED_INMEMORY_POSITIONLIST_H
#define XAPIAN_INCLUDED_INMEMORY_POSITIONLIST_H

#include "api/smallvector.h"
#include "backends/positionlist.h"

/// PositionList from an InMemory DB or a Document object.
class InMemoryPositionList : public PositionList {
    /// Sorted list of term positions.
    Xapian::VecCOW<Xapian::termpos> positions;

    /** Current index into @a positions.
     *
     *  Or size_t(-1) if we've not yet started.
     */
    size_t index = size_t(-1);

    /// Don't allow assignment.
    void operator=(const InMemoryPositionList&) = delete;

    /// Don't allow copying.
    InMemoryPositionList(const InMemoryPositionList&) = delete;

  public:
    /// Construct with an empty position list.
    InMemoryPositionList() {}

    /// Move construct with positional data.
    explicit
    InMemoryPositionList(Xapian::VecCOW<Xapian::termpos>&& positions_)
	: positions(std::move(positions_)) {}

    /// Construct with copied positional data.
    explicit
    InMemoryPositionList(const Xapian::VecCOW<Xapian::termpos>& positions_)
	: positions(positions_.copy()) {}

    /// Move assign positional data.
    void assign(Xapian::VecCOW<Xapian::termpos>&& positions_) {
	positions = std::move(positions_);
	index = size_t(-1);
    }

    /// Assign copied positional data.
    void assign(const Xapian::VecCOW<Xapian::termpos>& positions_) {
	positions = positions_.copy();
	index = size_t(-1);
    }

    Xapian::termcount get_approx_size() const;

    Xapian::termpos back() const;

    Xapian::termpos get_position() const;

    bool next();

    bool skip_to(Xapian::termpos termpos);
};

#endif // XAPIAN_INCLUDED_INMEMORY_POSITIONLIST_H
