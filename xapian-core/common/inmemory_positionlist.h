/* inmemory_positionlist.h
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#ifndef OM_HGUARD_INMEMORY_POSITIONLIST_H
#define OM_HGUARD_INMEMORY_POSITIONLIST_H

#include "om/omtypes.h"
#include "om/omerror.h"

#include <vector>
#include "positionlist.h"

/** A position list in a inemory database. */
class InMemoryPositionList : public PositionList
{
    private:
	/// The list of positions.
	vector<om_termpos> positions;

	/// Position of iteration through positions
	vector<om_termpos>::const_iterator mypos;

	/// True if we have started iterating
	bool iterating_in_progress;

	/// Copying is not allowed.
	InMemoryPositionList(const InMemoryPositionList &);

	/// Assignment is not allowed.
	void operator=(const InMemoryPositionList &);
    public:
	/// Default constructor.
	InMemoryPositionList() : iterating_in_progress(false) {}

	/// Destructor.
	~InMemoryPositionList() { return; }

	/// Fill list with data, and move the position to the start.
	void set_data(const vector<om_termpos> & positions_);
	
	/// Gets size of position list.
	om_termcount get_size() const;

	/// Gets current position.
	om_termpos get_position() const;

	/** Move to the next item in the list.
	 *  Either next() or skip_to() must be called before any other
	 *  methods.
	 */
	void next();

	/** Move to the next item in the list.
	 *  Either next() or skip_to() must be called before any other
	 *  methods.
	 */
	void skip_to(om_termpos termpos);

	/// True if we're off the end of the list
	bool at_end() const;
};

#endif /* OM_HGUARD_INMEMORY_POSITIONLIST_H */
