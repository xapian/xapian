/* sleepy_list.cc: lists of data from a sleepycat database */

#include <algorithm>

#include "omassert.h"
#include "sleepy_list.h"

// Sleepycat database stuff
#include <db_cxx.h>

SleepyList::SleepyList(Db *db_new)
	: db(db_new), opened(false), modified(false),
	  ids(NULL), wdfs(NULL), freqs(NULL), positions(NULL)
{}

SleepyList::~SleepyList()
{
    close();
}

// Packed format:
//    Number of ids
//    Byte offset of start of data (first id)
// (offsets are from this position)
//    Byte offset of first wdf      - 0 if not present
//    Byte offset of first freq     - 0 if not present
//    Byte offset of first position - 0 if not present
//    <ids>
//    <wdfs>
//    <freqs>
//    <positions>

entry_type
SleepyList::readentry(char **pos, char *end)
{
    entry_type *epos = (entry_type *) *pos;
    entry_type entry = *epos ++;
    *pos = (char *)epos;
    if(*pos > end) throw(OmError("Database corrupt - unexpected end of list"));
    return entry;
}

void
SleepyList::open(void *keydata_new, size_t keylen_new)
{
    close();

    // Copy and store key data
    void *keydata = malloc(keylen_new);
    if(keydata == NULL) throw std::bad_alloc();
    memcpy(keydata, keydata_new, keylen_new);
    key.set_data(keydata);
    key.set_size(keylen_new);

    // Read database
    Dbt data;
    data.set_flags(DB_DBT_MALLOC);
    int found;

    try {
	// For now, just read it all at once
	found = db->get(NULL, &key, &data, 0);
    } catch (DbException e) {
	throw OmError("PostlistDb error:" + string(e.what()));
    }
    
    // Unpack entry
    if(found != DB_NOTFOUND) {
	Assert(found == 0); // Any other errors should cause an exception.
	char * pos = (char *) data.get_data();
	char * end = pos + data.get_size();
	entry_type offset;

	// Read number of ids
	length = readentry(&pos, end);
	
	// Read offset of start of ids
	offset = readentry(&pos, end);

	// Current position is where offsets are measured from
	char * data_start = pos;

	// Set position of start of ids
	char * ids_start = data_start + offset;

	// Read position of start of wdfs
	offset = readentry(&pos, end);
	char * wdfs_start = data_start + offset;

	// Copy data
	ids = (entry_type *) malloc(sizeof(entry_type) * length);
	if(ids == NULL) throw std::bad_alloc();
	memcpy(ids, ids_start, sizeof(entry_type) * length);
	wdfs = (entry_type *) malloc(sizeof(entry_type) * length);
	if(wdfs == NULL) throw std::bad_alloc();
	memcpy(wdfs, wdfs_start, sizeof(entry_type) * length);

	free(data.get_data());
    }
    opened = true;
}

void
SleepyList::close()
{
    if(opened) {
	if(modified) {
	    // Pack entry
	    ;

	    try {
		// Write list
		//found = db->put(NULL, &key, &data, 0);
	    } catch (DbException e) {
		throw OmError("PostlistDb error:" + string(e.what()));
	    }
	}
	modified = false;

	free(ids);
	ids = NULL;
	free(wdfs);
	wdfs = NULL;
	free(freqs);
	freqs = NULL;

	free(key.get_data());
	key.set_data(NULL);
    }
    opened = false;
}

void
SleepyList::add(entry_type id, entry_type wdf)
{
    Assert(opened);

    modified = true;
}
