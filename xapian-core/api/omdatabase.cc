/* omdatabase.cc: External interface for running queries
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003 Olly Betts
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include <config.h>
#include "xapian/error.h"
#include "omdatabaseinternal.h"
#include "omdebug.h"
#include <xapian/postlistiterator.h>
#include "xapian/termiterator.h"
#include "omtermlistiteratorinternal.h"
#include "xapian/positionlistiterator.h"
#include <xapian/output.h>

OmDatabase::OmDatabase() : internal(new OmDatabase::Internal())
{
    DEBUGAPICALL(void, "OmDatabase::OmDatabase", "");
}

OmDatabase::OmDatabase(OmDatabase::Internal *internal_) : internal(internal_)
{
    DEBUGAPICALL(void, "OmDatabase::OmDatabase", "OmDatabase::Internal");
}

OmDatabase::OmDatabase(const OmDatabase &other)
	: internal(0)
{
    DEBUGAPICALL(void, "OmDatabase::OmDatabase", "OmDatabase");
    internal = new Internal(*(other.internal));
}

void
OmDatabase::operator=(const OmDatabase &other)
{
    DEBUGAPICALL(void, "OmDatabase::operator=", "OmDatabase");
    if (this == &other) {
	DEBUGLINE(API, "OmDatabase assigned to itself");
	return;
    }

    Internal * newinternal = new Internal(*(other.internal));
    std::swap(internal, newinternal);
    delete newinternal;
}

OmDatabase::~OmDatabase()
{
    DEBUGAPICALL(void, "OmDatabase::~OmDatabase", "");
    delete internal;
    internal = 0;
}

void
OmDatabase::reopen()
{
    std::vector<RefCntPtr<Database> >::iterator i;
    for (i = internal->databases.begin(); i != internal->databases.end(); i++) {
	(*i)->do_reopen();
    }
}

void
OmDatabase::add_database(const OmDatabase & database)
{
    DEBUGAPICALL(void, "OmDatabase::add_database", "OmDatabase");
    if (this == &database) {
	DEBUGLINE(API, "OmDatabase added to itself");
	throw Xapian::InvalidArgumentError("Can't add an OmDatabase to itself");
	return;
    }
    std::vector<RefCntPtr<Database> >::iterator i;
    for (i = database.internal->databases.begin();
	 i != database.internal->databases.end(); i++) {
	internal->add_database(*i);
    }
}

Xapian::PostListIterator
OmDatabase::postlist_begin(const string &tname) const
{
    DEBUGAPICALL(Xapian::PostListIterator, "OmDatabase::postlist_begin", tname);
    if (tname.empty())
       	throw Xapian::InvalidArgumentError("Zero length terms are invalid");
    RETURN(Xapian::PostListIterator(internal->open_post_list(tname, *this)));
}

Xapian::PostListIterator
OmDatabase::postlist_end(const string &tname) const
{
    DEBUGAPICALL(Xapian::PostListIterator, "OmDatabase::postlist_end", tname);
    if (tname.empty())
       	throw Xapian::InvalidArgumentError("Zero length terms are invalid");
    RETURN(Xapian::PostListIterator(NULL));
}

Xapian::TermIterator
OmDatabase::termlist_begin(om_docid did) const
{
    DEBUGAPICALL(Xapian::TermIterator, "OmDatabase::termlist_begin", did);
    if (did == 0) throw Xapian::InvalidArgumentError("Document IDs of 0 are invalid");

    RETURN(Xapian::TermIterator(internal->open_term_list(did, *this)));
}

Xapian::TermIterator
OmDatabase::termlist_end(om_docid did) const
{
    DEBUGAPICALL(Xapian::TermIterator, "OmDatabase::termlist_end", did);
    if (did == 0) throw Xapian::InvalidArgumentError("Document IDs of 0 are invalid");
    RETURN(Xapian::TermIterator(NULL));
}

Xapian::TermIterator
OmDatabase::allterms_begin() const
{
    DEBUGAPICALL(Xapian::TermIterator, "OmDatabase::allterms_begin", "");
    RETURN(Xapian::TermIterator(internal->open_allterms()));
}

Xapian::TermIterator
OmDatabase::allterms_end() const
{
    DEBUGAPICALL(Xapian::TermIterator, "OmDatabase::allterms_end", "");
    RETURN(Xapian::TermIterator(NULL));
}

Xapian::PositionListIterator
OmDatabase::positionlist_begin(om_docid did, const string &tname) const
{
    DEBUGAPICALL(Xapian::PositionListIterator, "OmDatabase::positionlist_begin",
		 did << ", " << tname);
    if (tname.empty())
       	throw Xapian::InvalidArgumentError("Zero length terms are invalid");
    if (did == 0) throw Xapian::InvalidArgumentError("Document IDs of 0 are invalid");
    RETURN(Xapian::PositionListIterator(internal->open_position_list(did, tname)));
}

Xapian::PositionListIterator
OmDatabase::positionlist_end(om_docid did, const string &tname) const
{
    DEBUGAPICALL(Xapian::PositionListIterator, "OmDatabase::positionlist_end",
		 did << ", " << tname);
    if (tname.empty())
       	throw Xapian::InvalidArgumentError("Zero length terms are invalid");
    if (did == 0) throw Xapian::InvalidArgumentError("Document IDs of 0 are invalid");
    RETURN(Xapian::PositionListIterator(NULL));
}

om_doccount
OmDatabase::get_doccount() const
{
    DEBUGAPICALL(om_doccount, "OmDatabase::get_doccount", "");
    om_doccount docs = 0;
    std::vector<RefCntPtr<Database> >::const_iterator i;
    for (i = internal->databases.begin(); i != internal->databases.end(); i++) {
	docs += (*i)->get_doccount();
    }
    RETURN(docs);
}

om_doclength
OmDatabase::get_avlength() const
{
    DEBUGAPICALL(om_doclength, "OmDatabase::get_avlength", "");
    RETURN(internal->get_avlength());
}

om_doccount
OmDatabase::get_termfreq(const string & tname) const
{
    DEBUGAPICALL(om_doccount, "OmDatabase::get_termfreq", tname);
    if (tname.empty())
	throw Xapian::InvalidArgumentError("Zero length terms are invalid");
    om_doccount tf = 0;
    std::vector<RefCntPtr<Database> >::const_iterator i;
    for (i = internal->databases.begin(); i != internal->databases.end(); i++) {
	tf += (*i)->get_termfreq(tname);
    }
    RETURN(tf);
}

om_termcount
OmDatabase::get_collection_freq(const string & tname) const
{
    DEBUGAPICALL(om_termcount, "OmDatabase::get_collection_freq", tname);
    if (tname.empty())
	throw Xapian::InvalidArgumentError("Zero length terms are invalid");
    om_termcount cf = 0;
    std::vector<RefCntPtr<Database> >::const_iterator i;
    for (i = internal->databases.begin(); i != internal->databases.end(); i++) {
	cf += (*i)->get_collection_freq(tname);
    }
    RETURN(cf);
}

om_doclength
OmDatabase::get_doclength(om_docid did) const
{
    DEBUGAPICALL(om_doclength, "OmDatabase::get_doclength", did);
    if (did == 0) throw Xapian::InvalidArgumentError("Document IDs of 0 are invalid");
    unsigned int multiplier = internal->databases.size();
    om_docid realdid = (did - 1) / multiplier + 1;
    om_doccount dbnumber = (did - 1) % multiplier;
    RETURN(internal->databases[dbnumber]->get_doclength(realdid));
}

OmDocument
OmDatabase::get_document(om_docid did) const
{
    DEBUGAPICALL(OmDocument, "OmDatabase::get_document", did);
    if (did == 0) throw Xapian::InvalidArgumentError("Document IDs of 0 are invalid");

    unsigned int multiplier = internal->databases.size();
    om_docid realdid = (did - 1) / multiplier + 1;
    om_doccount dbnumber = (did - 1) % multiplier;

    // create our own RefCntPtr in case another thread assigns a new ptr
    RefCntPtr<Database> database = internal->databases[dbnumber];

    RETURN(OmDocument(new OmDocument::Internal(database->open_document(realdid),
					       *this, did)));
}

bool
OmDatabase::term_exists(const string & tname) const
{
    if (tname.empty())
	throw Xapian::InvalidArgumentError("Zero length terms are invalid");
    std::vector<RefCntPtr<Database> >::const_iterator i;
    for (i = internal->databases.begin(); i != internal->databases.end(); i++) {
	if ((*i)->term_exists(tname)) return true;
    }
    return false;
}

void
OmDatabase::keep_alive()
{
    std::vector<RefCntPtr<Database> >::const_iterator i;
    for (i = internal->databases.begin(); i != internal->databases.end(); i++) {
	(*i)->keep_alive();
    }
}

std::string
OmDatabase::get_description() const
{
    DEBUGCALL(INTRO, std::string, "OmDatabase::get_description", "");
    /// \todo display contents of the database
    RETURN("OmDatabase()");
}

OmWritableDatabase::OmWritableDatabase() : OmDatabase()
{
    DEBUGAPICALL(void, "OmWritableDatabase::OmWritableDatabase", "");
}

OmWritableDatabase::OmWritableDatabase(OmDatabase::Internal *internal_)
	: OmDatabase(internal_)
{
    DEBUGAPICALL(void, "OmWritableDatabase::OmWritableDatabase",
		 "OmDatabase::Internal");
}

OmWritableDatabase::OmWritableDatabase(const OmWritableDatabase &other)
	: OmDatabase(other)
{
    DEBUGAPICALL(void, "OmWritableDatabase::OmWritableDatabase", "OmWritableDatabase");
}

void
OmWritableDatabase::operator=(const OmWritableDatabase &other)
{
    DEBUGAPICALL(void, "OmWritableDatabase::operator=", "OmWritableDatabase");
    OmDatabase::operator=(other);
}

OmWritableDatabase::~OmWritableDatabase()
{
    DEBUGAPICALL(void, "OmWritableDatabase::~OmWritableDatabase", "");
    // Don't delete internal here - that's OmDatabase's dtor's job
}

void
OmWritableDatabase::flush()
{
    DEBUGAPICALL(void, "OmWritableDatabase::flush", "");
    // create our own RefCntPtr in case another thread assigns a new ptr
    RefCntPtr<Database> database = internal->databases[0];
    database->flush();
}

void
OmWritableDatabase::begin_transaction()
{
    DEBUGAPICALL(void, "OmWritableDatabase::begin_transaction", "");
    // create our own RefCntPtr in case another thread assigns a new ptr
    RefCntPtr<Database> database = internal->databases[0];
    database->begin_transaction();
}

void
OmWritableDatabase::commit_transaction()
{
    DEBUGAPICALL(void, "OmWritableDatabase::commit_transaction", "");
    // create our own RefCntPtr in case another thread assigns a new ptr
    RefCntPtr<Database> database = internal->databases[0];
    database->commit_transaction();
}

void
OmWritableDatabase::cancel_transaction()
{
    DEBUGAPICALL(void, "OmWritableDatabase::cancel_transaction", "");
    // create our own RefCntPtr in case another thread assigns a new ptr
    RefCntPtr<Database> database = internal->databases[0];
    database->cancel_transaction();
}


om_docid
OmWritableDatabase::add_document(const OmDocument & document)
{
    DEBUGAPICALL(om_docid, "OmWritableDatabase::add_document", document);
    // create our own RefCntPtr in case another thread assigns a new ptr
    RefCntPtr<Database> database = internal->databases[0];
    RETURN(database->add_document(document));
}

void
OmWritableDatabase::delete_document(om_docid did)
{
    DEBUGAPICALL(void, "OmWritableDatabase::delete_document", did);
    if (did == 0) throw Xapian::InvalidArgumentError("Document IDs of 0 are invalid");
    // create our own RefCntPtr in case another thread assigns a new ptr
    RefCntPtr<Database> database = internal->databases[0];
    database->delete_document(did);
}

void
OmWritableDatabase::replace_document(om_docid did, const OmDocument & document)
{
    DEBUGAPICALL(void, "OmWritableDatabase::replace_document",
		 did << ", " << document);
    if (did == 0) throw Xapian::InvalidArgumentError("Document IDs of 0 are invalid");
    // create our own RefCntPtr in case another thread assigns a new ptr
    RefCntPtr<Database> database = internal->databases[0];
    database->replace_document(did, document);
}

std::string
OmWritableDatabase::get_description() const
{
    DEBUGCALL(INTRO, std::string, "OmWritableDatabase::get_description", "");
    /// \todo display contents of the writable database
    RETURN("OmWritableDatabase()");
}
