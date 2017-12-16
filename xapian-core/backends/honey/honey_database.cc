#include <config.h>

#include "honey_database.h"

#include "honey_alltermslist.h"
#include "honey_document.h"
#include "honey_termlist.h"
#include "honey_spellingwordslist.h"
#include "honey_valuelist.h"

#include "api/leafpostlist.h"
#include "backends/backends.h"
#include "xapian/error.h"

using namespace std;

HoneyDatabase::HoneyDatabase(const std::string& path_)
    : Xapian::Database::Internal(TRANSACTION_READONLY),
      path(path_),
      version_file(path_),
      docdata_table(path_, true),
      postlist_table(path_, true),
      position_table(path_, true),
      spelling_table(path_, true),
      synonym_table(path_, true),
      termlist_table(path_, true, false),
      value_manager(postlist_table, termlist_table)
{
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
    return postlist_table.get_doclength(did);
}

Xapian::termcount
HoneyDatabase::get_unique_terms(Xapian::docid did) const
{
    Assert(did != 0);
    (void)did;
    return 0; // TODO1
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
    // We don't store per-term wdf upper bounds currently, only a per-database
    // wdf bound.  However, the collection frequency of the term provides a
    // second upper bound (since collection frequency is the sum of the wdf and
    // wdf >= 0), so pick the tighter of these bounds.
    Xapian::termcount wdf_bound = version_file.get_wdf_upper_bound();
    // It's unlikely wdf is always 0, but when it is there's no need to check
    // the collection frequency.
    if (usual(wdf_bound != 0)) {
	Xapian::termcount coll_freq;
	get_freqs(term, NULL, &coll_freq);
	if (coll_freq < wdf_bound) {
	    wdf_bound = coll_freq;
	}
    }
    return wdf_bound;
}

bool
HoneyDatabase::term_exists(const string& term) const
{
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
    return HoneyDatabase::open_leaf_post_list(term);
}

LeafPostList*
HoneyDatabase::open_leaf_post_list(const string& term) const
{
    return postlist_table.open_post_list(this, term);
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
    return new HoneyTermList(this, did);
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
HoneyDatabase::remove_spelling(const string& word, Xapian::termcount freqdec) const
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
    (void)key;
    return string(); // TODO2
}

TermList*
HoneyDatabase::open_metadata_keylist(const string& prefix) const
{
    (void)prefix;
    return NULL; // TODO2
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
    return false;
}

void
HoneyDatabase::close()
{
    // TODO1
}

void
HoneyDatabase::request_document(Xapian::docid did) const
{
    Assert(did != 0);
    (void)did; // FIXME
}

string
HoneyDatabase::get_revision_info() const
{
    return string(); // TODO2
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
    // TODO1: this isn't the actual used range.
    first = 1;
    last = version_file.get_last_docid();
}

string
HoneyDatabase::get_description() const
{
    string desc = "Honey(";
    desc += path;
    desc += ')';
    return desc;
}
