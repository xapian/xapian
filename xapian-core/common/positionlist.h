/* positionlist.h
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003,2004 Olly Betts
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

#ifndef OM_HGUARD_POSITIONLIST_H
#define OM_HGUARD_POSITIONLIST_H

#include <xapian/types.h>
#include <xapian/error.h>
#include <xapian/positioniterator.h>

using namespace std;

/** Abstract base class for position lists. */
class Xapian::PositionIterator::Internal : public Xapian::Internal::RefCntBase
{
    private:
	/// Copying is not allowed.
	Internal(const Internal &);

	/// Assignment is not allowed.
	void operator=(const Internal &);
    public:
	/// Default constructor.
	Internal() { }

	/// Destructor.
	virtual ~Internal() { }

	/** Gets size of position list.  This need only be an approximation.
	 *  Typical use is to look for positional match restrictions (e.g.
	 *  NEAR, PHRASE) around the least frequent term.
	 */	
	virtual Xapian::termcount get_size() const = 0;

	/// Gets current position.
	virtual Xapian::termpos get_position() const = 0;

	/** Move to the next item in the list.
	 *  Either next() or skip_to() must be called before get_position()
	 *  - the list initially points to before the beginning of the
	 *  list.
	 */
	virtual void next() = 0;

	/** Move to the next item in the list >= the specified item.
	 *  Either next() or skip_to() must be called before get_position()
	 *  - the list initially points to before the beginning of the
	 *  list.
	 */
	virtual void skip_to(Xapian::termpos termpos) = 0;

	/** True if we're off the end of the list
	 */
	virtual bool at_end() const = 0;

	/** For use by PhrasePostList - ignored by PostingList itself.
	 *  This isn't the most elegant place to put this, but it greatly
	 *  eases the implementation of PhrasePostList which can't subclass
	 *  PositionList (since it gets it from PostList::read_position_list())
	 */
	Xapian::termcount index;
};

typedef Xapian::PositionIterator::Internal PositionList;

#endif /* OM_HGUARD_POSITIONLIST_H */
