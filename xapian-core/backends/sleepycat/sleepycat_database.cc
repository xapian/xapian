/* sleepy_database.cc: interface to sleepycat database routines */


#include "omassert.h"
#include "sleepy_database.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>

// Sleepycat database stuff
#include <db_cxx.h>

#define FILENAME_POSTLIST "postlist.db"
#define FILENAME_TERMLIST "termlist.db"
#define FILENAME_TERMTOID "termid.db"
#define FILENAME_IDTOTERM "termname.db"

/////////////////////////////
// Internal database state //
/////////////////////////////

class SleepyDatabaseInternals {
    private:
	DbEnv dbenv;
	bool opened;
    public:
	Db *postlist_db;
	Db *termlist_db;
	Db *termid_db;
	Db *termname_db;

	SleepyDatabaseInternals();
	~SleepyDatabaseInternals();
	void open(const string &, bool);
	void close();
};

inline
SleepyDatabaseInternals::SleepyDatabaseInternals() {
    postlist_db = NULL;
    termlist_db = NULL;
    termid_db = NULL;
    termname_db = NULL;
    opened = false;
}

inline
SleepyDatabaseInternals::~SleepyDatabaseInternals(){
    close();
}

inline void
SleepyDatabaseInternals::open(const string &pathname, bool readonly)
{
    // Set up environment
    u_int32_t flags = DB_INIT_CDB;
    int mode = 0;

    if(readonly) {
	flags = DB_RDONLY;
    } else {
	flags = DB_CREATE;
	mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    }
    dbenv.appinit(pathname.c_str(), NULL, flags);
    opened = true;

    Db::open(FILENAME_POSTLIST, DB_BTREE, flags, mode,
	     &dbenv, NULL, &postlist_db);

    Db::open(FILENAME_TERMLIST, DB_BTREE, flags, mode,
	     &dbenv, NULL, &termlist_db);

    Db::open(FILENAME_TERMTOID, DB_HASH, flags, mode,
	     &dbenv, NULL, &termid_db);

    Db::open(FILENAME_IDTOTERM, DB_RECNO, flags, mode,
	     &dbenv, NULL, &termname_db);
}

inline void
SleepyDatabaseInternals::close()
{
    if(postlist_db) postlist_db->close(0);
    postlist_db = NULL;
    if(termlist_db) termlist_db->close(0);
    termlist_db = NULL;
    if(termid_db) termid_db->close(0);
    termid_db = NULL;
    if(termname_db) termname_db->close(0);
    termname_db = NULL;

    if(opened) dbenv.appexit();
}

///////////////
// Postlists //
///////////////

SleepyPostList::SleepyPostList(docid *data_new, doccount termfreq_new) {
    pos = 0;
    data = data_new;
    termfreq = termfreq_new;
}


SleepyPostList::~SleepyPostList() {
    free(data);
}

weight SleepyPostList::get_weight() const {
    return 1;
}

///////////////
// Termlists //
///////////////

SleepyTermList::SleepyTermList(termid *data_new, termcount terms_new) {
    pos = 0;
    data = data_new;
    terms = terms_new;
}

SleepyTermList::~SleepyTermList() {
    free(data);
}

///////////////////////////
// Actual database class //
///////////////////////////

SleepyDatabase::SleepyDatabase() {
    internals = new SleepyDatabaseInternals();
}

SleepyDatabase::~SleepyDatabase() {
    delete internals;
}

void SleepyDatabase::open(const string &pathname, bool readonly) {
    // Open databases
    // FIXME - catch exceptions
    try {
	internals->open(pathname, readonly);
    }
    catch (DbException e) {
	throw (OmError(string("Database error on open: ") + e.what()));
    }
}

void SleepyDatabase::close() {
    // Close databases
    // FIXME - catch exceptions
    try {
	internals->close();
    }
    catch (DbException e) {
	throw (OmError(string("Database error on close: ") + e.what()));
    }
}

PostList *
SleepyDatabase::open_post_list(termid id) const {
    Dbt key(&id, sizeof(id));
    Dbt data;

    // FIXME - should use DB_DBT_USERMEM and DB_DBT_PARTIAL eventually
    data.set_flags(DB_DBT_MALLOC);

    // Get, no transactions, no flags
    try {
	int found = internals->postlist_db->get(NULL, &key, &data, 0);
	if(found == DB_NOTFOUND) throw RangeError("Termid not found");

	// Any other errors should cause an exception.
	Assert(found == 0);
    }
    catch (DbException e) {
	throw OmError("PostlistDb error:" + string(e.what()));
    }

    return new SleepyPostList((docid *)data.get_data(),
			      data.get_size() / sizeof(docid));
}

