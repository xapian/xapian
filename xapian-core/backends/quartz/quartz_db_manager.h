/* quartz_db_manager.h: Management of databases for quartz
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

#ifndef OM_HGUARD_QUARTZ_DB_MANAGER_H
#define OM_HGUARD_QUARTZ_DB_MANAGER_H

#include "config.h"
#include <om/omsettings.h>
#include "quartz_db_table.h"
#include "quartz_log.h"
#include "refcnt.h"

/** Class managing the databases used by Quartz.
 *
 *  This holds the handles used to access the Berkeley DB library.
 */
class QuartzDbManager : public RefCntBase {
    private:
	/** Whether the tables have been opened: true if opened, false if not.
	 */
	bool tables_open;

	/** Directory to store databases in.
	 */
	string db_dir;

	/** Directory to store temporary files in.
	 */
	string tmp_dir;

	/** Pointer to object to log modifications.
	 */
	auto_ptr<QuartzLog> log;

	/** Whether the database is readonly.
	 */
	bool readonly;

	/** Whether to perform recovery, if it is needed.
	 */
	bool perform_recovery;


	/// Copying not allowed
	QuartzDbManager(const QuartzDbManager &);

	/// Assignment not allowed
	void operator=(const QuartzDbManager &);

	/** Calculate the mode that database files should be created with.
	 */
	static int       calc_mode();

    public:
	/** Table storing posting lists.
	 *
	 *  Whenever an update is performed, this table is the first to be
	 *  updated: therefore, its most recent revision number is the most
	 *  recent anywhere in the database.
	 */
	RefCntPtr<QuartzDbTable> postlist_table;

	/** Table storing position lists.
	 */
	RefCntPtr<QuartzDbTable> positionlist_table;

	/** Table storing term lists.
	 */
	RefCntPtr<QuartzDbTable> termlist_table;

	/** Table storing position lists.
	 */
	RefCntPtr<QuartzDbTable> lexicon_table;

	/** Table storing records.
	 *
	 *  Whenever an update is performed, this table is the last to be
	 *  updated: therefore, its most recent revision number is the most
	 *  recent consistent revision available.
	 */
	RefCntPtr<QuartzDbTable> record_table;


	/** Construct the manager.
	 */
	QuartzDbManager(string db_dir_,
			string tmp_dir_,
			string log_filename_,
			bool readonly_,
			bool perform_recovery_);

	/** Delete the manager.
	 */
	~QuartzDbManager();


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

	/** Open tables at specified revision number.
	 *
	 *  @exception OmInvalidArgumentError is thrown if the specified
	 *  revision is not available.
	 */
	void open_tables(QuartzRevisionNumber revision);

	/** Get an object holding the revision number which the tables are
	 *  opened at.
	 *
	 *  See the documentation for the QuartzRevisionNumber class for
	 *  an explanation of why the actual revision number may not be
	 *  accessed.
	 *
	 *  @return the current revision number.
	 */
	QuartzRevisionNumber get_revision_number() const;
};

#endif /* OM_HGUARD_QUARTZ_DB_MANAGER_H */
