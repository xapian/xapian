/* sleepy_termcache.cc: Term cache for sleepycat database
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


#include "omdebug.h"
#include "om/omerror.h"
#include "om/omtypes.h"
#include "sleepy_database_internals.h"
#include "sleepy_termcache.h"

// Sleepycat database stuff
#include <db_cxx.h>

om_termid
SleepyDatabaseTermCache::term_name_to_id(const om_termname &tname) const
{
    DebugMsg("Looking up term ID for `" << tname <<  "' ...");
    Assert(tname.size() != 0);

    Dbt key((void *)tname.c_str(), tname.size());
    Dbt data;
    om_termid tid;

    data.set_flags(DB_DBT_USERMEM);
    data.set_ulen(sizeof(tid));
    data.set_data(&tid);

    // Get, no transactions, no flags
    try {
	int found = internals->termid_db->get(0, &key, &data, 0);
	if(found == DB_NOTFOUND) {
	    tid = 0;
	} else {
	    // Any other errors should cause an exception.
	    Assert(found == 0);

	    if(data.get_size() != sizeof(om_termid)) {
		throw OmDatabaseError("TermidDb: found termname, but data is not a termid.");
	    }
	}
    }
    catch (DbException e) {
	throw OmDatabaseError("TermidDb error: " + std::string(e.what()));
    }

    DEBUGLINE(DB, " found (" << tid << ")");
    return tid;
}

om_termname
SleepyDatabaseTermCache::term_id_to_name(om_termid tid) const
{
    if(tid == 0) throw OmRangeError("Termid 0 not valid");
    DebugMsg("Looking up termname for term ID " << tid << " ...");

    Dbt key(&tid, sizeof(tid));
    Dbt data;
    data.set_flags(DB_DBT_MALLOC);

    // Get, no transactions, no flags
    try {
	int found = internals->termname_db->get(0, &key, &data, 0);
	if(found == DB_NOTFOUND) throw OmRangeError("Termid " +
						    om_tostring(tid) +
						    " not found in termcache");

	// Any other errors should cause an exception.
	Assert(found == 0);
    }
    catch (DbException e) {
	throw OmDatabaseError("TermnameDb error :" + std::string(e.what()));
    }

    om_termname tname((char *)data.get_data(), data.get_size());
    free(data.get_data());

    DEBUGLINE(DB, " found (`" << tname << "')");
    return tname;
}

om_termid
SleepyDatabaseTermCache::assign_new_termid(const om_termname & tname) const
{
    om_termid tid = term_name_to_id(tname);
    if(tid) return tid;

    try {
	int err_num;
	DebugMsg("TermID not in database: adding new id ...");

	Dbt key(&tid, sizeof(tid));
	Dbt data((void *)tname.data(), tname.size());
	key.set_flags(DB_DBT_USERMEM);
	data.set_flags(DB_DBT_USERMEM);

	// Append to list of terms sorted by id - gets new id
	err_num = internals->termname_db->put(NULL, &key, &data, DB_APPEND);
	Assert(err_num == 0); // Any errors should cause an exception.

	// Put in termname to id database
	err_num = internals->termid_db->put(NULL, &data, &key, 0);
	Assert(err_num == 0); // Any errors should cause an exception.

	DEBUGLINE(DB, " added (" << tname << ", " << tid << ")");
    }
    catch (DbException e) {
	throw OmDatabaseError("TermidDb error: " + std::string(e.what()));
    }

    return tid;
}
