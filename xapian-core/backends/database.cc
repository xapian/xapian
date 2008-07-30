/* database.cc: Database factories and base class
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007,2008 Olly Betts
 * Copyright 2008 Lemur Consulting Ltd
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

#include "database.h"

#include "utils.h" // for om_tostring
#include "fileutils.h"
#include <xapian/dbfactory.h>
#include <xapian/error.h>
#include <xapian/version.h> // For XAPIAN_HAS_XXX_BACKEND

#include <fstream>
#include <string>

using namespace std;

// Include headers for all the enabled database backends
#ifdef XAPIAN_HAS_CHERT_BACKEND
#include "chert/chert_database.h"
#endif
#ifdef XAPIAN_HAS_FLINT_BACKEND
#include "flint/flint_database.h"
#endif
#ifdef XAPIAN_HAS_INMEMORY_BACKEND
#include "inmemory/inmemory_database.h"
#endif

namespace Xapian {

#ifdef XAPIAN_HAS_CHERT_BACKEND
Database
Chert::open(const string &dir) {
    DEBUGAPICALL_STATIC(Database, "Chert::open", dir);
    return Database(new ChertDatabase(dir));
}

WritableDatabase
Chert::open(const string &dir, int action, int block_size) {
    DEBUGAPICALL_STATIC(WritableDatabase, "Chert::open", dir << ", " <<
			action << ", " << block_size);
    return WritableDatabase(new ChertWritableDatabase(dir, action, block_size));
}
#endif

#ifdef XAPIAN_HAS_FLINT_BACKEND
Database
Flint::open(const string &dir) {
    DEBUGAPICALL_STATIC(Database, "Flint::open", dir);
    return Database(new FlintDatabase(dir));
}

WritableDatabase
Flint::open(const string &dir, int action, int block_size) {
    DEBUGAPICALL_STATIC(WritableDatabase, "Flint::open", dir << ", " <<
			action << ", " << block_size);
    return WritableDatabase(new FlintWritableDatabase(dir, action, block_size));
}
#endif

#ifdef XAPIAN_HAS_INMEMORY_BACKEND
// Note: a read-only inmemory database will always be empty, and so there's
// not much use in allowing one to be created.
WritableDatabase
InMemory::open() {
    DEBUGAPICALL_STATIC(Database, "InMemory::open", "");
    return WritableDatabase(new InMemoryDatabase);
}
#endif

static void
open_stub(Database *db, const string &file)
{
    // A stub database is a text file with one or more lines of this format:
    // <dbtype> <serialised db object>
    //
    // Lines which start with a "#" character, and lines which have no spaces
    // in them, are ignored.
    //
    // Any paths specified in stub database files which are relative will be
    // considered to be relative to the directory containing the stub database.
    ifstream stub(file.c_str());
    string stubdir = calc_dirname(file);
    string line;
    int line_no = 0;
    bool ok = false;
    while (getline(stub, line)) {
	++line_no;
	if (line.empty() || line[0] == '#')
	    continue;
	string::size_type space = line.find(' ');
	if (space == string::npos)
	    continue;

	ok = false;
	string type = line.substr(0, space);
	line.erase(0, space + 1);
	if (type == "auto") {
	    db->add_database(Database(join_paths(stubdir, line)));
	    ok = true;
#ifdef XAPIAN_HAS_FLINT_BACKEND
	} else if (type == "flint") {
	    db->add_database(Flint::open(join_paths(stubdir, line)));
	    ok = true;
#endif
#ifdef XAPIAN_HAS_CHERT_BACKEND
	} else if (type == "chert") {
	    db->add_database(Chert::open(join_paths(stubdir, line)));
	    ok = true;
#endif
#ifdef XAPIAN_HAS_REMOTE_BACKEND
	} else if (type == "remote") {
	    string::size_type colon = line.find(':');
	    if (colon == 0) {
		// prog
		// FIXME: timeouts
		// FIXME: Is prog actually useful beyond testing?
		// Is it a security risk?
		space = line.find(' ');
		string args;
		if (space != string::npos) {
		    args = line.substr(space + 1);
		    line = line.substr(1, space - 1);
		} else {
		    line.erase(0, 1);
		}
		db->add_database(Remote::open(line, args));
		ok = true;
	    } else if (colon != string::npos) {
		// tcp
		// FIXME: timeouts
		unsigned int port = atoi(line.c_str() + colon + 1);
		line.erase(colon);
		db->add_database(Remote::open(line, port));
		ok = true;
	    }
#endif
	}
	if (!ok) break;
    }
    if (!ok) {
	// Don't include the line itself - that might help an attacker
	// by revealing part of a sensitive file's contents if they can
	// arrange for it to be read as a stub database via infelicities in
	// an application which uses Xapian.  The line number is enough
	// information to identify the problem line.
	throw DatabaseOpeningError("Bad line " + om_tostring(line_no) + " in stub database file `" + file + "'");
    }
}

Database
Auto::open_stub(const string &file)
{
    DEBUGAPICALL_STATIC(Database, "Auto::open_stub", file);
    Database db;
    open_stub(&db, file);
    return db;
}

Database::Database(const string &path)
{
    DEBUGAPICALL(void, "Database::Database", path);
    // Check for path actually being a file - if so, assume it to be
    // a stub database.
    if (file_exists(path)) {
	open_stub(this, path);
	return;
    }

#ifdef XAPIAN_HAS_FLINT_BACKEND
    if (file_exists(path + "/iamflint")) {
	internal.push_back(new FlintDatabase(path));
	return;
    }
#endif

#ifdef XAPIAN_HAS_CHERT_BACKEND
    if (file_exists(path + "/iamchert")) {
	internal.push_back(new ChertDatabase(path));
	return;
    }
#endif

    // Check for "stub directories".
    if (file_exists(path + "/XAPIANDB")) {
	open_stub(this, path + "/XAPIANDB");
	return;
    }

    throw DatabaseOpeningError("Couldn't detect type of database");
}

WritableDatabase::WritableDatabase(const std::string &path, int action)
    : Database()
{
    DEBUGAPICALL(void, "WritableDatabase::WritableDatabase",
		 path << ", " << action);
#ifdef XAPIAN_HAS_FLINT_BACKEND
# ifdef XAPIAN_HAS_CHERT_BACKEND
    bool use_flint = true;
    if (file_exists(path + "/iamflint")) {
	// Existing flint DB.
    } else if (file_exists(path + "/iamchert")) {
	// Existing chert DB.
	use_flint = false;
    } else {
	// If $XAPIAN_PREFER_CHERT is set to a non-empty value, prefer chert.
	const char *p = getenv("XAPIAN_PREFER_CHERT");
	if (p && *p) use_flint = false;
    }

    if (use_flint) {
	internal.push_back(new FlintWritableDatabase(path, action, 8192));
    } else {
	internal.push_back(new ChertWritableDatabase(path, action, 8192));
    }
# else
    // Only Flint is enabled.
    internal.push_back(new FlintWritableDatabase(path, action, 8192));
# endif
#else
# ifdef XAPIAN_HAS_CHERT_BACKEND
    // Only Chert is enabled.
    internal.push_back(new ChertWritableDatabase(path, action, 8192));
# else
    throw FeatureUnavailableError("No disk-based writable backend is enabled");
# endif
#endif
}

///////////////////////////////////////////////////////////////////////////////

Database::Internal::Internal()
	: transaction_state(TRANSACTION_NONE)
{
}

Database::Internal::~Internal()
{
}

void
Database::Internal::keep_alive()
{
    // For the normal case of local databases, nothing needs to be done.
}


Xapian::doccount
Database::Internal::get_value_freq(Xapian::valueno) const
{
    throw Xapian::UnimplementedError("This backend doesn't support get_value_freq");
}

std::string
Database::Internal::get_value_lower_bound(Xapian::valueno) const
{
    return "";
}

std::string
Database::Internal::get_value_upper_bound(Xapian::valueno) const
{
    throw Xapian::UnimplementedError("This backend doesn't support get_value_upper_bound");
}

// Discard any exceptions - we're called from the destructors of derived
// classes so we can't safely throw.
void
Database::Internal::dtor_called()
{
    try {
	if (transaction_active()) {
	    cancel_transaction();
	} else if (transaction_state == TRANSACTION_NONE) {
	    flush();
	}
    } catch (...) {
	// We can't safely throw exceptions from a destructor in case an
	// exception is already active and causing us to be destroyed.
    }
}

void
Database::Internal::flush()
{
    // Writable databases should override this method.
    Assert(false);
}

void
Database::Internal::cancel()
{
    // Writable databases should override this method.
    Assert(false);
}

void
Database::Internal::begin_transaction(bool flushed)
{
    if (transaction_state != TRANSACTION_NONE) {
	if (transaction_state == TRANSACTION_UNIMPLEMENTED)
	    throw Xapian::UnimplementedError("This backend doesn't implement transactions");
	throw InvalidOperationError("Cannot begin transaction - transaction already in progress");
    }
    if (flushed) {
	// N.B. Call flush() before we set transaction_state since flush()
	// isn't allowing during a transaction.
	flush();
	transaction_state = TRANSACTION_FLUSHED;
    } else {
	transaction_state = TRANSACTION_UNFLUSHED;
    }
}

void
Database::Internal::commit_transaction()
{
    if (!transaction_active()) {
	if (transaction_state == TRANSACTION_UNIMPLEMENTED)
	    throw Xapian::UnimplementedError("This backend doesn't implement transactions");
	throw InvalidOperationError("Cannot commit transaction - no transaction currently in progress");
    }
    bool flushed = (transaction_state == TRANSACTION_FLUSHED);
    transaction_state = TRANSACTION_NONE;
    // N.B. Call flush() after we clear transaction_state since flush()
    // isn't allowing during a transaction.
    if (flushed) flush();
}

void
Database::Internal::cancel_transaction()
{
    if (!transaction_active()) {
	if (transaction_state == TRANSACTION_UNIMPLEMENTED)
	    throw Xapian::UnimplementedError("This backend doesn't implement transactions");
	throw InvalidOperationError("Cannot cancel transaction - no transaction currently in progress");
    }
    transaction_state = TRANSACTION_NONE;
    cancel();
}

Xapian::docid
Database::Internal::add_document(const Xapian::Document &)
{
    // Writable databases should override this method.
    Assert(false);
    return 0;
}

void
Database::Internal::delete_document(Xapian::docid)
{
    // Writable databases should override this method.
    Assert(false);
}

void
Database::Internal::delete_document(const std::string & unique_term)
{
    // Default implementation - overridden for remote databases
    Xapian::Internal::RefCntPtr<LeafPostList> pl(open_post_list(unique_term));
    while (pl->next(), !pl->at_end()) {
	delete_document(pl->get_docid());
    }
}

void
Database::Internal::replace_document(Xapian::docid, const Xapian::Document &)
{
    // Writable databases should override this method.
    Assert(false);
}

Xapian::docid
Database::Internal::replace_document(const std::string & unique_term,
				     const Xapian::Document & document)
{
    // Default implementation - overridden for remote databases
    Xapian::Internal::RefCntPtr<LeafPostList> pl(open_post_list(unique_term));
    pl->next();
    if (pl->at_end()) {
	return add_document(document);
    }
    Xapian::docid did = pl->get_docid();
    replace_document(did, document);
    while (pl->next(), !pl->at_end()) {
	delete_document(pl->get_docid());
    }
    return did;
}

TermList *
Database::Internal::open_spelling_termlist(const string &) const
{
    // Only implemented for some database backends - others will just not
    // suggest spelling corrections (or not contribute to them in a multiple
    // database situation).
    return NULL;
}

TermList *
Database::Internal::open_spelling_wordlist() const
{
    // Only implemented for some database backends - others will just not
    // suggest spelling corrections (or not contribute to them in a multiple
    // database situation).
    return NULL;
}

Xapian::doccount
Database::Internal::get_spelling_frequency(const string &) const
{
    // Only implemented for some database backends - others will just not
    // suggest spelling corrections (or not contribute to them in a multiple
    // database situation).
    return 0;
}

void
Database::Internal::add_spelling(const string &, Xapian::termcount) const
{
    throw Xapian::UnimplementedError("This backend doesn't implement spelling correction");
}

void
Database::Internal::remove_spelling(const string &, Xapian::termcount) const
{
    throw Xapian::UnimplementedError("This backend doesn't implement spelling correction");
}

TermList *
Database::Internal::open_synonym_termlist(const string &) const
{
    // Only implemented for some database backends - others will just not
    // expand synonyms (or not contribute to them in a multiple database
    // situation).
    return NULL;
}

TermList *
Database::Internal::open_synonym_keylist(const string &) const
{
    // Only implemented for some database backends - others will just not
    // expand synonyms (or not contribute to them in a multiple database
    // situation).
    return NULL;
}

void
Database::Internal::add_synonym(const string &, const string &) const
{
    throw Xapian::UnimplementedError("This backend doesn't implement synonyms");
}

void
Database::Internal::remove_synonym(const string &, const string &) const
{
    throw Xapian::UnimplementedError("This backend doesn't implement synonyms");
}

void
Database::Internal::clear_synonyms(const string &) const
{
    throw Xapian::UnimplementedError("This backend doesn't implement synonyms");
}

string
Database::Internal::get_metadata(const string &) const
{
    return string();
}

TermList *
Database::Internal::open_metadata_keylist(const std::string &) const
{
    // Only implemented for some database backends - others will simply report
    // there being no metadata keys.
    return NULL;
}

void
Database::Internal::set_metadata(const string &, const string &)
{
    throw Xapian::UnimplementedError("This backend doesn't implement metadata");
}

void
Database::Internal::reopen()
{
    // Database backends which don't support simultaneous update and reading
    // probably don't need to do anything here.
}

void
Database::Internal::request_document(Xapian::docid /*did*/) const
{
}

Xapian::Document::Internal *
Database::Internal::collect_document(Xapian::docid did) const
{
    return open_document(did);
}

void
Database::Internal::write_changesets_to_fd(int, const std::string &, bool, ReplicationInfo *)
{
    throw Xapian::UnimplementedError("This backend doesn't provide changesets");
}

string
Database::Internal::get_revision_info() const
{
    throw Xapian::UnimplementedError("This backend doesn't provide access to revision information");
}

bool
Database::Internal::check_revision_at_least(const string &, const string &) const
{
    throw Xapian::UnimplementedError("This backend doesn't support comparing revision numbers");
}

string
Database::Internal::apply_changeset_from_conn(RemoteConnection &,
					      const OmTime &)
{
    throw Xapian::UnimplementedError("This backend doesn't support applying changesets");
}

string
Database::Internal::get_uuid() const
{
    throw Xapian::UnimplementedError("This backend doesn't support UUIDs");
}

RemoteDatabase *
Database::Internal::as_remotedatabase()
{
    return NULL;
}

}
