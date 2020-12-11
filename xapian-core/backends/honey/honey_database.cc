/** @file
 * @brief Honey backend database class
 */
/* Copyright 2015,2017,2018 Olly Betts
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

#include "honey_database.h"

#include "honey_alltermslist.h"
#include "honey_document.h"
#include "honey_metadata.h"
#include "honey_termlist.h"
#include "honey_spellingwordslist.h"
#include "honey_valuelist.h"

#include "backends/backends.h"
#include "backends/leafpostlist.h"
#include "xapian/error.h"

using namespace std;

void
HoneyDatabase::throw_termlist_table_close_exception() const
{
    // Either the database has been closed, or else there's no termlist table.
    // Check if the postlist table is open to determine which is the case.
    if (!postlist_table.is_open())
	HoneyTable::throw_database_closed();
    throw Xapian::FeatureUnavailableError("Database has no termlist");
}

// Relied on below - opening to read should allow the termlist to be missing.
static_assert(Xapian::DB_READONLY_ & Xapian::DB_NO_TERMLIST,
	"Xapian::DB_READONLY_ should imply Xapian::DB_NO_TERMLIST");

HoneyDatabase::HoneyDatabase(const std::string& path_, int flags)
    : Xapian::Database::Internal(TRANSACTION_READONLY),
      path(path_),
      version_file(path_),
      docdata_table(path_, true),
      postlist_table(path_, true),
      position_table(path_, true),
      spelling_table(path_, true),
      synonym_table(path_, true),
      // Note: (Xapian::DB_READONLY_ & Xapian::DB_NO_TERMLIST) is true, so
      // opening to read we always allow the termlist to be missing.
      termlist_table(path_, true, (flags & Xapian::DB_NO_TERMLIST)),
      value_manager(postlist_table, termlist_table)
{
    version_file.read();
    auto rev = version_file.get_revision();
    docdata_table.open(flags, version_file.get_root(Honey::DOCDATA), rev);
    postlist_table.open(flags, version_file.get_root(Honey::POSTLIST), rev);
    position_table.open(flags, version_file.get_root(Honey::POSITION), rev);
    spelling_table.open(flags, version_file.get_root(Honey::SPELLING), rev);
    synonym_table.open(flags, version_file.get_root(Honey::SYNONYM), rev);
    termlist_table.open(flags, version_file.get_root(Honey::TERMLIST), rev);
}

HoneyDatabase::HoneyDatabase(int fd, int flags)
    : Xapian::Database::Internal(TRANSACTION_READONLY),
      version_file(fd),
      docdata_table(fd, version_file.get_offset(), true),
      postlist_table(fd, version_file.get_offset(), true),
      position_table(fd, version_file.get_offset(), true),
      spelling_table(fd, version_file.get_offset(), true),
      synonym_table(fd, version_file.get_offset(), true),
      // Note: (Xapian::DB_READONLY_ & Xapian::DB_NO_TERMLIST) is true, so
      // opening to read we always allow the termlist to be missing.
      termlist_table(fd, version_file.get_offset(), true,
		     (flags & Xapian::DB_NO_TERMLIST)),
      value_manager(postlist_table, termlist_table)
{
    version_file.read();
    auto rev = version_file.get_revision();
    docdata_table.open(flags, version_file.get_root(Honey::DOCDATA), rev);
    postlist_table.open(flags, version_file.get_root(Honey::POSTLIST), rev);
    position_table.open(flags, version_file.get_root(Honey::POSITION), rev);
    spelling_table.open(flags, version_file.get_root(Honey::SPELLING), rev);
    synonym_table.open(flags, version_file.get_root(Honey::SYNONYM), rev);
    termlist_table.open(flags, version_file.get_root(Honey::TERMLIST), rev);
}

HoneyDatabase::~HoneyDatabase()
{
    delete doclen_cursor;
}

void
HoneyDatabase::readahead_for_query(const Xapian::Query& query) const
{
    (void)query;
    // FIXME: Implement - pre-read the start of the postlist table?
}

Xapian::doccount
HoneyDatabase::get_doccount() const
{
    return version_file.get_doccount();
}

Xapian::docid
HoneyDatabase::get_lastdocid() const
{
    return version_file.get_last_docid();
}

Xapian::totallength
HoneyDatabase::get_total_length() const
{
    return version_file.get_total_doclen();
}

Xapian::termcount
HoneyDatabase::get_doclength(Xapian::docid did) const
{
    Assert(did != 0);
    if (usual(did <= version_file.get_last_docid())) {
	if (doclen_cursor == NULL) {
	    doclen_cursor = get_postlist_cursor();
	} else {
	    if (doclen_chunk_reader.find_doclength(did)) {
		return doclen_chunk_reader.get_doclength();
	    }
	}

	// If exact is true, the desired docid is the last in this chunk.
	bool exact =
	    doclen_cursor->find_entry_ge(Honey::make_doclenchunk_key(did));
	if (doclen_chunk_reader.update(doclen_cursor)) {
	    if (exact)
		return doclen_chunk_reader.back();
	    if (doclen_chunk_reader.find_doclength(did)) {
		return doclen_chunk_reader.get_doclength();
	    }
	}
    }

    string message = "Document ID not in use: ";
    message += str(did);
    throw Xapian::DocNotFoundError(message);
}

Xapian::termcount
HoneyDatabase::get_unique_terms(Xapian::docid did) const
{
    Assert(did != 0);
    return HoneyTermList(this, did).get_unique_terms();
}

Xapian::termcount
HoneyDatabase::get_wdfdocmax(Xapian::docid did) const
{
    Assert(did != 0);
    HoneyTermList termlist(this, did);
    Xapian::termcount max_wdf = 0;
    termlist.next();
    while (!termlist.at_end()) {
	Xapian::termcount current_wdf = termlist.get_wdf();
	if (current_wdf > max_wdf) max_wdf = current_wdf;
	termlist.next();
    }
    return max_wdf;
}

void
HoneyDatabase::get_freqs(const string& term,
			 Xapian::doccount* termfreq_ptr,
			 Xapian::termcount* collfreq_ptr) const
{
    postlist_table.get_freqs(term, termfreq_ptr, collfreq_ptr);
}

Xapian::doccount
HoneyDatabase::get_value_freq(Xapian::valueno slot) const
{
    return value_manager.get_value_freq(slot);
}

string
HoneyDatabase::get_value_lower_bound(Xapian::valueno slot) const
{
    return value_manager.get_value_lower_bound(slot);
}

string
HoneyDatabase::get_value_upper_bound(Xapian::valueno slot) const
{
    return value_manager.get_value_upper_bound(slot);
}

Xapian::termcount
HoneyDatabase::get_doclength_lower_bound() const
{
    return version_file.get_doclength_lower_bound();
}

Xapian::termcount
HoneyDatabase::get_doclength_upper_bound() const
{
    return version_file.get_doclength_upper_bound();
}

Xapian::termcount
HoneyDatabase::get_wdf_upper_bound(const string& term) const
{
    Xapian::termcount wdf_bound = version_file.get_wdf_upper_bound();
    // It's unlikely wdf is always 0, but when it is there's no need to do any
    // further work.
    if (usual(wdf_bound != 0)) {
	// We don't store per-term wdf upper bounds currently, but
	// HoneyPostListTable can provide an upper bound based on termfreq,
	// coll_freq, and the first wdf value, which more often than not is
	// actually the exact bound (in 77% of cases in an example database of
	// wikipedia data).
	wdf_bound = min(wdf_bound, postlist_table.get_wdf_upper_bound(term));
    }
    return wdf_bound;
}

Xapian::termcount
HoneyDatabase::get_unique_terms_lower_bound() const
{
    return version_file.get_unique_terms_lower_bound();
}

Xapian::termcount
HoneyDatabase::get_unique_terms_upper_bound() const
{
    return version_file.get_unique_terms_upper_bound();
}

bool
HoneyDatabase::term_exists(const string& term) const
{
    if (term.empty())
	return HoneyDatabase::get_doccount() != 0;
    return postlist_table.term_exists(term);
}

bool
HoneyDatabase::has_positions() const
{
    return !position_table.empty();
}

PostList*
HoneyDatabase::open_post_list(const string& term) const
{
    return HoneyDatabase::open_leaf_post_list(term, false);
}

LeafPostList*
HoneyDatabase::open_leaf_post_list(const string& term, bool need_read_pos) const
{
    if (term.empty()) {
	Assert(!need_read_pos);
	return new HoneyAllDocsPostList(this, get_doccount());
    }

    return postlist_table.open_post_list(this, term, need_read_pos);
}

ValueList*
HoneyDatabase::open_value_list(Xapian::valueno slot) const
{
    return new HoneyValueList(slot, this);
}

TermList*
HoneyDatabase::open_term_list(Xapian::docid did) const
{
    Assert(did != 0);
    if (!termlist_table.is_open())
	throw_termlist_table_close_exception();
    HoneyTermList* tl = new HoneyTermList(this, did);
    if (tl->size() == 0) {
	// It could be the document has no terms, but maybe it doesn't exist -
	// in the latter case we ought to throw DocNotFoundError.  FIXME: If
	// the document has no terms, but does have values, we should be able
	// to avoid this check.

	// Put the pointer in a unique_ptr so it gets released if an exception
	// is thrown.
	unique_ptr<TermList> tl_ptr(tl);

	// This will throw DocNotFoundError if did isn't in use.
	(void)HoneyDatabase::get_doclength(did);
	tl_ptr.release();
    }
    return tl;
}

TermList*
HoneyDatabase::open_term_list_direct(Xapian::docid did) const
{
    // Same as open_term_list() except for MultiDatabase.
    return HoneyDatabase::open_term_list(did);
}

TermList*
HoneyDatabase::open_allterms(const string& prefix) const
{
    return new HoneyAllTermsList(this, prefix);
}

PositionList*
HoneyDatabase::open_position_list(Xapian::docid did, const string& term) const
{
    return new HoneyPositionList(position_table, did, term);
}

Xapian::Document::Internal*
HoneyDatabase::open_document(Xapian::docid did, bool lazy) const
{
    Assert(did != 0);
    if (!lazy) {
	// This will throw DocNotFoundError if did isn't in use.
	(void)HoneyDatabase::get_doclength(did);
    }
    return new HoneyDocument(this, did, &value_manager, &docdata_table);
}

TermList*
HoneyDatabase::open_spelling_termlist(const string& word) const
{
    return spelling_table.open_termlist(word);
}

TermList*
HoneyDatabase::open_spelling_wordlist() const
{
    auto cursor = spelling_table.cursor_get();
    if (rare(cursor == NULL)) {
	// No spelling table.
	return NULL;
    }
    return new HoneySpellingWordsList(this, cursor);
}

Xapian::doccount
HoneyDatabase::get_spelling_frequency(const string& word) const
{
    return spelling_table.get_word_frequency(word);
}

void
HoneyDatabase::add_spelling(const string& word, Xapian::termcount freqinc) const
{
    (void)word;
    (void)freqinc;
    throw Xapian::UnimplementedError("Honey backend doesn't support update");
}

Xapian::termcount
HoneyDatabase::remove_spelling(const string& word,
			       Xapian::termcount freqdec) const
{
    (void)word;
    (void)freqdec;
    throw Xapian::UnimplementedError("Honey backend doesn't support update");
}

TermList*
HoneyDatabase::open_synonym_termlist(const string& term) const
{
    return synonym_table.open_termlist(term);
}

TermList*
HoneyDatabase::open_synonym_keylist(const string& prefix) const
{
    auto cursor = synonym_table.cursor_get();
    if (rare(cursor == NULL)) {
	// No synonym table.
	return NULL;
    }
    return new HoneySynonymTermList(this, cursor, prefix);
}

void
HoneyDatabase::add_synonym(const string& term, const string& synonym) const
{
    (void)term;
    (void)synonym;
    throw Xapian::UnimplementedError("Honey backend doesn't support update");
}

void
HoneyDatabase::remove_synonym(const string& term, const string& synonym) const
{
    (void)term;
    (void)synonym;
    throw Xapian::UnimplementedError("Honey backend doesn't support update");
}

void
HoneyDatabase::clear_synonyms(const string& term) const
{
    (void)term;
    throw Xapian::UnimplementedError("Honey backend doesn't support update");
}

string
HoneyDatabase::get_metadata(const string& key) const
{
    return postlist_table.get_metadata(key);
}

TermList*
HoneyDatabase::open_metadata_keylist(const string& prefix) const
{
    auto cursor = postlist_table.cursor_get();
    Assert(cursor != NULL);
    return new HoneyMetadataTermList(this, cursor, prefix);
}

void
HoneyDatabase::set_metadata(const string& key, const string& value)
{
    (void)key;
    (void)value;
    throw Xapian::UnimplementedError("Honey backend doesn't support update");
}

bool
HoneyDatabase::reopen()
{
    if (!postlist_table.is_open())
	HoneyTable::throw_database_closed();
    return false;
}

void
HoneyDatabase::close()
{
    docdata_table.close(true);
    postlist_table.close(true);
    position_table.close(true);
    spelling_table.close(true);
    synonym_table.close(true);
    termlist_table.close(true);
}

void
HoneyDatabase::request_document(Xapian::docid did) const
{
    Assert(did != 0);
    (void)did; // FIXME
}

Xapian::rev
HoneyDatabase::get_revision() const
{
    return version_file.get_revision();
}

string
HoneyDatabase::get_uuid() const
{
    return version_file.get_uuid_string();
}

int
HoneyDatabase::get_backend_info(string* path_ptr) const
{
    if (path_ptr)
	*path_ptr = path;
    return BACKEND_HONEY;
}

void
HoneyDatabase::get_used_docid_range(Xapian::docid& first,
				    Xapian::docid& last) const
{
    auto doccount = version_file.get_doccount();
    if (doccount == 0) {
	// Empty database.
	first = last = 0;
	return;
    }
    auto last_docid = version_file.get_last_docid();
    if (last_docid == doccount) {
	// Contiguous range starting at 1.
	first = 1;
	last = last_docid;
	return;
    }
    postlist_table.get_used_docid_range(doccount, first, last);
}

string
HoneyDatabase::get_description() const
{
    string desc = "Honey(";
    desc += path;
    desc += ')';
    return desc;
}
