/* quartz_database.cc: quartz database
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

#include "quartz_database.h"
#include "utils.h"
#include <om/omerror.h>
#include <string>

/// Major version number of required Berkeley DB library
#define DB_DESIRED_VERSION_MAJOR 3

/// Minor version number of required Berkeley DB library
#define DB_DESIRED_VERSION_MINOR 1

/** Internals of QuartzDatabase.
 *
 *  This holds the handles used to access the Berkeley DB library.
 */
class QuartzDatabase::Internals {
    private:
	/// Copying not allowed
	Internals(const Internals &);
	/// Assignment not allowed
	void operator=(const Internals &);
    public:
	static void      check_library_version();
	static u_int32_t calc_env_flags(bool readonly, bool use_transactions);
	static int       calc_mode();
	DbEnv dbenv;
	
	Internals();
	~Internals() {};
};


/** Make a new database.
 */
QuartzDatabase::Internals::Internals()
	: dbenv(DB_CXX_NO_EXCEPTIONS)
{
}

/** Check that the version of Berkeley DB available is correct.
 *
 *  @exception OmFeatureUnavailableError will be thrown if version of
 *  Berkeley DB linked with is incorrect.
 */
void
QuartzDatabase::Internals::check_library_version()
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

/** Calculate the flags required to open the database environment.
 */
u_int32_t
QuartzDatabase::Internals::calc_env_flags(bool readonly, bool use_transactions)
{
    u_int32_t flags = 0;

    // Work out which subsystems to initialise.
    flags |= DB_INIT_MPOOL;
    if (use_transactions) {
	flags |= DB_INIT_LOCK | DB_INIT_LOG | DB_INIT_TXN;

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

/** Calculate the mode that database files should be opened with.
 */
int
QuartzDatabase::Internals::calc_mode()
{
    return S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
}


//
// Compulsory parameters.
// quartz_dbdir  - Directory that the database is stored in.
//
// Optional parameters.
// quartz_tmpdir - Directory in which to store temporary files.  If not
//                 specified, the database directory will be used.
// quartz_envdir - Directory to use to keep the database environment in.
// 		   If not specified, the database directory will be used.
//
QuartzDatabase::QuartzDatabase(const DatabaseBuilderParams & params)
	: internals(new Internals())
{
// FIXME: Make sure that environment is not in a network filesystem, eg NFS.

    Internals::check_library_version();

    string db_dir="quartz";
    string tmp_dir;
    string env_dir;

    bool readonly = false;
    bool use_transactions = false;

    // set cache size parameters, etc, here.

    // open environment here
    // FIXME: check return value
    internals->dbenv.open(db_dir.c_str(),
			  Internals::calc_env_flags(readonly, use_transactions),
			  Internals::calc_mode());

    if (!tmp_dir.empty()) {
	// FIXME: check return value
	internals->dbenv.set_tmp_dir(tmp_dir.c_str());
    }
}

QuartzDatabase::~QuartzDatabase()
{
    // FIXME: check return value
    internals->dbenv.close(0);

    delete internals;
}


void
QuartzDatabase::do_begin_session(om_timeout timeout)
{
}

void
QuartzDatabase::do_end_session()
{
}

void
QuartzDatabase::do_flush()
{
}


void
QuartzDatabase::do_begin_transaction()
{
    throw OmUnimplementedError("QuartzDatabase::do_begin_transaction() not yet implemented");
}

void
QuartzDatabase::do_commit_transaction()
{
    throw OmUnimplementedError("QuartzDatabase::do_commit_transaction() not yet implemented");
}

void
QuartzDatabase::do_cancel_transaction()
{
    throw OmUnimplementedError("QuartzDatabase::do_cancel_transaction() not yet implemented");
}


om_docid
QuartzDatabase::do_add_document(const OmDocumentContents & document)
{
}

void
QuartzDatabase::do_delete_document(om_docid did)
{
}

void
QuartzDatabase::do_replace_document(om_docid did,
				    const OmDocumentContents & document)
{
}


OmDocumentContents
QuartzDatabase::do_get_document(om_docid did)
{
}



om_doccount 
QuartzDatabase::get_doccount() const
{
}

om_doclength
QuartzDatabase::get_avlength() const
{
}

om_doclength
QuartzDatabase::get_doclength(om_docid did) const
{
}

om_doccount
QuartzDatabase::get_termfreq(const om_termname & tname) const
{
}

bool
QuartzDatabase::term_exists(const om_termname & tname) const
{
}


LeafPostList *
QuartzDatabase::open_post_list(const om_termname& tname) const
{
}

LeafTermList *
QuartzDatabase::open_term_list(om_docid did) const
{
}

LeafDocument *
QuartzDatabase::open_document(om_docid did) const
{
}

