/* quartz_positionlist.h: Position lists in quartz databases
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

#ifndef OM_HGUARD_QUARTZ_POSITIONLIST_H
#define OM_HGUARD_QUARTZ_POSITIONLIST_H

#include "config.h"
#include "om/omtypes.h"
#include "positionlist.h"
#include <string>
#include "om/omdocument.h"
#include "quartz_table_entries.h"

class QuartzTable;
class QuartzBufferedTable;

/** A position list in a quartz database.
 */
class QuartzPositionList : public PositionList {
    private:
        /// The data.
        std::string data;

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
	om_termpos current_pos;
	
	/// The number of entries in the position list.
	om_termcount number_of_entries;

        /// Copying is not allowed.
        QuartzPositionList(const QuartzPositionList &);

        /// Assignment is not allowed.
        void operator=(const QuartzPositionList &);

	/// Advance position by one.
	void next_internal();

	/// Make a key for accessing the positionlist.
	static void make_key(om_docid did,
			     const om_termname & tname,
			     QuartzDbKey & key);

    public:
        /// Default constructor.
        QuartzPositionList() : have_started(false) {}

        /// Destructor.
        ~QuartzPositionList() { return; }

        /// Fill list with data, and move the position to the start.
        void read_data(const QuartzTable * table,
		       om_docid did,
		       const om_termname & tname);

        /// Gets size of position list.
        om_termcount get_size() const {
	    DEBUGCALL(DB, om_termcount, "QuartzPositionList::get_size", "");
	    RETURN(number_of_entries);
	}

        /// Gets current position.
        om_termpos get_position() const {
	    Assert(have_started);
	    DEBUGCALL(DB, om_termpos, "QuartzPositionList::get_position", "");
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
        void skip_to(om_termpos termpos);

        /// True if we're off the end of the list
        bool at_end() const {
	    DEBUGCALL(DB, bool, "QuartzPositionList::at_end", "");
	    RETURN(is_at_end);
	}

	/// Set the position list for the given docid and termname
	static void set_positionlist(QuartzBufferedTable * table,
			om_docid did,
			const om_termname & tname,
			OmPositionListIterator pos,
			const OmPositionListIterator &pos_end);

	/// Delete the position list for the given docid and termname
	static void delete_positionlist(QuartzBufferedTable * table,
					om_docid did,
					const om_termname & tname);
};

#endif /* OM_HGUARD_QUARTZ_POSITIONLIST_H */
