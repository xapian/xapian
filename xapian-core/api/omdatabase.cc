/* omdatabase.cc: External interface for running queries
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#include "om/omerror.h"
#include "omdatabaseinternal.h"
#include "omdebug.h"
#include "om/ompostlistiterator.h"
#include "ompostlistiteratorinternal.h"
#include "om/omtermlistiterator.h"
#include "omtermlistiteratorinternal.h"
#include "om/ompositionlistiterator.h"
#include "ompositionlistiteratorinternal.h"
#include "om/omoutput.h"

OmDatabase::OmDatabase()
{
    DEBUGAPICALL(void, "OmDatabase::OmDatabase", "");
    internal = new OmDatabase::Internal();
}

OmDatabase::OmDatabase(const OmSettings & params, bool readonly)
	: internal(new OmDatabase::Internal(params, readonly))
{
    DEBUGAPICALL(void, "OmDatabase::OmDatabase", params << ", " << readonly);
}

OmDatabase::OmDatabase(const OmSettings & params)
	: internal(new OmDatabase::Internal(params, true))
{
    DEBUGAPICALL(void, "OmDatabase::OmDatabase", params);
}

OmDatabase::OmDatabase(const OmDatabase &other)
	: internal(0)
{
    DEBUGAPICALL(void, "OmDatabase::OmDatabase", "OmDatabase");
    OmLockSentry locksentry(other.internal->mutex);

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

    // we get these locks in a defined order to avoid deadlock
    // should two threads try to assign two databases to each
    // other at the same time.
    Internal * newinternal;

    {
	OmLockSentry locksentry1(std::min(internal, other.internal)->mutex);
	OmLockSentry locksentry2(std::max(internal, other.internal)->mutex);

	newinternal = new Internal(*(other.internal));

	std::swap(internal, newinternal);
    }

    delete newinternal;
}

OmDatabase::~OmDatabase()
{
    DEBUGAPICALL(void, "OmDatabase::~OmDatabase", "");
    delete internal;
    internal = 0;
}

void
OmDatabase::add_database(const OmSettings &params)
{
    DEBUGAPICALL(void, "OmDatabase::add_database", params);
    internal->add_database(params);
}

void
OmDatabase::add_database(const OmDatabase & database)
{
    DEBUGAPICALL(void, "OmDatabase::add_database", "OmDatabase");
    if (this == &database) {
	DEBUGLINE(API, "OmDatabase added to itself");
	throw OmInvalidArgumentError("Can't add an OmDatabase to itself");
	return;
    }
    std::vector<RefCntPtr<Database> >::iterator i;
    OmLockSentry locksentry(database.internal->mutex);
    for (i = database.internal->databases.begin();
	 i != database.internal->databases.end(); i++) {
	internal->add_database(*i);
    }
}

OmPostListIterator
OmDatabase::postlist_begin(const om_termname &tname) const
{
    DEBUGAPICALL(OmPostListIterator, "OmDatabase::postlist_begin", tname);
    if (tname.empty()) throw OmInvalidArgumentError("Zero length terms are invalid");
    RETURN(OmPostListIterator(new OmPostListIterator::Internal(internal->open_post_list(tname, *this))));
}

OmPostListIterator
OmDatabase::postlist_end(const om_termname &tname) const
{
    DEBUGAPICALL(OmPostListIterator, "OmDatabase::postlist_end", tname);
    if (tname.empty()) throw OmInvalidArgumentError("Zero length terms are invalid");
    RETURN(OmPostListIterator(NULL));
}

OmTermListIterator
OmDatabase::termlist_begin(om_docid did) const
{
    DEBUGAPICALL(OmTermListIterator, "OmDatabase::termlist_begin", did);
    if (did == 0) throw OmInvalidArgumentError("Document IDs of 0 are invalid");
    RETURN(OmTermListIterator(new OmTermListIterator::Internal(internal->open_term_list(did, *this))));
}

OmTermListIterator
OmDatabase::termlist_end(om_docid did) const
{
    DEBUGAPICALL(OmTermListIterator, "OmDatabase::termlist_end", did);
    if (did == 0) throw OmInvalidArgumentError("Document IDs of 0 are invalid");
    RETURN(OmTermListIterator(NULL));
}

OmPositionListIterator
OmDatabase::positionlist_begin(om_docid did, const om_termname &tname) const
{
    DEBUGAPICALL(OmPositionListIterator, "OmDatabase::positionlist_begin",
		 did << ", " << tname);
    if (tname.empty()) throw OmInvalidArgumentError("Zero length terms are invalid");
    if (did == 0) throw OmInvalidArgumentError("Document IDs of 0 are invalid");
    throw OmUnimplementedError("positionlist_begin() needs backends to support get_position_list on databases");
    //RETURN(OmPositionListIterator(new OmPositionListIterator::Internal( ... )));
}

OmPositionListIterator
OmDatabase::positionlist_end(om_docid did, const om_termname &tname) const
{
    DEBUGAPICALL(OmPositionListIterator, "OmDatabase::positionlist_end",
		 did << ", " << tname);
    if (tname.empty()) throw OmInvalidArgumentError("Zero length terms are invalid");
    if (did == 0) throw OmInvalidArgumentError("Document IDs of 0 are invalid");
    throw OmUnimplementedError("positionlist_end() needs backends to support get_position_list on databases");
    RETURN(OmPositionListIterator(NULL));
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
OmDatabase::get_termfreq(const om_termname & tname) const
{
    DEBUGAPICALL(om_doccount, "OmDatabase::get_termfreq", tname);
    if (tname.empty())
	throw OmInvalidArgumentError("Zero length terms are invalid");
    om_doccount tf = 0;
    std::vector<RefCntPtr<Database> >::const_iterator i;
    for (i = internal->databases.begin(); i != internal->databases.end(); i++) {
	tf += (*i)->get_termfreq(tname);
    }
    RETURN(tf);
}

om_doclength
OmDatabase::get_doclength(om_docid did) const
{
    DEBUGAPICALL(om_doclength, "OmDatabase::get_doclength", did);
    if (did == 0) throw OmInvalidArgumentError("Document IDs of 0 are invalid");
    unsigned int multiplier = internal->databases.size();
    om_docid realdid = (did - 1) / multiplier + 1;
    om_doccount dbnumber = (did - 1) % multiplier;
    RETURN(internal->databases[dbnumber]->get_doclength(realdid));
}

bool
OmDatabase::term_exists(const om_termname & tname) const
{
    if (tname.empty())
	throw OmInvalidArgumentError("Zero length terms are invalid");
    std::vector<RefCntPtr<Database> >::const_iterator i;
    for (i = internal->databases.begin(); i != internal->databases.end(); i++) {
	if ((*i)->term_exists(tname)) return true;
    }
    return false;
}

std::string
OmDatabase::get_description() const
{
    DEBUGAPICALL(std::string, "OmDatabase::get_description", "");
    /// \todo display contents of the database
    RETURN("OmDatabase()");
}


OmWritableDatabase::OmWritableDatabase(const OmSettings & params)
	: OmDatabase(params, false)
{
    DEBUGAPICALL(void, "OmWritableDatabase::OmWritableDatabase", params);
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
    delete internal;
    internal = 0;
}

void
OmWritableDatabase::begin_session(om_timeout timeout)
{
    DEBUGAPICALL(void, "OmWritableDatabase::begin_session", timeout);
    // Get the pointer while locked, in case someone is assigning to it.
    Database * database;
    {
	OmLockSentry locksentry(internal->mutex);
	database = internal->databases[0].get();
    }

    database->begin_session(timeout);
}

void
OmWritableDatabase::end_session()
{
    DEBUGAPICALL(void, "OmWritableDatabase::end_session", "");
    // Get the pointer while locked, in case someone is assigning to it.
    Database * database;
    {
	OmLockSentry locksentry(internal->mutex);
	database = internal->databases[0].get();
    }

    database->end_session();
}

void
OmWritableDatabase::flush()
{
    DEBUGAPICALL(void, "OmWritableDatabase::flush", "");
    // Get the pointer while locked, in case someone is assigning to it.
    Database * database;
    {
	OmLockSentry locksentry(internal->mutex);
	database = internal->databases[0].get();
    }

    database->flush();
}

void
OmWritableDatabase::begin_transaction()
{
    DEBUGAPICALL(void, "OmWritableDatabase::begin_transaction", "");
    // Get the pointer while locked, in case someone is assigning to it.
    Database * database;
    {
	OmLockSentry locksentry(internal->mutex);
	database = internal->databases[0].get();
    }

    database->begin_transaction();
}

void
OmWritableDatabase::commit_transaction()
{
    DEBUGAPICALL(void, "OmWritableDatabase::commit_transaction", "");
    // Get the pointer while locked, in case someone is assigning to it.
    Database * database;
    {
	OmLockSentry locksentry(internal->mutex);
	database = internal->databases[0].get();
    }

    database->commit_transaction();
}

void
OmWritableDatabase::cancel_transaction()
{
    DEBUGAPICALL(void, "OmWritableDatabase::cancel_transaction", "");
    // Get the pointer while locked, in case someone is assigning to it.
    Database * database;
    {
	OmLockSentry locksentry(internal->mutex);
	database = internal->databases[0].get();
    }

    database->cancel_transaction();
}


om_docid
OmWritableDatabase::add_document(const OmDocumentContents & document,
				 om_timeout timeout)
{
    DEBUGAPICALL(om_docid, "OmWritableDatabase::add_document",
	       document << ", " << timeout);
    // Check the validity of the document
    OmDocumentContents::document_terms::const_iterator i;
    for(i = document.terms.begin(); i != document.terms.end(); i++) {
	if(i->second.tname.size() == 0) {
	    throw OmInvalidArgumentError(
		"Cannot add termnames of zero length to the database.");
	}
    }

    // Get the pointer while locked, in case someone is assigning to it.
    Database * database;
    {
	OmLockSentry locksentry(internal->mutex);
	database = internal->databases[0].get();
    }

    RETURN(database->add_document(document, timeout));
}

void
OmWritableDatabase::delete_document(om_docid did, om_timeout timeout)
{
    DEBUGAPICALL(void, "OmWritableDatabase::delete_document",
		 did << ", " << timeout);
    if (did == 0) throw OmInvalidArgumentError("Document IDs of 0 are invalid");
    // Get the pointer while locked, in case someone is assigning to it.
    Database * database;
    {
	OmLockSentry locksentry(internal->mutex);
	database = internal->databases[0].get();
    }

    database->delete_document(did, timeout);
}

void
OmWritableDatabase::replace_document(om_docid did,
				     const OmDocumentContents & document,
				     om_timeout timeout)
{
    DEBUGAPICALL(void, "OmWritableDatabase::replace_document",
		 did << ", " << document << ", " << timeout);
    if (did == 0) throw OmInvalidArgumentError("Document IDs of 0 are invalid");
    // Get the pointer while locked, in case someone is assigning to it.
    Database * database;
    {
	OmLockSentry locksentry(internal->mutex);
	database = internal->databases[0].get();
    }

    database->replace_document(did, document, timeout);
}

OmDocumentContents
OmWritableDatabase::get_document(om_docid did) const
{
    DEBUGAPICALL(OmDocumentContents, "OmWritableDatabase::get_document", did);
    if (did == 0) throw OmInvalidArgumentError("Document IDs of 0 are invalid");
    // Get the pointer while locked, in case someone is assigning to it.
    Database * database;
    {
	OmLockSentry locksentry(internal->mutex);
	database = internal->databases[0].get();
    }

    RETURN(database->get_document(did));
}

std::string
OmWritableDatabase::get_description() const
{
    DEBUGAPICALL(std::string, "OmWritableDatabase::get_description", "");
    /// \todo display contents of the writable database
    RETURN("OmWritableDatabase()");
}
