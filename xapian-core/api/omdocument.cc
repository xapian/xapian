/* omdocument.cc: class for performing a match
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

#include <om/omdocument.h>
#include "om/omtypes.h"
#include "refcnt.h"
#include "document.h"
#include "omdocumentinternal.h"

//////////////////////////////////
// implementation of OmDocument //
//////////////////////////////////

OmDocument::OmDocument(OmDocument::Internal *internal_) : internal(internal_)
{
}

OmKey
OmDocument::get_key(om_keyno key) const
{
    DEBUGAPICALL(OmKey, "OmDocument::get_data", key);
    if (internal->keys_here) {
	std::map<om_keyno, OmKey>::const_iterator i;
	i = internal->keys.find(key);
	if (i == internal->keys.end()) {
	    OmKey nul;
	    RETURN(nul);
	}
	RETURN(i->second);
    }
    // create our own RefCntPtr in case another thread assigns a new ptr
    RefCntPtr<LeafDocument> myptr = internal->ptr;
    RETURN(myptr->get_key(key));
}

OmData
OmDocument::get_data() const
{
    DEBUGAPICALL(OmData, "OmDocument::get_data", "");
    if (internal->data_here) RETURN(internal->data);
    // create our own RefCntPtr in case another thread assigns a new ptr
    RefCntPtr<LeafDocument> myptr = internal->ptr;
    RETURN(myptr->get_data());
}

void
OmDocument::set_data(const OmData &data)
{
    DEBUGAPICALL(void, "OmDocument::set_data", data);
    // FIXME: locking???
    internal->data = data;
    internal->data_here = true;
}

void
OmDocument::operator=(const OmDocument &other)
{
    // pointers are reference counted.
    internal->ptr = other.internal->ptr;
}

OmDocument::OmDocument(const OmDocument &other)
	: internal(0)
{
    internal = new Internal(*other.internal);
}

OmDocument::~OmDocument()
{
    delete internal;
}

std::string
OmDocument::get_description() const
{
    // FIXME - return document contents
    return "OmDocument()";
//    description = "OmDocumentContents(" +
//	    data.get_description() +
//	    ", keys[" + om_tostring(keys.size()) + "]" +
//	    ", terms[" + om_tostring(terms.size()) + "]" +
//	    ")";
}

void
OmDocument::add_key(om_keyno keyno, const OmKey &key)
{
    DEBUGAPICALL(void, "OmDocument::add_key", keyno << ", " << key);
    // FIXME: need to lock here...
    if (!internal->keys_here) {
	internal->keys = internal->ptr->get_all_keys();
	internal->keys_here = true;
    }
    internal->keys.insert(std::make_pair(keyno, key));	
}

void
OmDocument::remove_key(om_keyno keyno)
{
    DEBUGAPICALL(void, "OmDocument::remove_key", keyno);
    // FIXME: need to lock here...
    if (!internal->keys_here) {
	internal->keys = internal->ptr->get_all_keys();
	internal->keys_here = true;
    }
    internal->keys.erase(keyno);
}

void
OmDocument::clear_keys()
{
    DEBUGAPICALL(void, "OmDocument::clear_keys", "");
    if (internal->keys_here) {
	internal->keys.clear();
    } else {
	internal->keys_here = true;
    }
}

void
OmDocument::add_posting(const om_termname & tname, om_termpos tpos)
{
    DEBUGAPICALL(void, "OmDocument::add_posting", tname << ", " << tpos);
    // FIXME: need to lock here...
    if (!internal->terms_here) {
	// FIXME: read terms from LeafDocument into terms
	Assert(false);
	internal->terms_here = true;
    }
    std::map<om_termname, OmDocumentTerm>::iterator i;
    i = internal->terms.find(tname);

    if (i == internal->terms.end()) {
	internal->terms.insert(std::make_pair(tname,
					      OmDocumentTerm(tname, tpos)));
    } else {
	i->second.add_posting(tpos);
    }
}

void
OmDocument::remove_posting(const om_termname & tname, om_termpos tpos)
{
    DEBUGAPICALL(void, "OmDocument::remove_posting", tname << ", " << tpos);
    // FIXME: need to lock here...
    if (!internal->terms_here) {
	// FIXME: read terms from LeafDocument into terms
	Assert(false);
	internal->terms_here = true;
    }
    std::map<om_termname, OmDocumentTerm>::iterator i;
    i = internal->terms.find(tname);

    if (i == internal->terms.end()) return;
    
    // FIXME: implement
    // i->second.remove_posting(tpos);
    // if (no postings left) internal->terms.erase(i);
    Assert(0);
}

void
OmDocument::remove_term(const om_termname & tname)
{
    DEBUGAPICALL(void, "OmDocument::remove_term", tname);
    // FIXME: need to lock here...
    if (!internal->terms_here) {
	// FIXME: read terms from LeafDocument into terms
	Assert(false);
	internal->terms_here = true;
    }
    internal->terms.erase(tname);
}


void
OmDocument::clear_terms()
{
    DEBUGAPICALL(void, "OmDocument::clear_terms", "");
    if (internal->terms_here) {
	internal->terms.clear();
    } else {
	internal->terms_here = true;
    }
}
