/* quartz_db_diffs.h: Diffs made to a given quartz database
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

#ifndef OM_HGUARD_QUARTZ_DB_DIFFS_H
#define OM_HGUARD_QUARTZ_DB_DIFFS_H

#include "config.h"
#include "quartz_db_manager.h"
#include "quartz_db_blocks.h"
#include "om/omtypes.h"
#include "om/omindexdoc.h"

/** Base class managing a set of diffs to a Quartz database.
 */
class QuartzDbDiffs {
    private:
	/// Copying not allowed
	QuartzDbDiffs(const QuartzDbDiffs &);

	/// Assignment not allowed
	void operator=(const QuartzDbDiffs &);

    protected:
	/** Blocks which have been changed.
	 */
	QuartzDbBlocks diffs;

    public:
	/** Construct the diffs object.
	 */
	QuartzDbDiffs() {}

	/** Destroy the diffs.  Any unapplied diffs will be lost.
	 */
	virtual ~QuartzDbDiffs() {};

	/** Apply the diffs.  Throws an exception if an error
	 *  occurs.
	 */
	virtual void apply() = 0;
};

/** Class managing a set of diffs to a Quartz PostList database.
 */
class QuartzPostListDbDiffs : public QuartzDbDiffs {
    private:
	/** Pointer to the database manager.
	 */
	QuartzDbManager * db_manager;

    public:
	/** Construct the diffs object.
	 *
	 *  @param db_manager_  The object managing access to databases on
	 *                      disk.
	 */
	QuartzPostListDbDiffs(QuartzDbManager * db_manager_)
		: db_manager(db_manager_) {}

	~QuartzPostListDbDiffs() {}

	/** Add a posting to the diffs.
	 *
	 *  @param tname  The name of the term whose posting list an entry
	 *                should be added to.
	 *  @param did    The document ID to add to the posting list.
	 *  @param wdf    The within document frequency to store in the
	 *                posting list.
	 */
	void add_posting(om_termname tname, om_docid did, om_termcount wdf);

	/** Apply the diffs.
	 */
	void apply();
};

/** Class managing a set of diffs to a Quartz PositionList database.
 */
class QuartzPositionListDbDiffs : public QuartzDbDiffs {
    private:
	/** Pointer to the database manager.
	 */
	QuartzDbManager * db_manager;

    public:
	/** Construct the diffs object.
	 *
	 *  @param db_manager_  The object managing access to databases on
	 *                      disk.
	 */
	QuartzPositionListDbDiffs(QuartzDbManager * db_manager_)
		: db_manager(db_manager_) {}

	~QuartzPositionListDbDiffs() {}

	/** Add a posting to the diffs.
	 */
	void add_positionlist(om_docid did,
			      om_termname tname,
			      OmDocumentTerm::term_positions positions);

	/** Apply the diffs.
	 */
	void apply();
};

#endif /* OM_HGUARD_QUARTZ_DB_DIFFS_H */
