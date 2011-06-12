/** @file databasereplicator.cc
 * @brief Support class for database replication.
 */
/* Copyright 2008 Lemur Consulting Ltd
 * Copyright 2009,2010 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>

#include "databasereplicator.h"

#include "xapian/error.h"
#include "xapian/version.h" // For XAPIAN_HAS_XXX_BACKEND.

#include "debuglog.h"
#include "utils.h"

#ifdef XAPIAN_HAS_BRASS_BACKEND
# include "brass/brass_databasereplicator.h"
#endif
#ifdef XAPIAN_HAS_CHERT_BACKEND
# include "chert/chert_databasereplicator.h"
#endif
#ifdef XAPIAN_HAS_FLINT_BACKEND
# include "flint/flint_databasereplicator.h"
#endif

using namespace std;

namespace Xapian {

DatabaseReplicator::~DatabaseReplicator()
{
}

DatabaseReplicator *
DatabaseReplicator::open(const string & path)
{
    LOGCALL_STATIC_VOID(DB, "DatabaseReplicator::DatabaseReplicator", path);

#ifdef XAPIAN_HAS_CHERT_BACKEND
    if (file_exists(path + "/iamchert")) {
	return new ChertDatabaseReplicator(path);
    }
#endif

#ifdef XAPIAN_HAS_FLINT_BACKEND
    if (file_exists(path + "/iamflint")) {
	return new FlintDatabaseReplicator(path);
    }
#endif

#ifdef XAPIAN_HAS_BRASS_BACKEND
    if (file_exists(path + "/iambrass")) {
	return new BrassDatabaseReplicator(path);
    }
#endif

    throw DatabaseOpeningError("Couldn't detect type of database: " + path);
}

}
