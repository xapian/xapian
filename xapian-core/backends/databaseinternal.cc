/** @file databaseinternal.cc
 * @brief Virtual base class for Database internals
 */
/* Copyright 2003,2004,2006,2007,2008,2009,2011,2014,2015,2017,2019 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include "databaseinternal.h"

#include "api/leafpostlist.h"
#include "api/termlist.h"
#include "heap.h"
#include "omassert.h"
#include "slowvaluelist.h"
#include "stringutils.h"
#include "xapian/error.h"

#include <algorithm>
#include <memory>
#include <string>

using namespace std;
using Xapian::Internal::intrusive_ptr;

namespace Xapian {

[[noreturn]]
static void invalid_operation(const char* msg)
{
    throw InvalidOperationError(msg);
}

Database::Internal::size_type
Database::Internal::size() const
{
    return 1;
}

void
Database::Internal::keep_alive()
{
    // No-op except for remote databases.
}

void
Database::Internal::readahead_for_query(const Xapian::Query &) const
{
}

Xapian::termcount
Database::Internal::get_unique_terms_lower_bound() const
{
    return 1;
}

Xapian::termcount
Database::Internal::get_unique_terms_upper_bound() const
{
    return get_doclength_upper_bound();
}

// Discard any exceptions - we're called from the destructors of derived
// classes so we can't safely throw.
void
Database::Internal::dtor_called_()
{
    try {
	if (transaction_active()) {
	    end_transaction(false);
	} else {
	    // TRANSACTION_READONLY and TRANSACTION_UNIMPLEMENTED should be
	    // handled by the inlined dtor_called() wrapper.
	    AssertEq(state, TRANSACTION_NONE);
	    commit();
	}
    } catch (...) {
	// We can't safely throw exceptions from a destructor in case an
	// exception is already active and causing us to be destroyed.
    }
}

void
Database::Internal::commit()
{
    // Writable databases should override this method, but this can get called
    // if a read-only shard gets added to a WritableDatabase.
    invalid_operation("WritableDatabase::commit() called with a read-only shard");
}

void
Database::Internal::cancel()
{
    // Writable databases should override this method, but this can get called
    // if a read-only shard gets added to a WritableDatabase.
    invalid_operation("WritableDatabase::cancel() called with a read-only shard");
}

void
Database::Internal::begin_transaction(bool flushed)
{
    if (state != TRANSACTION_NONE) {
	if (transaction_active()) {
	    invalid_operation("WritableDatabase::begin_transaction(): already "
			      "in a transaction");
	}
	if (is_read_only()) {
	    invalid_operation("WritableDatabase::begin_transaction(): called "
			      "with a read-only shard");
	}
	throw UnimplementedError("This backend doesn't implement transactions");
    }
    if (flushed) {
	// N.B. Call commit() before we set state since commit() isn't allowed
	// during a transaction.
	commit();
	state = TRANSACTION_FLUSHED;
    } else {
	state = TRANSACTION_UNFLUSHED;
    }
}

void
Database::Internal::end_transaction(bool do_commit)
{
    if (!transaction_active()) {
	if (state != TRANSACTION_NONE) {
	    if (is_read_only()) {
		invalid_operation(do_commit ?
				  "WritableDatabase::commit_transaction(): "
				  "called with a read-only shard" :
				  "WritableDatabase::cancel_transaction(): "
				  "called with a read-only shard");
	    }
	    throw UnimplementedError("This backend doesn't implement transactions");
	}
	invalid_operation(do_commit ?
			  "WritableDatabase::commit_transaction(): not in a "
			  "transaction" :
			  "WritableDatabase::cancel_transaction(): not in a "
			  "transaction");
    }

    auto old_state = state;
    state = TRANSACTION_NONE;
    if (!do_commit) {
	cancel();
    } else if (old_state == TRANSACTION_FLUSHED) {
	// N.B. Call commit() after we clear state since commit() isn't
	// allowed during a transaction.
	commit();
    }
}

Xapian::docid
Database::Internal::add_document(const Xapian::Document &)
{
    // Writable databases should override this method, but this can get called
    // if a read-only shard gets added to a WritableDatabase.
    invalid_operation("WritableDatabase::add_document() called with a "
		      "read-only shard");
}

void
Database::Internal::delete_document(Xapian::docid)
{
    // Writable databases should override this method, but this can get called
    // if a read-only shard gets added to a WritableDatabase.
    invalid_operation("WritableDatabase::delete_document() called with a "
		      "read-only shard");
}

void
Database::Internal::delete_document(const string& unique_term)
{
    // Default implementation - overridden for remote and sharded databases.

    if (is_read_only()) {
	// This can happen if a read-only shard gets added to a
	// WritableDatabase.
	invalid_operation("WritableDatabase::delete_document() called with a "
			  "read-only shard");
    }

    unique_ptr<PostList> pl(open_post_list(unique_term));

    // We want this operation to be atomic if possible, so if we aren't in a
    // transaction and the backend supports transactions, temporarily enter an
    // unflushed transaction.
    auto old_state = state;
    if (state != TRANSACTION_UNIMPLEMENTED)
	state = TRANSACTION_UNFLUSHED;
    try {
	while (pl->next(), !pl->at_end()) {
	    delete_document(pl->get_docid());
	}
    } catch (...) {
	state = old_state;
	throw;
    }
    state = old_state;
}

void
Database::Internal::replace_document(Xapian::docid, const Xapian::Document &)
{
    // Writable databases should override this method, but this can get called
    // if a read-only shard gets added to a WritableDatabase.
    invalid_operation("WritableDatabase::replace_document() called with a "
		      "read-only shard");
}

Xapian::docid
Database::Internal::replace_document(const string & unique_term,
				     const Xapian::Document & document)
{
    // Default implementation - overridden for remote and sharded databases.

    if (is_read_only()) {
	// This can happen if a read-only shard gets added to a
	// WritableDatabase.
	invalid_operation("WritableDatabase::replace_document() called with a "
			  "read-only shard");
    }

    unique_ptr<PostList> pl(open_post_list(unique_term));
    pl->next();
    if (pl->at_end()) {
	return add_document(document);
    }
    Xapian::docid did = pl->get_docid();

    // We want this operation to be atomic if possible, so if we aren't in a
    // transaction and the backend supports transactions, temporarily enter an
    // unflushed transaction.
    auto old_state = state;
    if (state != TRANSACTION_UNIMPLEMENTED)
	state = TRANSACTION_UNFLUSHED;
    try {
	replace_document(did, document);
	while (pl->next(), !pl->at_end()) {
	    delete_document(pl->get_docid());
	}
    } catch (...) {
	state = old_state;
	throw;
    }
    state = old_state;
    return did;
}

ValueList *
Database::Internal::open_value_list(Xapian::valueno slot) const
{
    return new SlowValueList(this, slot);
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

Xapian::termcount
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

TermList*
Database::Internal::open_metadata_keylist(const string&) const
{
    // Only implemented for some database backends - others will simply report
    // there being no metadata keys.
    return NULL;
}

void
Database::Internal::set_metadata(const string&, const string&)
{
    throw Xapian::UnimplementedError("This backend doesn't implement metadata");
}

bool
Database::Internal::reopen()
{
    // Database backends which don't support simultaneous update and reading
    // probably don't need to do anything here.  And since we didn't do
    // anything we should return false to indicate that nothing has changed.
    return false;
}

void
Database::Internal::request_document(Xapian::docid) const
{
}

void
Database::Internal::write_changesets_to_fd(int, const string&, bool, ReplicationInfo*)
{
    throw Xapian::UnimplementedError("This backend doesn't provide changesets");
}

Xapian::rev
Database::Internal::get_revision() const
{
    throw Xapian::UnimplementedError("This backend doesn't provide access to revision information");
}

string
Database::Internal::get_uuid() const
{
    return string();
}

void
Database::Internal::invalidate_doc_object(Xapian::Document::Internal*) const
{
    // Do nothing, by default.
}

void
Database::Internal::get_used_docid_range(Xapian::docid &,
					 Xapian::docid &) const
{
    throw Xapian::UnimplementedError("This backend doesn't implement get_used_docid_range()");
}

bool
Database::Internal::locked() const
{
    return false;
}

Database::Internal*
Database::Internal::update_lock(int flags)
{
    if (flags == Xapian::DB_READONLY_) return this;
    throw Xapian::DatabaseLockError("Not possible to lock for writing");
}

namespace {
    class Pos {
	Xapian::termpos pos;

	PositionList* p;

	string term;

      public:
	Pos(string&& term_, PositionList* p_)
	    : p(p_), term(term_) {
	    pos = p->get_position();
	}

	~Pos() { delete p; }

	Xapian::termpos get_pos() const { return pos; }

	const string& get_term() const { return term; }

	bool next() {
	    if (!p->next()) {
		return false;
	    }
	    pos = p->get_position();
	    return true;
	}
    };
}

static void
reconstruct_open_poslists(TermList* termlist,
			  Xapian::termpos start_pos,
			  Xapian::termpos end_pos,
			  const string& end,
			  vector<unique_ptr<Pos>>& heap,
			  size_t prefix_size = 0)
{
    constexpr Xapian::termpos LAST_POS = Xapian::termpos(-1);
    while (!termlist->at_end()) {
	const string& term = termlist->get_termname();
	if (!end.empty() && term >= end) {
	    break;
	}
	PositionList* poslist = termlist->positionlist_begin();
	if ((start_pos ? poslist->skip_to(start_pos) : poslist->next()) &&
	    (end_pos == LAST_POS || poslist->get_position() <= end_pos)) {
	    heap.emplace_back(new Pos(term.substr(prefix_size), poslist));
	} else {
	    delete poslist;
	}
	termlist->next();
    }
}

string
Database::Internal::reconstruct_text(Xapian::docid did,
				     size_t length,
				     const std::string& prefix,
				     Xapian::termpos start_pos,
				     Xapian::termpos end_pos) const
{
    if (end_pos == 0) {
	// Wrap to largest possible value.
	--end_pos;
    }

    if (length == 0) {
	// Wrap to largest possible value.
	--length;
    }

    struct PosCmp {
	bool operator()(const unique_ptr<Pos>& a, const unique_ptr<Pos>& b) {
	    if (a->get_pos() != b->get_pos()) {
		return a->get_pos() > b->get_pos();
	    }
	    return a->get_term() > b->get_term();
	}
    };

    vector<unique_ptr<Pos>> heap;

    unique_ptr<TermList> termlist(open_term_list_direct(did));
    if (usual(termlist.get())) {
	if (prefix.empty()) {
	    termlist->next();
	    reconstruct_open_poslists(termlist.get(), start_pos, end_pos,
				      "A", heap);
	    termlist->skip_to("[");
	    reconstruct_open_poslists(termlist.get(), start_pos, end_pos,
				      prefix, heap);
	} else {
	    termlist->skip_to(prefix);
	    if (!termlist->at_end()) {
		// Calculate the first possible term without the specified
		// prefix.
		string term_ub = prefix;
		size_t i = term_ub.find_last_not_of('\xff');
		term_ub.resize(i + 1);
		if (i != string::npos) {
		    term_ub[i] = (unsigned char)term_ub[i] + 1;
		}
		reconstruct_open_poslists(termlist.get(), start_pos, end_pos,
					  term_ub, heap, prefix.size());
	    }
	}

	Heap::make(heap.begin(), heap.end(), PosCmp());
    }

    string result;

    Xapian::termpos old_pos = start_pos - 1;
    while (!heap.empty()) {
	Pos* tip = heap.front().get();
	Xapian::termpos pos = tip->get_pos();
	if (pos > end_pos) break;

	Xapian::termpos delta = pos - old_pos;
	// Ignore additional terms at the same position.
	if (delta) {
	    if (usual(!result.empty())) {
		// Insert newline for gap in used positions.
		result += (delta == 1 ? ' ' : '\n');
	    }
	    result += tip->get_term();
	}

	if (result.size() >= length) break;

	old_pos = pos;

	if (tip->next()) {
	    Heap::replace(heap.begin(), heap.end(), PosCmp());
	} else {
	    Heap::pop(heap.begin(), heap.end(), PosCmp());
	    heap.resize(heap.size() - 1);
	}
    }

    return result;
}

}
