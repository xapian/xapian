/** @file database.cc
 * @brief Database API class
 */
/* Copyright 2006,2007,2008,2009,2010,2011,2013,2014,2015,2016,2017,2019 Olly Betts
 * Copyright 2007,2008,2009 Lemur Consulting Ltd
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

#include <xapian/database.h>

#include "backends/databaseinternal.h"
#include "backends/empty_database.h"
#include "backends/multi/multi_database.h"
#include "debuglog.h"
#include "editdistance.h"
#include "omassert.h"
#include "postingiteratorinternal.h"
#include <xapian/constants.h>
#include <xapian/error.h>
#include <xapian/positioniterator.h>
#include <xapian/postingiterator.h>
#include <xapian/termiterator.h>
#include <xapian/unicode.h>

#include <algorithm>
#include <cstdlib> // For abs().
#include <memory>
#include <string>
#include <vector>

using namespace std;

[[noreturn]]
static void docid_zero_invalid()
{
    throw Xapian::InvalidArgumentError("Document ID 0 is invalid");
}

[[noreturn]]
static void empty_metadata_key()
{
    throw Xapian::InvalidArgumentError("Empty metadata keys are invalid");
}

[[noreturn]]
static void empty_term_invalid()
{
    throw Xapian::InvalidArgumentError("Empty terms are invalid");
}

namespace Xapian {

Database::Database(Database::Internal* internal_)
    : internal(internal_)
{
}

Database::Database(const Database&) = default;

Database&
Database::operator=(const Database&) = default;

Database::Database(Database&&) = default;

Database&
Database::operator=(Database&&) = default;

Database::Database()
    : internal(new EmptyDatabase)
{
}

Database::~Database()
{
}

bool
Database::reopen()
{
    return internal->reopen();
}

void
Database::close()
{
    internal->close();
}

size_t
Database::size() const
{
    return internal->size();
}

void
Database::add_database_(const Database& o, bool read_only)
{
    if (this == &o) {
	const char* msg = read_only ?
	    "Database::add_database(): Can't add a Database to itself" :
	    "WritableDatabase::add_database(): Can't add a WritableDatabase "
	    "to itself";
	throw InvalidArgumentError(msg);
    }

    auto o_size = o.internal->size();
    if (o_size == 0) {
	// Adding an empty database is a no-op.
	return;
    }

    auto my_size = internal->size();
    if (my_size == 0 && o_size == 1) {
	// Just copy.
	internal = o.internal;
	return;
    }

#if 0
    // The check below doesn't work - for example:
    //
    // Database db;
    // db.add_database(WritableDatabase("one.db"));
    // db.add_database(WritableDatabase("two.db"));
    //
    // The first add_database() assigns the internal across, so at the second
    // call internal->is_read_only() returns false but read_only is true.
    //
    // I'm not entirely convinced the extra complexity required to make this
    // work is worthwhile.  We catch static violations such as this at compile
    // time:
    //
    // WritableDatabase db;
    // db.add_database(Database("one.db"));
    //
    // The case we don't catch at compile time is:
    //
    // WritableDatabase db;
    // Database ro_db = db;
    // ro_db.add_database(Database("one.db"));
    //
    // But performing WritableDatabase actions using such a WritableDatabase
    // should now throw InvalidOperationError.
    if (!internal->is_read_only() && read_only) {
	throw InvalidArgumentError("Database::add_database(): Can't add a "
				   "Database to a WritableDatabase");
    }
#endif

    // Make sure internal is a MultiDatabase with enough space reserved.
    auto new_size = my_size + o_size;
    MultiDatabase* multi_db;
    if (my_size <= 1) {
	multi_db = new MultiDatabase(new_size, read_only);
	if (my_size) multi_db->push_back(internal.get());
	internal = multi_db;
    } else {
	// Must already be a MultiDatabase as everything else reports 1 for
	// size().
	multi_db = static_cast<MultiDatabase*>(internal.get());
	multi_db->reserve(new_size);
    }

    if (o_size == 1) {
	multi_db->push_back(o.internal.get());
    } else {
	// Must be a MultiDatabase.
	auto o_multi = static_cast<MultiDatabase*>(o.internal.get());
	// Add the shards from o to ourself.
	for (auto&& shard : o_multi->shards) {
	    multi_db->push_back(shard);
	}
    }
}

PostingIterator
Database::postlist_begin(const string& term) const
{
    PostList* pl = internal->open_post_list(term);
    if (!pl) return PostingIterator();
    return PostingIterator(new PostingIterator::Internal(pl, *this));
}

TermIterator
Database::termlist_begin(Xapian::docid did) const
{
    if (did == 0)
	docid_zero_invalid();

    return TermIterator(internal->open_term_list(did));
}

TermIterator
Database::allterms_begin(const string& prefix) const
{
    return TermIterator(internal->open_allterms(prefix));
}

bool
Database::has_positions() const
{
    return internal->has_positions();
}

PositionIterator
Database::positionlist_begin(Xapian::docid did, const string& term) const
{
    if (did == 0)
	docid_zero_invalid();

    if (term.empty())
	empty_term_invalid();

    return PositionIterator(internal->open_position_list(did, term));
}

Xapian::doccount
Database::get_doccount() const
{
    return internal->get_doccount();
}

Xapian::docid
Database::get_lastdocid() const
{
    return internal->get_lastdocid();
}

double
Database::get_average_length() const
{
    Xapian::doccount doc_count = internal->get_doccount();
    if (rare(doc_count == 0))
	return 0.0;

    Xapian::totallength total_length = internal->get_total_length();
    return total_length / double(doc_count);
}

Xapian::totallength
Database::get_total_length() const
{
    return internal->get_total_length();
}

Xapian::doccount
Database::get_termfreq(const string& term) const
{
    if (term.empty())
	return get_doccount();

    Xapian::doccount result;
    internal->get_freqs(term, &result, NULL);
    return result;
}

Xapian::termcount
Database::get_collection_freq(const string& term) const
{
    if (term.empty())
	return get_doccount();

    Xapian::termcount result;
    internal->get_freqs(term, NULL, &result);
    return result;
}

Xapian::doccount
Database::get_value_freq(Xapian::valueno slot) const
{
    return internal->get_value_freq(slot);
}

string
Database::get_value_lower_bound(Xapian::valueno slot) const
{
    return internal->get_value_lower_bound(slot);
}

string
Database::get_value_upper_bound(Xapian::valueno slot) const
{
    return internal->get_value_upper_bound(slot);
}

Xapian::termcount
Database::get_doclength_lower_bound() const
{
    return internal->get_doclength_lower_bound();
}

Xapian::termcount
Database::get_doclength_upper_bound() const
{
    return internal->get_doclength_upper_bound();
}

Xapian::termcount
Database::get_wdf_upper_bound(const string& term) const
{
    if (term.empty())
	return 0;

    return internal->get_wdf_upper_bound(term);
}

Xapian::termcount
Database::get_unique_terms_lower_bound() const
{
    return internal->get_unique_terms_lower_bound();
}

Xapian::termcount
Database::get_unique_terms_upper_bound() const
{
    return internal->get_unique_terms_upper_bound();
}

ValueIterator
Database::valuestream_begin(Xapian::valueno slot) const
{
    return ValueIterator(internal->open_value_list(slot));
}

Xapian::termcount
Database::get_doclength(Xapian::docid did) const
{
    if (did == 0)
	docid_zero_invalid();

    return internal->get_doclength(did);
}

Xapian::termcount
Database::get_unique_terms(Xapian::docid did) const
{
    if (did == 0)
	docid_zero_invalid();

    return internal->get_unique_terms(did);
}

Document
Database::get_document(Xapian::docid did, unsigned flags) const
{
    if (rare(did == 0))
	docid_zero_invalid();

    bool assume_valid = flags & Xapian::DOC_ASSUME_VALID;
    return Document(internal->open_document(did, assume_valid));
}

bool
Database::term_exists(const string& term) const
{
    // NB Internal::term_exists() handles term.empty().
    return internal->term_exists(term);
}

void
Database::keep_alive()
{
    internal->keep_alive();
}

string
Database::get_description() const
{
    string desc = "Database(";
    desc += internal->get_description();
    desc += ')';
    return desc;
}

// Word must have a trigram score at least this close to the best score seen
// so far.
#define TRIGRAM_SCORE_THRESHOLD 2

string
Database::get_spelling_suggestion(const string& word,
				  unsigned max_edit_distance) const
{
    if (word.size() <= 1)
	return string();

    unique_ptr<TermList> merger(internal->open_spelling_termlist(word));
    if (!merger.get())
	return string();

    EditDistanceCalculator edcalc(word);
    Xapian::termcount best = 1;
    string result;
    int edist_best = max_edit_distance;
    Xapian::doccount freq_best = 0;
    Xapian::doccount freq_exact = 0;
    while (true) {
	TermList* ret = merger->next();
	if (ret) merger.reset(ret);

	if (merger->at_end()) break;

	string term = merger->get_termname();
	Xapian::termcount score = merger->get_wdf();

	LOGVALUE(SPELLING, term);
	LOGVALUE(SPELLING, score);
	if (score + TRIGRAM_SCORE_THRESHOLD >= best) {
	    if (score > best) best = score;

	    int edist = edcalc(term, edist_best);
	    LOGVALUE(SPELLING, edist);

	    if (edist <= edist_best) {
		Xapian::doccount freq = internal->get_spelling_frequency(term);

		LOGVALUE(SPELLING, freq);
		LOGVALUE(SPELLING, freq_best);
		// Even if we have an exact match, there may be a much more
		// frequent potential correction which will still be
		// interesting.
		if (edist == 0) {
		    freq_exact = freq;
		    continue;
		}

		if (edist < edist_best || freq > freq_best) {
		    LOGLINE(SPELLING, "Best so far: \"" << term <<
				      "\" edist " << edist << " freq " << freq);
		    result = term;
		    edist_best = edist;
		    freq_best = freq;
		}
	    }
	}
    }
    if (freq_best < freq_exact)
	return string();
    return result;
}

TermIterator
Database::spellings_begin() const
{
    return TermIterator(internal->open_spelling_wordlist());
}

TermIterator
Database::synonyms_begin(const string& term) const
{
    return TermIterator(internal->open_synonym_termlist(term));
}

TermIterator
Database::synonym_keys_begin(const string& prefix) const
{
    return TermIterator(internal->open_synonym_keylist(prefix));
}

string
Database::get_metadata(const string& key) const
{
    if (rare(key.empty()))
	empty_metadata_key();

    return internal->get_metadata(key);
}

Xapian::TermIterator
Database::metadata_keys_begin(const string& prefix) const
{
    return TermIterator(internal->open_metadata_keylist(prefix));
}

string
Database::get_uuid() const
{
    return internal->get_uuid();
}

bool
Database::locked() const
{
    return internal->locked();
}

Xapian::WritableDatabase
Database::lock(int flags) {
    return Xapian::WritableDatabase(internal->update_lock(flags));
}

Xapian::Database
Database::unlock() {
    return Xapian::Database(internal->update_lock(Xapian::DB_READONLY_));
}

Xapian::rev
Database::get_revision() const
{
    return internal->get_revision();
}

string
Database::reconstruct_text(Xapian::docid did,
			   size_t length,
			   const std::string& prefix,
			   Xapian::termpos start_pos,
			   Xapian::termpos end_pos) const
{
    return internal->reconstruct_text(did, length, prefix, start_pos, end_pos);
}

void
WritableDatabase::commit()
{
    internal->commit();
}

void
WritableDatabase::begin_transaction(bool flushed)
{
    internal->begin_transaction(flushed);
}

void
WritableDatabase::end_transaction_(bool do_commit)
{
    internal->end_transaction(do_commit);
}

Xapian::docid
WritableDatabase::add_document(const Document& doc)
{
    return internal->add_document(doc);
}

void
WritableDatabase::delete_document(Xapian::docid did)
{
    internal->delete_document(did);
}

void
WritableDatabase::delete_document(const string& term)
{
    if (term.empty())
	empty_term_invalid();

    internal->delete_document(term);
}

void
WritableDatabase::replace_document(Xapian::docid did, const Document& doc)
{
    if (rare(did == 0))
	docid_zero_invalid();

    internal->replace_document(did, doc);
}

Xapian::docid
WritableDatabase::replace_document(const string& term, const Document& doc)
{
    if (term.empty())
	empty_term_invalid();

    return internal->replace_document(term, doc);
}

void
WritableDatabase::add_spelling(const string& word,
			       Xapian::termcount freqinc) const
{
    internal->add_spelling(word, freqinc);
}

Xapian::termcount
WritableDatabase::remove_spelling(const string& word,
				  Xapian::termcount freqdec) const
{
    return internal->remove_spelling(word, freqdec);
}

void
WritableDatabase::add_synonym(const string& term,
			      const string& synonym) const
{
    internal->add_synonym(term, synonym);
}

void
WritableDatabase::remove_synonym(const string& term,
				 const string& synonym) const
{
    internal->remove_synonym(term, synonym);
}

void
WritableDatabase::clear_synonyms(const string& term) const
{
    internal->clear_synonyms(term);
}

void
WritableDatabase::set_metadata(const string& key, const string& value)
{
    if (rare(key.empty()))
	empty_metadata_key();

    internal->set_metadata(key, value);
}

string
WritableDatabase::get_description() const
{
    string desc = "WritableDatabase(";
    desc += internal->get_description();
    desc += ')';
    return desc;
}

}
