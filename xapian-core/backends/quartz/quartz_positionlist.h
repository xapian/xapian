/* quartz_positionlist.h: Position lists in quartz databases
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 Hein Ragas
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003 Olly Betts
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

#ifndef OM_HGUARD_QUARTZ_POSITIONLIST_H
#define OM_HGUARD_QUARTZ_POSITIONLIST_H

#include <xapian/types.h>
#include "positionlist.h"
#include <xapian/document.h>

#include <string>

using namespace std;

class QuartzTable;
class QuartzBufferedTable;

/** A position list in a quartz database.
 */
class QuartzPositionList : public PositionList {
    private:
        /// The data.
        string data;

        /** Position of iteration through data.
	 */
	const char * pos;

	/** Byte after end of data.
	 */
	const char * end;

	/// Whether we've run off the end of the list yet.
	bool is_at_end;

	/// Whether we've started iterating yet.
	bool have_started;

	/// The current position.
	Xapian::termpos current_pos;
	
	/// The number of entries in the position list.
	Xapian::termcount number_of_entries;

        /// Copying is not allowed.
        QuartzPositionList(const QuartzPositionList &);

        /// Assignment is not allowed.
        void operator=(const QuartzPositionList &);

	/// Advance position by one.
	void next_internal();

	/// Make a key for accessing the positionlist.
	static void make_key(Xapian::docid did,
			     const string & tname,
			     string & key);

    public:
        /// Default constructor.
        QuartzPositionList() : have_started(false) {}

        /// Destructor.
        ~QuartzPositionList() { return; }

        /// Fill list with data, and move the position to the start.
        void read_data(const QuartzTable * table,
		       Xapian::docid did,
		       const string & tname);

        /// Gets size of position list.
        Xapian::termcount get_size() const {
	    DEBUGCALL(DB, Xapian::termcount, "QuartzPositionList::get_size", "");
	    RETURN(number_of_entries);
	}

        /// Gets current position.
        Xapian::termpos get_position() const {
	    Assert(have_started);
	    DEBUGCALL(DB, Xapian::termpos, "QuartzPositionList::get_position", "");
	    RETURN(current_pos);
	}

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
        bool at_end() const {
	    DEBUGCALL(DB, bool, "QuartzPositionList::at_end", "");
	    RETURN(is_at_end);
	}

	/// Set the position list for the given docid and termname
	static void set_positionlist(QuartzBufferedTable * table,
			Xapian::docid did,
			const string & tname,
			Xapian::PositionIterator pos,
			const Xapian::PositionIterator &pos_end);

	/// Delete the position list for the given docid and termname
	static void delete_positionlist(QuartzBufferedTable * table,
					Xapian::docid did,
					const string & tname);
	/// Return the current position
	Xapian::termpos get_current_pos() {
	    return(current_pos);
	}
};

#endif /* OM_HGUARD_QUARTZ_POSITIONLIST_H */
