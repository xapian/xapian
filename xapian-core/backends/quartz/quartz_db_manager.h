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
#include "autoptr.h"

/** Class managing the databases used by Quartz.
 *
 *  This holds the handles used to access the Berkeley DB library.
 */
class QuartzDbManager : public RefCntBase {
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


	/// Copying not allowed
	QuartzDbManager(const QuartzDbManager &);

	/// Assignment not allowed
	void operator=(const QuartzDbManager &);

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

	/// Return the path that the lexicon table is stored at.
	string lexicon_path() const;

	/// Return the path that the termlist table is stored at.
	string termlist_path() const;

	/// Return the path that the positionlist table is stored at.
	string positionlist_path() const;

	/// Return the path that the postlist table is stored at.
	string postlist_path() const;
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
	 *  recent consistent revision available.  If this tables most
	 *  recent revision number is not available for all tables, there
	 *  is no consistent revision available, and the database is corrupt.
	 */
	RefCntPtr<QuartzDbTable> record_table;


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
	QuartzDbManager(string db_dir_,
			string tmp_dir_,
			string log_filename_,
			bool readonly_,
			bool perform_recovery);

	/** Delete the manager.
	 */
	~QuartzDbManager();


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
};

#endif /* OM_HGUARD_QUARTZ_DB_MANAGER_H */
