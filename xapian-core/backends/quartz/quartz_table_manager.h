/* quartz_table_manager.h: Management of tables for quartz
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

#ifndef OM_HGUARD_QUARTZ_TABLE_MANAGER_H
#define OM_HGUARD_QUARTZ_TABLE_MANAGER_H

#include "config.h"
#include <om/omsettings.h>
#include "quartz_table.h"
#include "quartz_log.h"
#include "om/autoptr.h"

/** Class managing the tables used by a Quartz database.
 */
class QuartzTableManager {
    private:
	/// Copying not allowed
	QuartzTableManager(const QuartzTableManager &);

	/// Assignment not allowed
	void operator=(const QuartzTableManager &);

    public:
	/** 
	 */
	QuartzTableManager() {}
	
	/** Delete the manager.
	 */
	virtual ~QuartzTableManager() {}

	/** Get table storing posting lists.
	 */
	virtual QuartzTable * get_postlist_table() = 0;

	/** Get table storing position lists.
	 */
	virtual QuartzTable * get_positionlist_table() = 0;

	/** Get table storing term lists.
	 */
	virtual QuartzTable * get_termlist_table() = 0;

	/** Get table storing lexicon.
	 */
	virtual QuartzTable * get_lexicon_table() = 0;

	/** Get table storing attributes.
	 */
	virtual QuartzTable * get_attribute_table() = 0;

	/** Get table storing records.
	 */
	virtual QuartzTable * get_record_table() = 0;
};

/** Class managing the disk tables used by Quartz.
 *
 *  This finds the tables, opens them at consistent revisions, manages
 *  determining the current and next revision numbers, and stores handles
 *  to the tables.
 */
class QuartzDiskTableManager : public QuartzTableManager {
    private:
	/** Directory to store databases in.
	 */
	string db_dir;

	/** Directory to store temporary files in.
	 */
	string tmp_dir;

	/** Whether the database is readonly.
	 */
	bool readonly;

	/** Table storing posting lists.
	 *
	 *  Whenever an update is performed, this table is the first to be
	 *  updated: therefore, its most recent revision number is the most
	 *  recent anywhere in the database.
	 */
	QuartzDiskTable postlist_table;

	/** Table storing position lists.
	 */
	QuartzDiskTable positionlist_table;

	/** Table storing term lists.
	 */
	QuartzDiskTable termlist_table;

	/** Table storing lexicon.
	 */
	QuartzDiskTable lexicon_table;

	/** Table storing keys.
	 */
	QuartzDiskTable attribute_table;

	/** Table storing records.
	 *
	 *  Whenever an update is performed, this table is the last to be
	 *  updated: therefore, its most recent revision number is the most
	 *  recent consistent revision available.  If this tables most
	 *  recent revision number is not available for all tables, there
	 *  is no consistent revision available, and the database is corrupt.
	 */
	QuartzDiskTable record_table;


	/// Copying not allowed
	QuartzDiskTableManager(const QuartzDiskTableManager &);

	/// Assignment not allowed
	void operator=(const QuartzDiskTableManager &);

	/** Open all tables at most recent revision.
	 *
	 *  @exception OmNeedRecoveryError is thrown if versions are not
	 *             consistent.
	 */
	void open_tables_newest();

	/** Open all tables at most recent consistent revision.
	 *
	 *  @exception OmDatabaseCorruptError is thrown if there is no
	 *  consistent revision available.
	 */
	void open_tables_consistent();

	/// Return the path that the record table is stored at.
	string record_path() const;

	/// Return the path that the attribute table is stored at.
	string attribute_path() const;

	/// Return the path that the lexicon table is stored at.
	string lexicon_path() const;

	/// Return the path that the termlist table is stored at.
	string termlist_path() const;

	/// Return the path that the positionlist table is stored at.
	string positionlist_path() const;

	/// Return the path that the postlist table is stored at.
	string postlist_path() const;
    public:

	/** Pointer to object to log modifications.
	 */
	AutoPtr<QuartzLog> log;


