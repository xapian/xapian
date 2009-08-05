/* inmemory_positionlist.h
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003,2008,2009 Olly Betts
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

#ifndef OM_HGUARD_INMEMORY_POSITIONLIST_H
#define OM_HGUARD_INMEMORY_POSITIONLIST_H

#include <xapian/types.h>
#include <xapian/error.h>

#include <vector>
#include "positionlist.h"
#include "documentterm.h"

using namespace std;

/** A position list in a inmemory database. */
class InMemoryPositionList : public PositionList
{
    private:
	/// The list of positions.
	vector<Xapian::termpos> positions;

	/// Position of iteration through positions
	vector<Xapian::termpos>::const_iterator mypos;

	/// True if we have started iterating
	bool iterating_in_progress;

	/// Copying is not allowed.
	InMemoryPositionList(const InMemoryPositionList &);

	/// Assignment is not allowed.
	void operator=(const InMemoryPositionList &);

    public:
	/// Default constructor.
	InMemoryPositionList() : iterating_in_progress(false) { }

	/// Construct an empty InMemoryPositionList.
	InMemoryPositionList(bool)
	    : mypos(positions.begin()), iterating_in_progress(false) { }

	/// Construct, fill list with data, and move the position to the start.
	InMemoryPositionList(const OmDocumentTerm::term_positions & positions_);

	/// Fill list with data, and move the position to the start.
	void set_data(const OmDocumentTerm::term_positions & positions_);

	/// Gets size of position list.
	Xapian::termcount get_size() const;

	/// Gets current position.
	Xapian::termpos get_position() const;

	/** Move to the next item in the list.
	 *  Either next() or skip_to() must be called before any other
	 *  methods.
	 */
	void next();

	/** Move to the next item in the list.
	 *  Either next() or skip_to() must be called before any other
	 *  methods.
	 */
	void skip_to(Xapian::termpos termpos);

	/// True if we're off the end of the list
	bool at_end() const;
};

#endif /* OM_HGUARD_INMEMORY_POSITIONLIST_H */
