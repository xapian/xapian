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
#include <xapian/error.h>
#include "omdebug.h"
#include <xapian/postingiterator.h>
#include <xapian/termiterator.h>
#include <xapian/positioniterator.h>
#include "../backends/multi/multi_postlist.h"
#include "../backends/multi/multi_termlist.h"
#include "alltermslist.h"
#include "multialltermslist.h"
#include "database.h"

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
    DEBUGAPICALL(TermIterator, "Database::allterms_begin", "");
    if (internal.empty()) RETURN(TermIterator(NULL));

    if (internal.size() == 1)
	RETURN(TermIterator(internal[0]->open_allterms()));
 
    vector<TermList *> lists;

    vector<Xapian::Internal::RefCntPtr<Database::Internal> >::const_iterator i;
    for (i = internal.begin(); i != internal.end(); ++i) {
	lists.push_back((*i)->open_allterms());
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
    if (tname.empty()) {
	return get_doccount();
    }
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
    if (tname.empty()) {
	return get_doccount();
    }

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
    DEBUGAPICALL(void, "WritableDatabase::replace_document",
		 unique_term << ", " << document);
    if (unique_term.empty())
	throw InvalidArgumentError("Empty termnames are invalid");
    return internal[0]->replace_document(unique_term, document);
}

string
WritableDatabase::get_description() const
{
    DEBUGCALL(INTRO, string, "WritableDatabase::get_description", "");
    /// \todo display contents of the writable database
    RETURN("WritableDatabase()");
}

}
