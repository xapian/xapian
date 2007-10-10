/* omdatabase.cc: External interface for running queries
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007 Olly Betts
 * Copyright 2006 Richard Boulton
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

#include "autoptr.h"
#include <xapian/error.h>
#include <xapian/unicode.h>
#include "omdebug.h"
#include <xapian/postingiterator.h>
#include <xapian/termiterator.h>
#include <xapian/positioniterator.h>
#include "../backends/multi/multi_postlist.h"
#include "../backends/multi/multi_termlist.h"
#include "alltermslist.h"
#include "multialltermslist.h"
#include "database.h"
#include "editdistance.h"
#include "ortermlist.h"

#include <stdlib.h> // For abs().

#include <vector>

using namespace std;

namespace Xapian {

Database::Database()
{
    DEBUGAPICALL(void, "Database::Database", "");
}

Database::Database(Database::Internal *internal_)
{
    DEBUGAPICALL(void, "Database::Database", "Database::Internal");
    Xapian::Internal::RefCntPtr<Database::Internal> newi(internal_);
    internal.push_back(newi);
}

Database::Database(const Database &other)
{
    DEBUGAPICALL(void, "Database::Database", "Database");
    internal = other.internal;
}

void
Database::operator=(const Database &other)
{
    DEBUGAPICALL(void, "Database::operator=", "Database");
    if (this == &other) {
	DEBUGLINE(API, "Database assigned to itself");
	return;
    }

    internal = other.internal;
}

Database::~Database()
{
    DEBUGAPICALL(void, "Database::~Database", "");
}

void
Database::reopen()
{
    vector<Xapian::Internal::RefCntPtr<Database::Internal> >::iterator i;
    for (i = internal.begin(); i != internal.end(); ++i) {
	(*i)->reopen();
    }
}

void
Database::add_database(const Database & database)
{
    DEBUGAPICALL(void, "Database::add_database", "Database");
    if (this == &database) {
	DEBUGLINE(API, "Database added to itself");
	throw Xapian::InvalidArgumentError("Can't add an Database to itself");
	return;
    }
    vector<Xapian::Internal::RefCntPtr<Database::Internal> >::const_iterator i;
    for (i = database.internal.begin(); i != database.internal.end(); ++i) {
	internal.push_back(*i);
    }
}

PostingIterator
Database::postlist_begin(const string &tname) const
{
    DEBUGAPICALL(PostingIterator, "Database::postlist_begin", tname);

    // Don't bother checking that the term exists first.  If it does, we
    // just end up doing more work, and if it doesn't, we save very little
    // work.
    vector<LeafPostList *> pls;
    try {
	vector<Xapian::Internal::RefCntPtr<Database::Internal> >::const_iterator i;
	for (i = internal.begin(); i != internal.end(); ++i) {
	    pls.push_back((*i)->open_post_list(tname));
	    pls.back()->next();
	}
	Assert(pls.begin() != pls.end());
    } catch (...) {
	vector<LeafPostList *>::iterator i;
	for (i = pls.begin(); i != pls.end(); i++) {
	    delete *i;
	    *i = 0;
	}
	throw;
    }

    RETURN(PostingIterator(new MultiPostList(pls, *this)));
}

TermIterator
Database::termlist_begin(Xapian::docid did) const
{
    DEBUGAPICALL(TermIterator, "Database::termlist_begin", did);
    if (did == 0) throw InvalidArgumentError("Document ID 0 is invalid");

    unsigned int multiplier = internal.size();
    TermList *tl;
    if (multiplier == 1) {
	// There's no need for the MultiTermList wrapper in the common case
	// where we're only dealing with a single database.
	tl = internal[0]->open_term_list(did);
    } else {
	Assert(multiplier != 0);
	Xapian::doccount n = (did - 1) % multiplier; // which actual database
	Xapian::docid m = (did - 1) / multiplier + 1; // real docid in that database

	tl = new MultiTermList(internal[n]->open_term_list(m), *this, n);
    }
    RETURN(TermIterator(tl));
}

TermIterator
Database::allterms_begin() const
{
    return allterms_begin("");
}

TermIterator
Database::allterms_begin(const std::string & prefix) const
{
    DEBUGAPICALL(TermIterator, "Database::allterms_begin", "");
    if (internal.empty()) RETURN(TermIterator(NULL));

    if (internal.size() == 1)
	RETURN(TermIterator(internal[0]->open_allterms(prefix)));
 
    vector<TermList *> lists;

    vector<Xapian::Internal::RefCntPtr<Database::Internal> >::const_iterator i;
    for (i = internal.begin(); i != internal.end(); ++i) {
	lists.push_back((*i)->open_allterms(prefix));
    }

    RETURN(TermIterator(new MultiAllTermsList(lists)));
}

bool
Database::has_positions() const
{
    DEBUGAPICALL(bool, "Database::has_positions", "");
    // If any sub-database has positions, the combined database does.
    vector<Xapian::Internal::RefCntPtr<Database::Internal> >::const_iterator i;
    for (i = internal.begin(); i != internal.end(); ++i) {
	if ((*i)->has_positions()) RETURN(true);
    }
    RETURN(false);
}

PositionIterator
Database::positionlist_begin(Xapian::docid did, const string &tname) const
{
    DEBUGAPICALL(PositionIterator, "Database::positionlist_begin",
		 did << ", " << tname);
    if (tname.empty())
	throw InvalidArgumentError("Zero length terms are invalid");
    if (did == 0) throw InvalidArgumentError("Document ID 0 is invalid");

    unsigned int multiplier = internal.size();
    Assert(multiplier != 0);
    Xapian::doccount n = (did - 1) % multiplier; // which actual database
    Xapian::docid m = (did - 1) / multiplier + 1; // real docid in that database

    RETURN(PositionIterator(internal[n]->open_position_list(m, tname)));
}

Xapian::doccount
Database::get_doccount() const
{
    DEBUGAPICALL(Xapian::doccount, "Database::get_doccount", "");
    Xapian::doccount docs = 0;
    vector<Xapian::Internal::RefCntPtr<Database::Internal> >::const_iterator i;
    for (i = internal.begin(); i != internal.end(); ++i) {
	docs += (*i)->get_doccount();
    }
    RETURN(docs);
}

Xapian::docid
Database::get_lastdocid() const
{
    DEBUGAPICALL(Xapian::docid, "Database::get_lastdocid", "");
    Xapian::docid did = 0;

    unsigned int multiplier = internal.size();
    Assert(multiplier != 0);
    for (Xapian::doccount i = 0; i < multiplier; ++i) {
	Xapian::docid did_i = internal[i]->get_lastdocid();
	if (did_i) did = std::max(did, (did_i - 1) * multiplier + i + 1);
    }
    RETURN(did);
}

Xapian::doclength
Database::get_avlength() const
{
    DEBUGAPICALL(Xapian::doclength, "Database::get_avlength", "");
    Xapian::doccount docs = 0;
    Xapian::doclength totlen = 0;

    vector<Xapian::Internal::RefCntPtr<Database::Internal> >::const_iterator i;
    for (i = internal.begin(); i != internal.end(); ++i) {
	Xapian::doccount db_doccount = (*i)->get_doccount();
	docs += db_doccount;
	totlen += (*i)->get_avlength() * db_doccount;
    }
    DEBUGLINE(UNKNOWN, "get_avlength() = " << totlen << " / " << docs <<
	      " (from " << internal.size() << " dbs)");

    if (docs == 0) RETURN(0.0);
    RETURN(totlen / docs);
}

Xapian::doccount
Database::get_termfreq(const string & tname) const
{
    DEBUGAPICALL(Xapian::doccount, "Database::get_termfreq", tname);
    if (tname.empty()) RETURN(get_doccount());

    Xapian::doccount tf = 0;
    vector<Xapian::Internal::RefCntPtr<Database::Internal> >::const_iterator i;
    for (i = internal.begin(); i != internal.end(); i++) {
	tf += (*i)->get_termfreq(tname);
    }
    RETURN(tf);
}

Xapian::termcount
Database::get_collection_freq(const string & tname) const
{
    DEBUGAPICALL(Xapian::termcount, "Database::get_collection_freq", tname);
    if (tname.empty()) RETURN(get_doccount());

    Xapian::termcount cf = 0;
    vector<Xapian::Internal::RefCntPtr<Database::Internal> >::const_iterator i;
    for (i = internal.begin(); i != internal.end(); i++) {
	cf += (*i)->get_collection_freq(tname);
    }
    RETURN(cf);
}

Xapian::doclength
Database::get_doclength(Xapian::docid did) const
{
    DEBUGAPICALL(Xapian::doclength, "Database::get_doclength", did);
    if (did == 0) throw InvalidArgumentError("Document ID 0 is invalid");

    unsigned int multiplier = internal.size();
    Assert(multiplier != 0);
    Xapian::doccount n = (did - 1) % multiplier; // which actual database
    Xapian::docid m = (did - 1) / multiplier + 1; // real docid in that database
    RETURN(internal[n]->get_doclength(m));
}

Document
Database::get_document(Xapian::docid did) const
{
    DEBUGAPICALL(Document, "Database::get_document", did);
    if (did == 0) throw InvalidArgumentError("Document ID 0 is invalid");

    unsigned int multiplier = internal.size();
    Assert(multiplier != 0);
    Xapian::doccount n = (did - 1) % multiplier; // which actual database
    Xapian::docid m = (did - 1) / multiplier + 1; // real docid in that database

    RETURN(Document(internal[n]->open_document(m)));
}

bool
Database::term_exists(const string & tname) const
{
    if (tname.empty()) {
	return get_doccount() != 0;
    }
    vector<Xapian::Internal::RefCntPtr<Database::Internal> >::const_iterator i;
    for (i = internal.begin(); i != internal.end(); ++i) {
	if ((*i)->term_exists(tname)) return true;
    }
    return false;
}

void
Database::keep_alive()
{
    vector<Xapian::Internal::RefCntPtr<Database::Internal> >::const_iterator i;
    for (i = internal.begin(); i != internal.end(); ++i) {
	(*i)->keep_alive();
    }
}

string
Database::get_description() const
{
    DEBUGCALL(INTRO, string, "Database::get_description", "");
    /// \todo display contents of the database
    RETURN("Database()");
}

// Word must have a trigram score at least this close to the best score seen
// so far.
#define TRIGRAM_SCORE_THRESHOLD 2

string
Database::get_spelling_suggestion(const string &word,
				  unsigned max_edit_distance) const
{
    DEBUGLINE(SPELLING, "Database::get_spelling_suggestion(" << word << ", " <<
			max_edit_distance << ")");
    AutoPtr<TermList> merger;
    for (size_t i = 0; i < internal.size(); ++i) {
	TermList * tl = internal[i]->open_spelling_termlist(word);
	DEBUGLINE(SPELLING, "Sub db " << i << " tl = " << (void*)tl);
	if (tl) {
	    if (merger.get()) {
		merger = new OrTermList(merger.release(), tl);
	    } else {
		merger = tl;
	    }
	}
    }
    if (!merger.get()) return string();

    // Convert word to UTF-32.
    vector<unsigned> utf32_word;
    utf32_word.assign(Utf8Iterator(word), Utf8Iterator());

    vector<unsigned> utf32_term;

    Xapian::termcount best = 1;
    string result;
    int edist_best = max_edit_distance;
    Xapian::doccount freq_best = 0;
    while (true) {
	TermList *ret = merger->next();
	if (ret) merger = ret;

	if (merger->at_end()) break;

	string term = merger->get_termname();
	Xapian::termcount score = merger->get_wdf();

	DEBUGLINE(SPELLING, "Term \"" << term << "\" ngram score " << score);
	if (score + TRIGRAM_SCORE_THRESHOLD >= best) {
	    if (score > best) best = score;

	    // There's no point considering a word where the difference
	    // in length is greater than the smallest number of edits we've
	    // found so far.

	    // First check the length of the encoded UTF-8 version of term.
	    // Each UTF-32 character is 1-4 bytes in UTF-8.
	    if (abs((long)term.size() - (long)word.size()) > edist_best * 4) {
		DEBUGLINE(SPELLING, "Lengths much too different");
		continue;
	    }

	    // Now convert to UTF-32, and compare the true lengths more
	    // strictly.
	    utf32_term.assign(Utf8Iterator(term), Utf8Iterator());

	    if (abs((long)utf32_term.size() - (long)utf32_word.size())
		    > edist_best) {
		DEBUGLINE(SPELLING, "Lengths too different");
		continue;
	    }

	    int edist = edit_distance_unsigned(&utf32_term[0],
					       utf32_term.size(),
					       &utf32_word[0],
					       utf32_word.size());
	    DEBUGLINE(SPELLING, "Edit distance " << edist);
	    // If we have an exact match, return an empty string since there's
	    // no correction required.
	    if (edist == 0) return string();

	    if (edist <= edist_best) {
		Xapian::doccount freq = 0;
		for (size_t j = 0; j < internal.size(); ++j)
		    freq += internal[j]->get_spelling_frequency(term);

		DEBUGLINE(SPELLING, "Freq " << freq << " best " << freq_best);
		if (edist < edist_best || freq > freq_best) {
		    DEBUGLINE(SPELLING, "Best so far: \"" << term <<
					"\" edist " << edist << " freq " <<
					freq);
		    result = term;
		    edist_best = edist;
		    freq_best = freq;
		}
	    }
	}
    }
    DEBUGLINE(SPELLING, "Suggesting \"" << result << "\"");
    return result;
}

TermIterator
Database::spellings_begin() const
{
    DEBUGAPICALL(TermIterator, "Database::spellings_begin", "");
    AutoPtr<TermList> merger;
    for (size_t i = 0; i < internal.size(); ++i) {
	TermList * tl = internal[i]->open_spelling_wordlist();
	if (tl) {
	    if (merger.get()) {
		merger = new FreqAdderOrTermList(merger.release(), tl);
	    } else {
		merger = tl;
	    }
	}
    }
    RETURN(TermIterator(merger.release()));
}

TermIterator
Database::synonyms_begin(const std::string &term) const
{
    DEBUGAPICALL(TermIterator, "Database::synonyms_begin", term);
    AutoPtr<TermList> merger;
    for (size_t i = 0; i < internal.size(); ++i) {
	TermList * tl = internal[i]->open_synonym_termlist(term);
	if (tl) {
	    if (merger.get()) {
		merger = new OrTermList(merger.release(), tl);
	    } else {
		merger = tl;
	    }
	}
    }
    RETURN(TermIterator(merger.release()));
}

TermIterator
Database::synonym_keys_begin(const std::string &prefix) const
{
    DEBUGAPICALL(TermIterator, "Database::synonyms_keys_begin", prefix);
    AutoPtr<TermList> merger;
    for (size_t i = 0; i < internal.size(); ++i) {
	TermList * tl = internal[i]->open_synonym_keylist(prefix);
	if (tl) {
	    if (merger.get()) {
		merger = new OrTermList(merger.release(), tl);
	    } else {
		merger = tl;
	    }
	}
    }
    RETURN(TermIterator(merger.release()));
}

string
Database::get_metadata(const string & key) const
{
    DEBUGAPICALL(string, "Database::get_metadata", key);
    if (key.empty())
	throw InvalidArgumentError("Empty metadata keys are invalid");
    RETURN(internal[0]->get_metadata(key));
}

///////////////////////////////////////////////////////////////////////////

WritableDatabase::WritableDatabase() : Database()
{
    DEBUGAPICALL(void, "WritableDatabase::WritableDatabase", "");
}

WritableDatabase::WritableDatabase(Database::Internal *internal_)
	: Database(internal_)
{
    DEBUGAPICALL(void, "WritableDatabase::WritableDatabase",
		 "Database::Internal");
}

WritableDatabase::WritableDatabase(const WritableDatabase &other)
	: Database(other)
{
    DEBUGAPICALL(void, "WritableDatabase::WritableDatabase", "WritableDatabase");
}

void
WritableDatabase::operator=(const WritableDatabase &other)
{
    DEBUGAPICALL(void, "WritableDatabase::operator=", "WritableDatabase");
    Database::operator=(other);
}

WritableDatabase::~WritableDatabase()
{
    DEBUGAPICALL(void, "WritableDatabase::~WritableDatabase", "");
}

void
WritableDatabase::flush()
{
    DEBUGAPICALL(void, "WritableDatabase::flush", "");
    internal[0]->flush();
}

void
WritableDatabase::begin_transaction(bool flushed)
{
    DEBUGAPICALL(void, "WritableDatabase::begin_transaction", "");
    internal[0]->begin_transaction(flushed);
}

void
WritableDatabase::commit_transaction()
{
    DEBUGAPICALL(void, "WritableDatabase::commit_transaction", "");
    internal[0]->commit_transaction();
}

void
WritableDatabase::cancel_transaction()
{
    DEBUGAPICALL(void, "WritableDatabase::cancel_transaction", "");
    internal[0]->cancel_transaction();
}


Xapian::docid
WritableDatabase::add_document(const Document & document)
{
    DEBUGAPICALL(Xapian::docid, "WritableDatabase::add_document", document);
    RETURN(internal[0]->add_document(document));
}

void
WritableDatabase::delete_document(Xapian::docid did)
{
    DEBUGAPICALL(void, "WritableDatabase::delete_document", did);
    if (did == 0) throw InvalidArgumentError("Document ID 0 is invalid");
    internal[0]->delete_document(did);
}

void
WritableDatabase::delete_document(const std::string & unique_term)
{
    DEBUGAPICALL(void, "WritableDatabase::delete_document", unique_term);
    if (unique_term.empty())
	throw InvalidArgumentError("Empty termnames are invalid");
    internal[0]->delete_document(unique_term);
}

void
WritableDatabase::replace_document(Xapian::docid did, const Document & document)
{
    DEBUGAPICALL(void, "WritableDatabase::replace_document",
		 did << ", " << document);
    if (did == 0) throw Xapian::InvalidArgumentError("Document ID 0 is invalid");
    internal[0]->replace_document(did, document);
}

Xapian::docid
WritableDatabase::replace_document(const std::string & unique_term,
				   const Document & document)
{
    DEBUGAPICALL(Xapian::docid, "WritableDatabase::replace_document",
		 unique_term << ", " << document);
    if (unique_term.empty())
	throw InvalidArgumentError("Empty termnames are invalid");
    RETURN(internal[0]->replace_document(unique_term, document));
}

void
WritableDatabase::add_spelling(const std::string & word,
			       Xapian::termcount freqinc) const
{
    DEBUGAPICALL(void, "WritableDatabase::add_spelling",
		 word << ", " << freqinc);
    internal[0]->add_spelling(word, freqinc);
}

void
WritableDatabase::remove_spelling(const std::string & word,
				  Xapian::termcount freqdec) const
{
    DEBUGAPICALL(void, "WritableDatabase::remove_spelling",
		 word << ", " << freqdec);
    internal[0]->remove_spelling(word, freqdec);
}

void
WritableDatabase::add_synonym(const std::string & term,
			      const std::string & synonym) const
{
    DEBUGAPICALL(void, "WritableDatabase::add_synonym",
		 term << ", " << synonym);
    internal[0]->add_synonym(term, synonym);
}

void
WritableDatabase::remove_synonym(const std::string & term,
				 const std::string & synonym) const
{
    DEBUGAPICALL(void, "WritableDatabase::remove_synonym",
		 term << ", " << synonym);
    internal[0]->remove_synonym(term, synonym);
}

void
WritableDatabase::clear_synonyms(const std::string & term) const
{
    DEBUGAPICALL(void, "WritableDatabase::clear_synonyms", term);
    internal[0]->clear_synonyms(term);
}

void
WritableDatabase::set_metadata(const string & key, const string & value)
{
    DEBUGAPICALL(void, "WritableDatabase::set_metadata", key << ", " << value);
    if (key.empty())
	throw InvalidArgumentError("Empty metadata keys are invalid");
    internal[0]->set_metadata(key, value);
}

string
WritableDatabase::get_description() const
{
    DEBUGCALL(INTRO, string, "WritableDatabase::get_description", "");
    /// \todo display contents of the writable database
    RETURN("WritableDatabase()");
}

}
