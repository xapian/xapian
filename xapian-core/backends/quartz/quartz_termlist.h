/* quartz_termlist.h: Termlists in quartz databases
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

#ifndef OM_HGUARD_QUARTZ_TERMLIST_H
#define OM_HGUARD_QUARTZ_TERMLIST_H

#include "config.h"
#include "quartz_table.h"
#include "om/omtypes.h"
#include "termlist.h"
#include "quartz_database.h"

/** A record in a quartz database.
 */
class QuartzTermList : public LeafTermList {
    friend class QuartzDatabase;
    private:

	/** The database we are searching.  This pointer is held so that the
	 *  database doesn't get deleted before us.
	 */
	RefCntPtr<const QuartzDatabase> this_db;

	/** The table holding the termlist.
	 */
	const QuartzTable * table;

	/** Open a termlist for the specified document.
	 */
	QuartzTermList(RefCntPtr<const QuartzDatabase> this_db_,
		       const QuartzTable & table_,
		       om_docid did);
    public:
	/** Return number of items in termlist
	 */
	om_termcount get_approx_size() const;

	//@{
	OmExpandBits get_weighting() const;
	const om_termname get_termname() const;
	om_termcount get_wdf() const;
	om_doccount get_termfreq() const;
	//@}

	TermList * next();
	bool   at_end() const;
};

#endif /* OM_HGUARD_QUARTZ_TERMLIST_H */
