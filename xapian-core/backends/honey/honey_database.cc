#include <config.h>

#include "honey_database.h"

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
      termlist_table(path_, true, false)
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
    (void)did;
    return 0; // TODO0
}

Xapian::termcount
HoneyDatabase::get_unique_terms(Xapian::docid did) const
{
    (void)did;
    return 0; // TODO1
}

void
HoneyDatabase::get_freqs(const string& term,
			 Xapian::doccount* termfreq_ptr,
			 Xapian::termcount* collfreq_ptr) const 
{
    (void)term;
    (void)termfreq_ptr;
    (void)collfreq_ptr;
    // TODO0
}

Xapian::doccount
HoneyDatabase::get_value_freq(Xapian::valueno slot) const
{
    (void)slot;
    return 0; // TODO1
}

string
HoneyDatabase::get_value_lower_bound(Xapian::valueno slot) const
{
    (void)slot;
    return string(); // TODO1
}

string
HoneyDatabase::get_value_upper_bound(Xapian::valueno slot) const
{
    (void)slot;
    return string(); // TODO1
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
    (void)term;
    return version_file.get_doclength_upper_bound(); // TODO0
}

bool
HoneyDatabase::term_exists(const string& term) const
{
    (void)term;
    return true; // TODO0
}

bool
HoneyDatabase::has_positions() const
{
    return !position_table.empty();
}

PostList*
HoneyDatabase::open_post_list(const string& term) const
{
    (void)term;
    return NULL; // TODO0
}

LeafPostList*
HoneyDatabase::open_leaf_post_list(const string& term) const
{
    (void)term;
    return NULL; // TODO0
}

ValueList*
HoneyDatabase::open_value_list(Xapian::valueno slot) const
{
    (void)slot;
    return NULL; // TODO1
}

TermList*
HoneyDatabase::open_term_list(Xapian::docid did) const
{
    (void)did;
    return NULL; // TODO0
}

TermList*
HoneyDatabase::open_term_list_direct(Xapian::docid did) const
{
    (void)did;
    return NULL; // TODO0
}

TermList*
HoneyDatabase::open_allterms(const string& prefix) const
{
    (void)prefix;
    return NULL; // TODO0
}

PositionList*
HoneyDatabase::open_position_list(Xapian::docid did, const string& term) const
{
    (void)did;
    (void)term;
    return NULL; // TODO0
}

Xapian::Document::Internal*
HoneyDatabase::open_document(Xapian::docid did, bool lazy) const
{
    (void)did;
    (void)lazy;
    return NULL; // TODO0
}

TermList*
HoneyDatabase::open_spelling_termlist(const string& word) const
{
    (void)word;
    return NULL; // TODO1
}

TermList*
HoneyDatabase::open_spelling_wordlist() const
{
    return NULL; // TODO1
}

Xapian::doccount
HoneyDatabase::get_spelling_frequency(const string& word) const
{
    (void)word;
    return 0; // TODO1
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
    (void)term;
    return NULL; // TODO2
}

TermList*
HoneyDatabase::open_synonym_keylist(const string& prefix) const
{
    (void)prefix;
    return NULL; // TODO2
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
