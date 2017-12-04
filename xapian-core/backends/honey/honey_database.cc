#include <config.h>

#include "honey_database.h"

using namespace std;

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
    return 0; // FIXME
}

Xapian::termcount
HoneyDatabase::get_unique_terms(Xapian::docid did) const
{
    (void)did;
    return 0; // FIXME
}

void
HoneyDatabase::get_freqs(const string& term,
			 Xapian::doccount* termfreq_ptr,
			 Xapian::termcount* collfreq_ptr) const 
{
    (void)term;
    (void)termfreq_ptr;
    (void)collfreq_ptr;
    // FIXME
}

Xapian::doccount
HoneyDatabase::get_value_freq(Xapian::valueno slot) const
{
    (void)slot;
    return 0; // FIXME
}

string
HoneyDatabase::get_value_lower_bound(Xapian::valueno slot) const
{
    (void)slot;
    return string(); // FIXME
}

string
HoneyDatabase::get_value_upper_bound(Xapian::valueno slot) const
{
    (void)slot;
    return string(); // FIXME
}

Xapian::termcount
HoneyDatabase::get_doclength_lower_bound() const
{
    return version_file.get_doclength_lower_bound();
}

Xapian::termcount
HoneyDatabase::get_doclength_upper_bound() const;
{
    return version_file.get_doclength_upper_bound();
}

Xapian::termcount
HoneyDatabase::get_wdf_upper_bound(const string& term) const
{
    return version_file.get_doclength_upper_bound(); // FIXME
}

bool
HoneyDatabase::term_exists(const string& term) const
{
    return true; // FIXME
}

bool
HoneyDatabase::has_positions() const
{
    return true; // FIXME
}

PostList*
HoneyDatabase::open_post_list(const string& term) const
{

}

LeafPostList*
HoneyDatabase::open_leaf_post_list(const string& term) const
{

}

ValueList*
HoneyDatabase::open_value_list(Xapian::valueno slot) const
{

}

TermList*
HoneyDatabase::open_term_list(docid did) const
{

}

TermList*
HoneyDatabase::open_term_list_direct(docid did) const
{

}

TermList*
HoneyDatabase::open_allterms(const string& prefix) const
{

}

PositionList*
HoneyDatabase::open_position_list(docid did, const string& term) const
{

}

Document::Internal*
HoneyDatabase::open_document(docid did, bool lazy) const
{

}

TermList*
HoneyDatabase::open_spelling_termlist(const string& word) const
{

}

TermList*
HoneyDatabase::open_spelling_wordlist() const
{

}

doccount
HoneyDatabase::get_spelling_frequency(const string& word) const
{

}

void
HoneyDatabase::add_spelling(const string& word, termcount freqinc) const
{
    (void)word;
    (void)freqinc;
    throw Xapian::FeatureUnimplemented("Honey backend doesn't support update");
}

termcount
HoneyDatabase::remove_spelling(const string& word, termcount freqdec) const
{
    (void)word;
    (void)freqdec;
    throw Xapian::FeatureUnimplemented("Honey backend doesn't support update");
}

TermList*
HoneyDatabase::open_synonym_termlist(const string& term) const
{

}

TermList*
HoneyDatabase::open_synonym_keylist(const string& prefix) const
{

}

void
HoneyDatabase::add_synonym(const string& term, const string& synonym) const
{
    (void)term;
    (void)synonym;
    throw Xapian::FeatureUnimplemented("Honey backend doesn't support update");
}

void
HoneyDatabase::remove_synonym(const string& term, const string& synonym) const
{
    (void)term;
    (void)synonym;
    throw Xapian::FeatureUnimplemented("Honey backend doesn't support update");
}

void
HoneyDatabase::clear_synonyms(const string& term) const
{
    (void)term;
    throw Xapian::FeatureUnimplemented("Honey backend doesn't support update");
}

string
HoneyDatabase::get_metadata(const string& key) const
{

}

TermList*
HoneyDatabase::open_metadata_keylist(const string& prefix) const
{

}

void
HoneyDatabase::set_metadata(const string& key, const string& value)
{
    (void)key;
    (void)value;
    throw Xapian::FeatureUnimplemented("Honey backend doesn't support update");
}

bool
HoneyDatabase::reopen()
{

}

void
HoneyDatabase::close()
{

}

void
HoneyDatabase::request_document(docid did) const
{

}

string
HoneyDatabase::get_revision_info() const
{

}

string
HoneyDatabase::get_uuid() const
{
    return version_file.get_uuid_string();
}

int
HoneyDatabase::get_backend_info(string* path) const
{
    if (path)
	*path = db_directory;
    return BACKEND_HONEY;
}

void
HoneyDatabase::get_used_docid_range(docid& first, docid& last) const
{
    // FIXME: this isn't the actual used range.
    first = 1;
    last = version_file.get_lastdocid();
}

string
HoneyDatabase::get_description() const
{
    return "Honey()"; // FIXME
}
