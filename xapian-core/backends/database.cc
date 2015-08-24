/* database.cc: Database::Internal base class.
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009 Olly Betts
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

#include "xapian/error.h"

#include "leafpostlist.h"
#include "omassert.h"
#include "slowvaluelist.h"

#include <algorithm>
#include <string>

using namespace std;

namespace Xapian {

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

string
Database::Internal::get_value_lower_bound(Xapian::valueno) const
{
    return string();
}

string
Database::Internal::get_value_upper_bound(Xapian::valueno) const
{
    throw Xapian::UnimplementedError("This backend doesn't support get_value_upper_bound");
}

Xapian::termcount
Database::Internal::get_doclength_lower_bound() const
{
    // A zero-length document can't contain any terms, so we ignore such
    // documents for the purposes of this lower bound.
    return 1;
}

Xapian::termcount
Database::Internal::get_doclength_upper_bound() const
{
    // Not a very tight bound in general, but this is only a fall-back for
    // backends which don't store these stats.
    return min(get_total_length(), totlen_t(Xapian::termcount(-1)));
}

Xapian::termcount
Database::Internal::get_wdf_upper_bound(const string & term) const
{
    // Not a very tight bound in general, but this is only a fall-back for
    // backends which don't store these stats.
    return get_collection_freq(term);
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
	// N.B. Call commit() before we set transaction_state since commit()
	// isn't allowing during a transaction.
	commit();
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
    // N.B. Call commit() after we clear transaction_state since commit()
    // isn't allowing during a transaction.
    if (flushed) commit();
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
Database::Internal::delete_document(const string & unique_term)
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
Database::Internal::replace_document(const string & unique_term,
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
Database::Internal::open_metadata_keylist(const string &) const
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
    // Open the document lazily - collect document is only called by
    // Enquire::Internal::read_doc() for a given MSetItem, so we know that the
    // document already exists.
    return open_document(did, true);
}

void
Database::Internal::write_changesets_to_fd(int, const string &, bool, ReplicationInfo *)
{
    throw Xapian::UnimplementedError("This backend doesn't provide changesets");
}

string
Database::Internal::get_revision_info() const
{
    throw Xapian::UnimplementedError("This backend doesn't provide access to revision information");
}

string
Database::Internal::get_uuid() const
{
    return string();
}

void
Database::Internal::invalidate_doc_object(Xapian::Document::Internal *) const
{
    // Do nothing, by default.
}

RemoteDatabase *
Database::Internal::as_remotedatabase()
{
    return NULL;
}

}
