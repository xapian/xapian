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
#include "utils.h"

SleepyDocument::SleepyDocument(Db * document_db_,
			       Db * key_db_,
			       om_docid did_)
	: document_db(document_db_),
	  key_db(key_db_),
	  did(did_),
	  have_data(false)
{
}

SleepyDocument::SleepyDocument(Db * document_db_,
			       Db * key_db_,
			       const OmDocumentContents & document_)
	: document_db(document_db_),
	  key_db(key_db_),
	  data(document_.data),
	  have_data(true),
	  keys(document_.keys)
{
    try {
	int err_num;

	Dbt dbkey1(&did, sizeof(did));
	dbkey1.set_flags(DB_DBT_USERMEM);

	Dbt dbdata1(const_cast<char *>(data.value.data()), data.value.size());
	dbdata1.set_flags(DB_DBT_USERMEM);

	// Append to document database - stores new document id in dbkey1.
	err_num = document_db->put(0, &dbkey1, &dbdata1, DB_APPEND);
	Assert(err_num == 0); // Any errors should cause an exception.

	// Store keys
	map<om_keyno, OmKey>::const_iterator i;
	for(i = keys.begin(); i != keys.end(); i++) {
	    string keyno = inttostring(did) + "_" + inttostring(i->first);
	    Dbt dbkey2(const_cast<char *>(keyno.data()), keyno.size());
	    Dbt dbdata2(const_cast<char *>(i->second.value.data()),
			i->second.value.size());
			

	    err_num = key_db->put(0, &dbkey2, &dbdata2, 0);
	    Assert(err_num == 0); // Any errors should cause an exception.
	}
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
    DebugMsg("Looking up key " << keyid << "...");
    if(keys.find(keyid) != keys.end()) {
	DebugMsg(" found (value == " << keys[keyid].value << ")" << endl);
	return keys[keyid];
    }
    OmKey result;
    try {
	int err_num;
	string keyno = inttostring(did) + "_" + inttostring(keyid);
	DebugMsg(" looking in database (for `" << keyno << "') ...");

	Dbt dbkey(const_cast<char *>(keyno.data()), keyno.size());

	Dbt dbdata;
	dbdata.set_flags(DB_DBT_MALLOC);

	err_num = key_db->get(0, &dbkey, &dbdata, 0);
	if(err_num == DB_NOTFOUND) {
	    DebugMsg(" not found" << endl);
	    keys[keyid] = result;
	    // Return a null key
	    return result;
	}

	result.value = string(reinterpret_cast<char *>(dbdata.get_data()),
			      dbdata.get_size());
	free(dbdata.get_data());

	keys[keyid] = result;
	DebugMsg(" found (value == " << result.value << ")" << endl);
    } catch (DbException e) {
	throw OmDatabaseError("Sleepycat database error, when reading key " +
			      inttostring(keyid) + " from document " +
			      inttostring(did) + ": " + string(e.what()));
    }
    return result;
}

map<om_keyno, OmKey>
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

	    Dbt dbkey(const_cast<om_docid *>(&did),
		      sizeof(did));
	    dbkey.set_flags(DB_DBT_USERMEM);

	    Dbt dbdata;
	    dbdata.set_flags(DB_DBT_MALLOC);

	    err_num = document_db->get(0, &dbkey, &dbdata, 0);
	    if(err_num == DB_NOTFOUND) throw OmRangeError("Document not found");

	    data.value = string(reinterpret_cast<char *>(dbdata.get_data()),
				dbdata.get_size());
	    have_data = true;

	    free(dbdata.get_data());
	} catch (DbException e) {
	    throw OmDatabaseError("DocumentDb error :" + string(e.what()));
	}
    }
    return data;
}
