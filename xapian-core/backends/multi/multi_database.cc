/** @file multi_database.cc
 * @brief Sharded database backend
 */
/* Copyright (C) 2017,2019 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include "multi_database.h"

#include "backends/backends.h"
#include "backends/multi.h"
#include "expand/ortermlist.h"
#include "expand/termlistmerger.h"
#include "multi_alltermslist.h"
#include "multi_postlist.h"
#include "multi_termlist.h"
#include "multi_valuelist.h"

#include <memory>

using namespace std;

MultiDatabase::size_type
MultiDatabase::size() const
{
    return shards.size();
}

bool
MultiDatabase::reopen()
{
    bool result = false;
    for (auto&& shard : shards) {
	if (shard->reopen()) {
	    result = true;
	}
    }
    return result;
}

void
MultiDatabase::close()
{
    for (auto&& shard : shards) {
	shard->close();
    }
}

PostList*
MultiDatabase::open_post_list(const string& term) const
{
    PostList** postlists = new PostList*[shards.size()];
    size_t count = 0;
    try {
	for (auto&& shard : shards) {
	    postlists[count] = shard->open_post_list(term);
	    ++count;
	}
	return new MultiPostList(count, postlists);
    } catch (...) {
	while (count)
	    delete postlists[--count];
	delete [] postlists;
	throw;
    }
}

LeafPostList*
MultiDatabase::open_leaf_post_list(const string&, bool) const
{
    // This should never get called.
    Assert(false);
    return NULL;
}

TermList*
MultiDatabase::open_term_list(Xapian::docid did) const
{
    return new MultiTermList(this, MultiDatabase::open_term_list_direct(did));
}

TermList*
MultiDatabase::open_term_list_direct(Xapian::docid did) const
{
    size_t n_shards = shards.size();
    auto shard = shards[shard_number(did, n_shards)];
    Xapian::docid shard_did = shard_docid(did, n_shards);
    return shard->open_term_list(shard_did);
}

TermList*
MultiDatabase::open_allterms(const string& prefix) const
{
    size_t count = 0;
    TermList** termlists = new TermList*[shards.size()];
    try {
	for (auto&& shard : shards) {
	    termlists[count] = shard->open_allterms(prefix);
	    ++count;
	}
	return new MultiAllTermsList(count, termlists);
    } catch (...) {
	while (count)
	    delete termlists[--count];
	delete [] termlists;
	throw;
    }
}

bool
MultiDatabase::has_positions() const
{
    for (auto&& shard : shards) {
	if (shard->has_positions()) {
	    return true;
	}
    }
    return false;
}

PositionList*
MultiDatabase::open_position_list(Xapian::docid did, const string& term) const
{
    auto n_shards = shards.size();
    auto shard = shards[shard_number(did, n_shards)];
    auto shard_did = shard_docid(did, n_shards);
    return shard->open_position_list(shard_did, term);
}

Xapian::doccount
MultiDatabase::get_doccount() const
{
    Xapian::doccount result = 0;
    for (auto&& shard : shards) {
	auto old_result = result;
	result += shard->get_doccount();
	if (result < old_result)
	    throw Xapian::DatabaseError("doccount overflowed!");
    }
    return result;
}

Xapian::docid
MultiDatabase::get_lastdocid() const
{
    Xapian::docid result = 0;
    auto n_shards = shards.size();
    for (size_t shard = 0; shard != n_shards; ++shard) {
	Xapian::docid shard_lastdocid = shards[shard]->get_lastdocid();
	if (shard_lastdocid == 0) {
	    // This shard is empty, so doesn't influence lastdocid for the
	    // combined database.
	    continue;
	}
	result = max(result, unshard(shard_lastdocid, shard, n_shards));
    }
    return result;
}

Xapian::totallength
MultiDatabase::get_total_length() const
{
    Xapian::totallength result = 0;
    for (auto&& shard : shards) {
	auto old_result = result;
	result += shard->get_total_length();
	if (result < old_result)
	    throw Xapian::DatabaseError("Total document length overflowed!");
    }
    return result;
}

void
MultiDatabase::get_freqs(const string& term,
			 Xapian::doccount* tf_ptr,
			 Xapian::termcount* cf_ptr) const
{
    Assert(!term.empty());

    Xapian::doccount shard_tf;
    Xapian::doccount* shard_tf_ptr = tf_ptr ? &shard_tf : NULL;
    Xapian::doccount total_tf = 0;

    Xapian::termcount shard_cf;
    Xapian::termcount* shard_cf_ptr = cf_ptr ? &shard_cf : NULL;
    Xapian::termcount total_cf = 0;

    for (auto&& shard : shards) {
	shard->get_freqs(term, shard_tf_ptr, shard_cf_ptr);
	if (shard_tf_ptr) {
	    auto old_tf = total_tf;
	    total_tf += *shard_tf_ptr;
	    if (total_tf < old_tf)
		throw Xapian::DatabaseError("termfreq overflowed!");
	}
	if (shard_cf_ptr) {
	    auto old_cf = total_cf;
	    total_cf += *shard_cf_ptr;
	    if (total_cf < old_cf)
		throw Xapian::DatabaseError("Collection freq overflowed!");
	}
    }
    if (tf_ptr) {
	*tf_ptr = total_tf;
    }
    if (cf_ptr) {
	*cf_ptr = total_cf;
    }
}

Xapian::doccount
MultiDatabase::get_value_freq(Xapian::valueno slot) const
{
    Xapian::termcount result = 0;
    for (auto&& shard : shards) {
	auto old_result = result;
	result += shard->get_value_freq(slot);
	if (result < old_result)
	    throw Xapian::DatabaseError("Value freq overflowed!");
    }
    return result;
}

string
MultiDatabase::get_value_lower_bound(Xapian::valueno slot) const
{
    string result;
    for (auto&& shard : shards) {
	string shard_result = shard->get_value_lower_bound(slot);
	if (shard_result.empty())
	    continue;
	if (result.empty() || shard_result < result)
	    result = std::move(shard_result);
    }
    return result;
}

string
MultiDatabase::get_value_upper_bound(Xapian::valueno slot) const
{
    string result;
    for (auto&& shard : shards) {
	string shard_result = shard->get_value_upper_bound(slot);
	if (shard_result > result)
	    result = std::move(shard_result);
    }
    return result;
}

Xapian::termcount
MultiDatabase::get_doclength_lower_bound() const
{
    // We want the smallest answer from amongst the shards, except that 0 means
    // that all documents have length 0 (including the special case of there
    // being no documents), so any non-zero answer should "beat" 0.  To achieve
    // this we find the *maximum* after negating each of the values (which
    // since Xapian::termcount is an unsigned type leaves 0 alone but flips the
    // order of all other values), then negate the answer again at the end.
    static_assert(std::is_unsigned<Xapian::termcount>::value,
		  "Unsigned type required");
    Xapian::termcount result = 0;
    for (auto&& shard : shards) {
	Xapian::termcount shard_result = -shard->get_doclength_lower_bound();
	result = max(result, shard_result);
    }
    return -result;
}

Xapian::termcount
MultiDatabase::get_doclength_upper_bound() const
{
    Xapian::termcount result = 0;
    for (auto&& shard : shards) {
	result = max(result, shard->get_doclength_upper_bound());
    }
    return result;
}

Xapian::termcount
MultiDatabase::get_wdf_upper_bound(const string& term) const
{
    Assert(!term.empty());

    Xapian::termcount result = 0;
    for (auto&& shard : shards) {
	result = max(result, shard->get_wdf_upper_bound(term));
    }
    return result;
}

ValueList*
MultiDatabase::open_value_list(Xapian::valueno slot) const
{
    SubValueList** valuelists = new SubValueList*[shards.size()];
    size_t count = 0;
    try {
	for (auto&& shard : shards) {
	    ValueList* vl = shard->open_value_list(slot);
	    valuelists[count] = new SubValueList(vl, count);
	    ++count;
	}
	return new MultiValueList(count, valuelists, slot);
    } catch (...) {
	while (count)
	    delete valuelists[--count];
	delete [] valuelists;
	throw;
    }
}

Xapian::termcount
MultiDatabase::get_doclength(Xapian::docid did) const
{
    Assert(did != 0);

    auto n_shards = shards.size();
    auto shard = shards[shard_number(did, n_shards)];
    auto shard_did = shard_docid(did, n_shards);
    return shard->get_doclength(shard_did);
}

Xapian::termcount
MultiDatabase::get_unique_terms(Xapian::docid did) const
{
    Assert(did != 0);

    auto n_shards = shards.size();
    auto shard = shards[shard_number(did, n_shards)];
    auto shard_did = shard_docid(did, n_shards);
    return shard->get_unique_terms(shard_did);
}

Xapian::Document::Internal*
MultiDatabase::open_document(Xapian::docid did, bool lazy) const
{
    Assert(did != 0);

    auto n_shards = shards.size();
    auto shard = shards[shard_number(did, n_shards)];
    auto shard_did = shard_docid(did, n_shards);
    return shard->open_document(shard_did, lazy);
}

bool
MultiDatabase::term_exists(const string& term) const
{
    for (auto&& shard : shards) {
	if (shard->term_exists(term))
	    return true;
    }
    return false;
}

void
MultiDatabase::keep_alive()
{
    for (auto&& shard : shards) {
	shard->keep_alive();
    }
}

TermList*
MultiDatabase::open_spelling_termlist(const string& word) const
{
    vector<TermList*> termlists;
    termlists.reserve(shards.size());

    try {
	for (auto&& shard : shards) {
	    TermList* termlist = shard->open_spelling_termlist(word);
	    if (!termlist)
		continue;
	    termlists.push_back(termlist);
	}

	return make_termlist_merger(termlists);
    } catch (...) {
	for (auto&& termlist : termlists)
	    delete termlist;
	throw;
    }
}

TermList*
MultiDatabase::open_spelling_wordlist() const
{
    vector<TermList*> termlists;
    termlists.reserve(shards.size());

    try {
	for (auto&& shard : shards) {
	    TermList* termlist = shard->open_spelling_wordlist();
	    if (!termlist)
		continue;
	    termlists.push_back(termlist);
	}

	return make_termlist_merger<FreqAdderOrTermList>(termlists);
    } catch (...) {
	for (auto&& termlist : termlists)
	    delete termlist;
	throw;
    }
}

Xapian::doccount
MultiDatabase::get_spelling_frequency(const string& word) const
{
    Xapian::doccount result = 0;
    for (auto&& shard : shards) {
	auto old_result = result;
	result += shard->get_spelling_frequency(word);
	if (result < old_result)
	    throw Xapian::DatabaseError("Spelling frequency overflowed!");
    }
    return result;
}

TermList*
MultiDatabase::open_synonym_termlist(const string& term) const
{
    vector<TermList*> termlists;
    termlists.reserve(shards.size());

    try {
	for (auto&& shard : shards) {
	    TermList* termlist = shard->open_synonym_termlist(term);
	    if (!termlist)
		continue;
	    termlists.push_back(termlist);
	}

	return make_termlist_merger(termlists);
    } catch (...) {
	for (auto&& termlist : termlists)
	    delete termlist;
	throw;
    }
}

TermList*
MultiDatabase::open_synonym_keylist(const string& prefix) const
{
    vector<TermList*> termlists;
    termlists.reserve(shards.size());

    try {
	for (auto&& shard : shards) {
	    TermList* termlist = shard->open_synonym_keylist(prefix);
	    if (!termlist)
		continue;
	    termlists.push_back(termlist);
	}

	return make_termlist_merger(termlists);
    } catch (...) {
	for (auto&& termlist : termlists)
	    delete termlist;
	throw;
    }
}

string
MultiDatabase::get_metadata(const string& key) const
{
    return shards[0]->get_metadata(key);
}

TermList*
MultiDatabase::open_metadata_keylist(const string& prefix) const
{
    return shards[0]->open_metadata_keylist(prefix);
}

string
MultiDatabase::get_uuid() const
{
    string uuid;
    for (auto&& shard : shards) {
	const string& sub_uuid = shard->get_uuid();
	// If any of the sub-databases have no uuid, we can't make a uuid for
	// the combined database.
	if (sub_uuid.empty())
	    return sub_uuid;
	if (!uuid.empty())
	    uuid += ':';
	uuid += sub_uuid;
    }
    return uuid;
}

bool
MultiDatabase::locked() const
{
    for (auto&& shard : shards) {
	if (shard->locked()) {
	    return true;
	}
    }
    return false;
}

void
MultiDatabase::write_changesets_to_fd(int,
				      const std::string&,
				      bool,
				      Xapian::ReplicationInfo*)
{
    throw Xapian::InvalidOperationError("write_changesets_to_fd() with "
					"more than one subdatabase");
}

Xapian::rev
MultiDatabase::get_revision() const
{
    throw Xapian::InvalidOperationError("Database::get_revision() with "
					"more than one subdatabase");
}

void
MultiDatabase::invalidate_doc_object(Xapian::Document::Internal*) const
{
    // This method should only be called on a single shard.
    Assert(false);
}

int
MultiDatabase::get_backend_info(string*) const
{
    // This method should only be called on a single shard.
    Assert(false);
    return BACKEND_UNKNOWN;
}

void
MultiDatabase::commit()
{
    for (auto&& shard : shards) {
	shard->commit();
    }
}

void
MultiDatabase::cancel()
{
    for (auto&& shard : shards) {
	shard->cancel();
    }
}

void
MultiDatabase::begin_transaction(bool flushed)
{
    for (auto&& shard : shards) {
	shard->begin_transaction(flushed);
    }
}

void
MultiDatabase::end_transaction_(bool do_commit)
{
    for (auto&& shard : shards) {
	shard->end_transaction(do_commit);
    }
}

Xapian::docid
MultiDatabase::add_document(const Xapian::Document& doc)
{
    // With a single shard, add_document() uses docid (get_lastdocid() + 1)
    // which seems a sensible invariant to preserve with multiple shards.
    Xapian::docid did = get_lastdocid() + 1;
    if (rare(did == 0)) {
	throw Xapian::DatabaseError("Run out of docids - you'll have to use "
				    "copydatabase to eliminate any gaps "
				    "before you can add more documents");
    }

    auto n_shards = shards.size();
    auto shard = shards[shard_number(did, n_shards)];
    shard->replace_document(shard_docid(did, n_shards), doc);
    return did;
}

void
MultiDatabase::delete_document(Xapian::docid did)
{
    auto n_shards = shards.size();
    auto shard = shards[shard_number(did, n_shards)];
    shard->delete_document(shard_docid(did, n_shards));
}

void
MultiDatabase::delete_document(const string& term)
{
    for (auto&& shard : shards) {
	shard->delete_document(term);
    }
}

void
MultiDatabase::replace_document(Xapian::docid did, const Xapian::Document& doc)
{
    auto n_shards = shards.size();
    auto shard = shards[shard_number(did, n_shards)];
    shard->replace_document(shard_docid(did, n_shards), doc);
}

Xapian::docid
MultiDatabase::replace_document(const string& term, const Xapian::Document& doc)
{
    auto n_shards = shards.size();
    unique_ptr<PostList> pl(open_post_list(term));
    pl->next();
    // If no unique_term in the database, this is just an add_document().
    if (pl->at_end()) {
	// Which database will the next never used docid be in?
	Xapian::docid did = get_lastdocid() + 1;
	if (rare(did == 0)) {
	    throw Xapian::DatabaseError("Run out of docids - you'll have to "
					"use copydatabase to eliminate any "
					"gaps before you can add more "
					"documents");
	}
	auto shard = shards[shard_number(did, n_shards)];
	return shard->add_document(doc);
    }

    Xapian::docid result = pl->get_docid();
    auto replacing_shard = shards[shard_number(result, n_shards)];
    replacing_shard->replace_document(shard_docid(result, n_shards), doc);

    // Delete any other occurrences of the unique term.
    while (pl->next(), !pl->at_end()) {
	Xapian::docid did = pl->get_docid();
	auto shard = shards[shard_number(did, n_shards)];
	shard->delete_document(shard_docid(did, n_shards));
    }

    return result;
}

void
MultiDatabase::request_document(Xapian::docid did) const
{
    Assert(did != 0);

    auto n_shards = shards.size();
    auto shard = shards[shard_number(did, n_shards)];
    auto shard_did = shard_docid(did, n_shards);
    shard->request_document(shard_did);
}

void
MultiDatabase::add_spelling(const string& word,
			    Xapian::termcount freqinc) const
{
    shards[0]->add_spelling(word, freqinc);
}

Xapian::termcount
MultiDatabase::remove_spelling(const string& word,
			       Xapian::termcount freqdec) const
{
    for (auto&& shard : shards) {
	freqdec = shard->remove_spelling(word, freqdec);
	if (freqdec == 0)
	    break;
    }
    return freqdec;
}

void
MultiDatabase::add_synonym(const string& term,
			   const string& synonym) const
{
    shards[0]->add_synonym(term, synonym);
}

void
MultiDatabase::remove_synonym(const string& term,
			      const string& synonym) const
{
    for (auto&& shard : shards) {
	shard->remove_synonym(term, synonym);
    }
}

void
MultiDatabase::clear_synonyms(const string& term) const
{
    for (auto&& shard : shards) {
	shard->clear_synonyms(term);
    }
}

void
MultiDatabase::set_metadata(const string& key, const string& value)
{
    shards[0]->set_metadata(key, value);
}

string
MultiDatabase::reconstruct_text(Xapian::docid did,
				size_t length,
				const string& prefix,
				Xapian::termpos start_pos,
				Xapian::termpos end_pos) const
{
    Assert(did != 0);

    auto n_shards = shards.size();
    auto shard = shards[shard_number(did, n_shards)];
    auto shard_did = shard_docid(did, n_shards);
    return shard->reconstruct_text(shard_did, length, prefix,
				   start_pos, end_pos);
}

string
MultiDatabase::get_description() const
{
    string desc;
    for (auto&& shard : shards) {
	if (!desc.empty()) {
	    desc += ", ";
	}
	desc += shard->get_description();
    }
    desc += ')';
    return desc;
}
