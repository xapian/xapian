/* quartz_db_manager.cc: Database management for quartz
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

#include "config.h"

// This is needed so that u_long gets defined, despite our specifying -ansi;
// otherwise db_cxx.h is broken.
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* _GNU_SOURCE */
#include <sys/types.h>
#include <db_cxx.h>

// Needed for macros to specify file modes
#include <sys/stat.h>

#include "quartz_db_manager.h"

#include "utils.h"
#include <om/omerror.h>
#include <string>

/// Major version number of required Berkeley DB library
#define DB_DESIRED_VERSION_MAJOR 3

/// Minor version number of required Berkeley DB library
#define DB_DESIRED_VERSION_MINOR 1

QuartzDBManager::QuartzDBManager(const OmSettings & settings, bool readonly)
	: dbenv(DB_CXX_NO_EXCEPTIONS)
{
    // FIXME: Make sure that environment is not in a network filesystem, eg NFS.

    QuartzDBManager::check_library_version();

    string db_dir=settings.get_value("quartz_dir");
    string tmp_dir=settings.get_value("quartz_tmpdir", db_dir);
    string env_dir=settings.get_value("quartz_envdir", db_dir);

    bool use_transactions = false;

    // set cache size parameters, etc, here.

    // FIXME: check return value
    dbenv.set_tmp_dir(tmp_dir.c_str());

    // open environment here
    // FIXME: check return value
    dbenv.open(db_dir.c_str(),
	       calc_env_flags(readonly, use_transactions),
	       calc_mode());
}

QuartzDBManager::~QuartzDBManager()
{
    // FIXME: check return value
    dbenv.close(0);
}

void
QuartzDBManager::check_library_version()
{
    int major, minor, patch;
    DbEnv::version(&major, &minor, &patch);
    if ((major != DB_DESIRED_VERSION_MAJOR) ||
	(minor != DB_DESIRED_VERSION_MINOR)) {
	throw OmFeatureUnavailableError(
	    string("Incorrect version of Berkeley DB available - found ") +
	    om_tostring(major) + "." +
	    om_tostring(minor) + "." +
	    om_tostring(patch) + "wanted " +
	    om_tostring(DB_DESIRED_VERSION_MAJOR) + "." +
	    om_tostring(DB_DESIRED_VERSION_MINOR) + ".x");
    }
}

u_int32_t
QuartzDBManager::calc_env_flags(bool readonly, bool use_transactions)
{
    u_int32_t flags = 0;

    // Work out which subsystems to initialise.
    flags |= DB_INIT_MPOOL;
    if (use_transactions) {
	flags |= DB_INIT_LOCK;

	flags |= DB_INIT_LOG | DB_INIT_TXN;

	// If using transactions, always must have permission to write to
	// the database anyway, and we want to be sure that normal recovery
	// has been run.  Thus, we specify these flags even in the read only
	// situation.
	flags |= DB_RECOVER | DB_CREATE;
    } else {
	flags |= DB_INIT_CDB;
    }

    if (readonly) {
	flags |= 0;
    } else {
	flags |= DB_CREATE;
    }

#ifdef MUS_USE_PTHREAD
    // Allows access to the dbenv handle from multiple threads
    flags |= DB_THREAD;
#endif
    
    return flags;
}

u_int32_t
QuartzDBManager::calc_db_flags(bool readonly, bool use_transactions)
{
    u_int32_t flags = 0;

    if (readonly) {
	flags |= DB_RDONLY;
    } else {
	flags |= DB_CREATE;
    }

#ifdef MUS_USE_PTHREAD
    // Allows access to the dbenv handle from multiple threads
    flags |= DB_THREAD;
#endif
    
    return flags;
}

int
QuartzDBManager::calc_mode()
{
    return S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
}