TermList *
SleepyDatabase::open_term_list(docid id) const {
    Dbt key(&id, sizeof(id));
    Dbt data;

    // FIXME - should use DB_DBT_USERMEM and DB_DBT_PARTIAL eventually
    data.set_flags(DB_DBT_MALLOC);

    // Get, no transactions, no flags
    try {
	int found = internals->postlist_db->get(NULL, &key, &data, 0);
	if(found == DB_NOTFOUND) throw RangeError("Termid not found");

	// Any other errors should cause an exception.
	Assert(found == 0);
    }
    catch (DbException e) {
	throw OmError("TermlistDb error:" + string(e.what()));
    }

    return new SleepyTermList((termid *)data.get_data(),
			      data.get_size() / sizeof(termid));
}

termid
SleepyDatabase::add_term(const termname &tname) {
    termid newid;
    newid = term_name_to_id(tname);
    if(newid) return newid;

    try {
	// FIXME - currently no transactions
	Dbt key(&newid, sizeof(newid));
	Dbt data((void *)tname.data(), tname.size());
	int found;

	key.set_flags(DB_DBT_USERMEM);

	// Append to list of terms sorted by id - gets new id
	found = internals->termname_db->put(NULL, &key, &data, DB_APPEND);
	Assert(found == 0); // Any errors should cause an exception.
	
	// Put in termname to id database
	found = internals->termid_db->put(NULL, &data, &key, 0);
	Assert(found == 0); // Any errors should cause an exception.
    }
    catch (DbException e) {
	throw OmError("SleepyDatabase::add_term(): " + string(e.what()));
    }

    return newid;
}

docid
SleepyDatabase::add_doc(IRDocument &doc) {
    throw OmError("SleepyDatabase.add_doc() not implemented");
}

void
SleepyDatabase::add(termid tid, docid did) {
    // Add to Postlist
    try {
	// First see if appropriate postlist already exists
	Dbt key(&tid, sizeof(tid));
	Dbt data;
	data.set_flags(DB_DBT_MALLOC);
	docid * postlist = NULL;
	size_t postlist_size = 0;
	int found;

	found = internals->postlist_db->get(NULL, &key, &data, 0);
	if(found != DB_NOTFOUND) {
	    Assert(found == 0); // Any other errors should cause an exception.

	    postlist = (docid *)data.get_data();
	    postlist_size = data.get_size();
	}

	// Look through postlist for doc id
	docid * pos = postlist;
	docid * end = postlist + postlist_size / sizeof(docid);
	while(pos != end && *pos < did) pos++;
	if(pos != end && *pos == did) {
	    // Already there
	} else {
	    // Add doc id to postlist
	    postlist_size += sizeof(docid);
	    docid * postlist_new;
	    postlist_new = (docid *) realloc(postlist, postlist_size);
	    if(postlist_new == NULL) {
		free(postlist);
		throw std::bad_alloc();
	    }
	    pos = pos - postlist + postlist_new;
	    postlist = postlist_new;
	    memmove(pos + 1, pos,
		    postlist_size - (1 + (pos - postlist)) * sizeof(docid));
	    *pos = did;
	}

	// Save new postlist
	data.set_data(postlist);
	data.set_size(postlist_size);
	data.set_flags(0);
	found = internals->postlist_db->put(NULL, &key, &data, 0);
	Assert(found == 0); // Any errors should cause an exception.

#if 1
	cout << "New postlist: (";
	for(docid *pos = postlist;
	    pos != postlist + postlist_size / sizeof(docid);
	    pos++) {
	    cout << *pos << " ";
	}
	cout << ")" << endl;
#endif

	free(postlist);
    }
    catch (DbException e) {
	throw OmError("PostlistDb error:" + string(e.what()));
    }
}

termid
SleepyDatabase::term_name_to_id(const termname &tname) const {
    Dbt key((void *)tname.c_str(), tname.size());
    Dbt data;
    termid id;

    data.set_flags(DB_DBT_USERMEM);
    data.set_ulen(sizeof(id));
    data.set_data(&id);

    // Get, no transactions, no flags
    try {
	int found = internals->termid_db->get(NULL, &key, &data, 0);
	if(found == DB_NOTFOUND) return 0;

	// Any other errors should cause an exception.
	Assert(found == 0);
    }
    catch (DbException e) {
	throw OmError("TermidDb error:" + string(e.what()));
    }

    if(data.get_size() != sizeof(termid)) {
	// FIXME - test what _does_ happen if the termname is not the
	// size of an id.
	throw OmError("TermidDb: found termname, but data is not a termid.");
    }
    return id;
}

termname
SleepyDatabase::term_id_to_name(termid id) const {
    Dbt key(&id, sizeof(id));
    Dbt data;

    // Get, no transactions, no flags
    try {
	int found = internals->termname_db->get(NULL, &key, &data, 0);
	if(found == DB_NOTFOUND) throw RangeError("Termid not found");

	// Any other errors should cause an exception.
	Assert(found == 0);
    }
    catch (DbException e) {
	throw OmError("TermidDb error:" + string(e.what()));
    }

    termname tname((char *)data.get_data(), data.get_size());
    return tname;
}
