/* sleepy_document.cc: A document in a sleepycat database
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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

#include "sleepy_document.h"
#include <om/omdocument.h>

SleepyDocument::SleepyDocument(Db * db_, om_docid did_)
	: db(db_),
	  did(did_),
	  have_data(false)
{
}

SleepyDocument::SleepyDocument(Db * db_,
			       const OmData & data_,
			       const map<om_keyno, OmKey> & keys_ )
	: db(db_),
	  data(data_),
	  have_data(true),
	  keys(keys_)
{
    try {
	int err_num;

	Dbt dbkey(&did, sizeof(did));
	dbkey.set_flags(DB_DBT_USERMEM);

	Dbt dbdata((void *)data.value.data(), data.value.size());
	dbdata.set_flags(DB_DBT_USERMEM);

	// Append to document database - stores new document id in dbkey.
	err_num = db->put(NULL, &dbkey, &dbdata, DB_APPEND);
	Assert(err_num == 0); // Any errors should cause an exception.

	// FIXME: store keys
    } catch (DbException e) {
	throw OmDatabaseError("DocumentDb Error: " + string(e.what()));
    }
}

om_docid
SleepyDocument::get_docid() const
{
    return did;
}

OmKey
SleepyDocument::do_get_key(om_keyno keyid) const
{
    throw OmUnimplementedError("SleepyDocument::do_get_key() unimplemented");
}

vector<OmKey>
SleepyDocument::do_get_all_keys() const
{
    throw OmUnimplementedError("SleepyDocument::do_get_all_keys() unimplemented");
}

OmData
SleepyDocument::do_get_data() const
{
    if(!have_data) {
	try {
	    int err_num;

	    Dbt dbkey(const_cast<om_docid *>(&did), sizeof(did));
	    dbkey.set_flags(DB_DBT_USERMEM);

	    Dbt dbdata;
	    dbdata.set_flags(DB_DBT_MALLOC);

	    err_num = db->get(NULL, &dbkey, &dbdata, 0);
	    if(err_num == DB_NOTFOUND) throw OmRangeError("Document not found");

	    data.value = string((char *)dbdata.get_data(), dbdata.get_size());
	    have_data = true;

	    free(dbdata.get_data());
	} catch (DbException e) {
	    throw OmDatabaseError("DocumentDb error :" + string(e.what()));
	}
    }
    return data;
}