	/** Construct the manager.
	 *
	 *  @exception OmNeedRecoveryError is thrown if versions are not
	 *             consistent, and perform_recovery is not specified.
	 *  @exception OmDatabaseCorruptError is thrown if there is no
	 *             consistent revision available.
	 */
	QuartzDiskTableManager(string db_dir_,
			       string tmp_dir_,
			       string log_filename_,
			       bool readonly_,
			       bool perform_recovery);

	/** Delete the manager.
	 */
	~QuartzDiskTableManager();


	/** Open tables at specified revision number.
	 *
	 *  @exception OmInvalidArgumentError is thrown if the specified
	 *  revision is not available.
	 */
	void open_tables(QuartzRevisionNumber revision);

	/** Get an object holding the revision number which the tables are
	 *  opened at.
	 *
	 *  @return the current revision number.
	 */
	QuartzRevisionNumber get_revision_number() const;

	/** Get an object holding the next revision number which should be
	 *  used in the tables.
	 *
	 *  @return the next revision number.
	 */
	QuartzRevisionNumber get_next_revision_number() const;

	/** Set the revision number in the tables.
	 *
	 *  This updates the disk tables so that the currently open revision
	 *  becomes the specified revision number.
	 *
	 *  @param new_revision The new revision number to store.  This must
	 *          be greater than the latest revision number (see
	 *          get_latest_revision_number()), or undefined behaviour will
	 *          result.
	 */
	bool set_revision_number(QuartzRevisionNumber new_revision);
	
	/** Virtual methods of QuartzTableManager
	 */
	//@{
	QuartzDiskTable * get_postlist_table();
	QuartzDiskTable * get_positionlist_table();
	QuartzDiskTable * get_termlist_table();
	QuartzDiskTable * get_lexicon_table();
	QuartzDiskTable * get_attribute_table();
	QuartzDiskTable * get_record_table();
	//@}
};

class QuartzBufferedTableManager : public QuartzTableManager {
    private:
	/// Copying not allowed
	QuartzBufferedTableManager(const QuartzBufferedTableManager &);

	/// Assignment not allowed
	void operator=(const QuartzBufferedTableManager &);

	/** The tables stored on disk.
	 */
	QuartzDiskTableManager disktables;
 
	/** Buffered tables encapsulating tables on disk.
	 */
	//@{
	QuartzBufferedTable postlist_buffered_table;
	QuartzBufferedTable positionlist_buffered_table;
	QuartzBufferedTable termlist_buffered_table;
	QuartzBufferedTable lexicon_buffered_table;
	QuartzBufferedTable attribute_buffered_table;
	QuartzBufferedTable record_buffered_table;
	//@}

    public:
	/** Construct the manager.
	 *
	 *  This may throw any exceptions which are thrown by
	 *  QuartzDiskTableManager's constructor.
	 */
	QuartzBufferedTableManager(string db_dir_,
				   string tmp_dir_,
				   string log_filename_,
				   bool perform_recovery);
	
	/** Destroy the manager.  Any anapplied modifications will
	 *  be lost.
	 */
	~QuartzBufferedTableManager();
	
	/** Apply any outstanding changes to the tables.
	 *
	 *  If an error occurs during the operation, this will be signalled
	 *  by a return value of false.  The tables on disk will be left in
	 *  an unmodified state (though possibly with increased revision
	 *  numbers), and the changes made will be lost.
	 *
	 *  @return true if the operation completed successfully, false
	 *          otherwise.
	 */
	bool apply();

	/** Virtual methods of QuartzTableManager
	 */
	//@{
	QuartzBufferedTable * get_postlist_table();
	QuartzBufferedTable * get_positionlist_table();
	QuartzBufferedTable * get_termlist_table();
	QuartzBufferedTable * get_lexicon_table();
	QuartzBufferedTable * get_attribute_table();
	QuartzBufferedTable * get_record_table();
	//@}
};

#endif /* OM_HGUARD_QUARTZ_TABLE_MANAGER_H */
