/* quartz_diffs.h: Diffs made to a given quartz database
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

#ifndef OM_HGUARD_QUARTZ_DIFFS_H
#define OM_HGUARD_QUARTZ_DIFFS_H

#include "config.h"
#include "quartz_table_manager.h"
#include "quartz_table_entries.h"
#include "quartz_table.h"
#include "om/omtypes.h"
#include "om/omindexdoc.h"

/** Base class managing a set of diffs to a table in a Quartz database.
 */
class QuartzDiffs {
    private:
	/// Copying not allowed
	QuartzDiffs(const QuartzDiffs &);

	/// Assignment not allowed
	void operator=(const QuartzDiffs &);

    protected:
	/** Blocks which have been changed.
	 */
	QuartzTableEntries changed_entries;

	/** Table which blocks come from / get written to.
	 */
	QuartzDbTable * table;

	/** Get a pointer to the tag for a given key.
	 *
	 *  If the tag is not present in the database, or is currently
	 *  marked for deletion, this will return a null pointer.
	 *
	 *  The pointer is owned by the changed_entries object - it may
	 *  be modified, but must not be deleted.
	 */
	QuartzDbTag * get_tag(const QuartzDbKey &key);

	/** Get a pointer to the tag for a given key, creating a new tag if
	 *  not present.
	 *
	 *  This will never return a null pointer.
	 *
	 *  The pointer is owned by the changed_entries object - it may
	 *  be modified, but must not be deleted.
	 */
	QuartzDbTag * get_or_make_tag(const QuartzDbKey &key);

    public:
	/** Construct the diffs object.
	 */
	QuartzDiffs(QuartzDbTable * table_) : table(table_) {}

	/** Destroy the diffs.  Any unapplied diffs will be lost.
	 *
	 *  Virtual to allow full destruction when a subclass is deleted.
	 *  Pure virtual to make the class abstract.
	 */
	virtual ~QuartzDiffs() = 0;

	/** Apply the diffs to the database.
	 *
	 *  If an error occurs during the operation, this will be signalled
	 *  by a return value of false.  The table will be left in an
	 *  unmodified state.
	 *
	 *  After this operation, even if it is unsuccessful, the diffs
	 *  will be left empty.
	 *
	 *  @param new_revision  The new revision number to store the
	 *                       modifications under.
	 *
	 *  @return true if the operation completed successfully, false
	 *          otherwise.
	 */
	bool apply(QuartzRevisionNumber new_revision);

	/** Determine whether the object contains modifications.
	 *
	 *  @return true if the diffs object contains modifications to the
	 *          database, false if no changes have been made.
	 */
	bool is_modified();
};

inline QuartzDiffs::~QuartzDiffs() {}



/** Class managing a set of diffs to a Quartz PostList table.
 */
class QuartzPostListDiffs : public QuartzDiffs {
    private:
    public:
	/** Construct the diffs object.
	 *
	 *  @param table_  The object managing access to the table on disk.
	 */
	QuartzPostListDiffs(QuartzDbTable * table_)
		: QuartzDiffs(table_) {}

	/** Destroy the diffs.  Any unapplied diffs will be lost.
	 */
	~QuartzPostListDiffs() {}

	/** Add a posting to the diffs.
	 *
	 *  @param tname  The name of the term whose posting list an entry
	 *                should be added to.
	 *  @param did    The document ID to add to the posting list.
	 *  @param wdf    The within document frequency to store in the
	 *                posting list.
	 */
	void add_posting(om_termname tname, om_docid did, om_termcount wdf);
};

/** Class managing a set of diffs to a Quartz PositionList table.
 */
class QuartzPositionListDiffs : public QuartzDiffs {
    private:
    public:
	/** Construct the diffs object.
	 *
	 *  @param table_  The object managing access to the table on disk.
	 */
	QuartzPositionListDiffs(QuartzDbTable * table_)
		: QuartzDiffs(table_) {}

	/** Destroy the diffs.  Any unapplied diffs will be lost.
	 */
	~QuartzPositionListDiffs() {}

	/** Add a posting to the diffs.
	 */
	void add_positionlist(om_docid did,
			      om_termname tname,
			      OmDocumentTerm::term_positions positions);
};

/** Class managing a set of diffs to a Quartz TermList table.
 */
class QuartzTermListDiffs : public QuartzDiffs {
    private:
    public:
	/** Construct the diffs object.
	 *
	 *  @param table_  The object managing access to the table on disk.
	 */
	QuartzTermListDiffs(QuartzDbTable * table_)
		: QuartzDiffs(table_) {}

	/** Destroy the diffs.  Any unapplied diffs will be lost.
	 */
	~QuartzTermListDiffs() {}
};

/** Class managing a set of diffs to a Quartz Record table.
 */
class QuartzRecordDiffs : public QuartzDiffs {
    private:
    public:
	/** Construct the diffs object.
	 *
	 *  @param table_  The object managing access to the table on disk.
	 */
	QuartzRecordDiffs(QuartzDbTable * table_)
		: QuartzDiffs(table_) {}

	/** Destroy the diffs.  Any unapplied diffs will be lost.
	 */
	~QuartzRecordDiffs() {}

	/** Add a record.
	 */
	void add_record(om_docid did, const OmData & data);
};

/** Class managing a set of diffs to a Quartz Lexicon table.
 */
class QuartzLexiconDiffs : public QuartzDiffs {
    private:
    public:
	/** Construct the diffs object.
	 *
	 *  @param table_  The object managing access to the table on disk.
	 */
	QuartzLexiconDiffs(QuartzDbTable * table_)
		: QuartzDiffs(table_) {}

	/** Destroy the diffs.  Any unapplied diffs will be lost.
	 */
	~QuartzLexiconDiffs() {}
};

/** Class managing a set of diffs to a Quartz Attribute table.
 */
class QuartzAttributeDiffs : public QuartzDiffs {
    private:
    public:
	/** Construct the diffs object.
	 *
	 *  @param table_  The object managing access to the table on disk.
	 */
	QuartzAttributeDiffs(QuartzDbTable * table_)
		: QuartzDiffs(table_) {}

	/** Destroy the diffs.  Any unapplied diffs will be lost.
	 */
	~QuartzAttributeDiffs() {}
};

#endif /* OM_HGUARD_QUARTZ_DIFFS_H */
