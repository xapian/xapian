/* sleepycat_database_internals.h: interface to sleepycat database routines
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

#ifndef OM_HGUARD_SLEEPYCAT_DATABASE_INTERNALS_H
#define OM_HGUARD_SLEEPYCAT_DATABASE_INTERNALS_H

#include "config.h"
#ifdef MUS_BUILD_BACKEND_SLEEPYCAT

// Sleepycat database stuff
#include <db_cxx.h>

/** Internals of the sleepycat database.
 *
 *  This class allows the internals of the sleepycat database (ie,
 *  the sleepycat library includes) to be hidden from view externally.
 */
class SleepycatDatabaseInternals {
    private:
	DbEnv dbenv;

	/// Flag to record whether database has been opened.
	bool opened;

	/// Copying not allowed.
	SleepycatDatabaseInternals(const SleepycatDatabaseInternals &);

	/// Assignment not allowed.
	void operator=(const SleepycatDatabaseInternals &);
    public:
	Db *postlist_db;
	Db *termlist_db;
	Db *termid_db;
	Db *termname_db;
	Db *document_db;
	Db *key_db;
	Db *stats_db;

	SleepycatDatabaseInternals();
	~SleepycatDatabaseInternals();

	// open - FIXME - make this into the constructor.
	void open(const std::string & pathname, bool readonly);
	void close();

	om_doccount get_doccount() const;
	om_totlength get_totlength() const;
	void set_doccount(om_doccount doccount);
	void set_totlength(om_totlength doclength);
};

#endif /* MUS_BUILD_BACKEND_SLEEPYCAT */

#endif /* OM_HGUARD_SLEEPYCAT_DATABASE_INTERNALS_H */
