/* inmemory_database.cc
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

#include <stdio.h>

#include "omdebug.h"
#include "inmemory_database.h"
#include "inmemory_document.h"

#include <string>
#include <vector>
#include <map>
#include <list>

#include "om/omerror.h"

//////////////
// Postlist //
//////////////

om_doclength
InMemoryPostList::get_doclength() const
{
    return this_db->get_doclength(get_docid());
}

PositionList *
InMemoryPostList::get_position_list()
{
    mypositions.set_data(pos->positions);
    return &mypositions;
}

om_termcount
InMemoryPostList::get_wdf() const
{
    return (*pos).wdf;
}

///////////////////////////
// Actual database class //
///////////////////////////

InMemoryDatabase::InMemoryDatabase(const OmSettings & params, bool readonly)
	: totlen(0), die_in_next(false)
{
    if (!readonly) {
// FIXME:	throw OmInvalidArgumentError("InMemoryDatabase must be opened readonly.");
    }

    std::string errpos = params.get("inmemory_error", "");
    if (errpos == "") {
    } else if (errpos == "next") {
	die_in_next = true;
    } else {
	throw OmInvalidOperationError("Invalid position to cause error, "
				      + errpos);
    }
}

InMemoryDatabase::~InMemoryDatabase()
{
    try {
	internal_end_session();
    } catch (...) {
	// Ignore any exceptions, since we may be being called due to an
	// exception anyway.  internal_end_session() should have already
	// been called, in the normal course of events.
    }
}

LeafPostList *
InMemoryDatabase::do_open_post_list(const om_termname & tname) const
{
    Assert(term_exists(tname));

    std::map<om_termname, InMemoryTerm>::const_iterator i = postlists.find(tname);
    Assert(i != postlists.end());

    return new InMemoryPostList(RefCntPtr<const InMemoryDatabase>(RefCntPtrToThis(), this),
				i->second);
}

LeafTermList *
InMemoryDatabase::open_term_list(om_docid did) const
{
    if (did == 0) throw OmInvalidArgumentError("Docid 0 invalid");
    if (did > termlists.size()) {
	// FIXME: the docid in this message will be local, not global
	throw OmDocNotFoundError(std::string("Docid ") + om_tostring(did) +
				 std::string(" not found"));
    }
    return new InMemoryTermList(RefCntPtr<const InMemoryDatabase>(RefCntPtrToThis(), this),
				termlists[did - 1], get_doclength(did));
}

Document *
InMemoryDatabase::open_document(om_docid did) const
{
    if (did == 0) throw OmInvalidArgumentError("Docid 0 invalid");
    if (did > doclists.size()) {
	// FIXME: the docid in this message will be local, not global
	throw OmDocNotFoundError(std::string("Docid ") + om_tostring(did) +
				 std::string(" not found"));
    }
    return new InMemoryDocument(this, did, doclists[did - 1],
				keylists[did - 1]);
}

PositionList *
InMemoryDatabase::open_position_list(om_docid did,
				     const om_termname & tname) const
{
    throw OmUnimplementedError("InMemory databases do not support opening positionlist");
}

void
InMemoryDatabase::add_keys(om_docid did,
			   const std::map<om_keyno, OmKey> &keys_)
{
    Assert(keylists.size() == did - 1);
    keylists.push_back(keys_);
}

void
InMemoryDatabase::do_begin_session()
{
}

void
InMemoryDatabase::do_end_session()
{
}

void
InMemoryDatabase::do_flush()
{
}

void
InMemoryDatabase::do_begin_transaction()
{
    throw OmUnimplementedError("Transactions not implemented for InMemoryDatabase");
}

void
InMemoryDatabase::do_commit_transaction()
{
    throw OmUnimplementedError("Transactions not implemented for InMemoryDatabase");
}

void
InMemoryDatabase::do_cancel_transaction()
{
    throw OmUnimplementedError("Transactions not implemented for InMemoryDatabase");
}


void
InMemoryDatabase::do_delete_document(om_docid did)
{
    throw OmUnimplementedError("InMemoryDatabase::do_delete_document() not implemented");  
}

void
InMemoryDatabase::do_replace_document(om_docid did,
				      const OmDocument & document)
{
    throw OmUnimplementedError("InMemoryDatabase::do_replace_document() not implemented");  
}

om_docid
InMemoryDatabase::do_add_document(const OmDocument & document)
{
    om_docid did = make_doc(document.get_data());
 
    {
	std::map<om_keyno, OmKey> keys;
	OmKeyListIterator k = document.keylist_begin();
	OmKeyListIterator k_end = document.keylist_end();
	for ( ; k != k_end; ++k) {
	    keys.insert(std::make_pair(k.get_keyno(), *k));
	}
	add_keys(did, keys);
    }

    OmTermListIterator i = document.termlist_begin();
    OmTermListIterator i_end = document.termlist_end();
    for ( ; i != i_end; ++i) {
	make_term(*i);

	OmPositionListIterator j = i.positionlist_begin();
	OmPositionListIterator j_end = i.positionlist_end();
	for ( ; j != j_end; ++j) {
	    make_posting(*i, did, *j, i.get_wdf());
	}

	Assert(did > 0 && did <= doclengths.size());
	doclengths[did - 1] += i.get_wdf();
	totlen += i.get_wdf();
    }

    return did;
}

void
InMemoryDatabase::make_term(const om_termname & tname)
{
    postlists[tname];  // Initialise, if not already there.
}

om_docid
InMemoryDatabase::make_doc(const OmData & docdata)
{
    termlists.push_back(InMemoryDoc());
    doclengths.push_back(0);
    doclists.push_back(docdata.value);

    AssertParanoid(termlists.size() == doclengths.size());

    return termlists.size();
}

void InMemoryDatabase::make_posting(const om_termname & tname,
				    om_docid did,
				    om_termpos position,
				    om_termcount wdf)
{
    Assert(postlists.find(tname) != postlists.end());
    Assert(did > 0 && did <= termlists.size());
    Assert(did > 0 && did <= doclengths.size());

    // Make the posting
    InMemoryPosting posting;
    posting.tname = tname;
    posting.did = did;
    posting.positions.push_back(position);
    posting.wdf = wdf;

    // Now record the posting
    postlists[tname].add_posting(posting);
    termlists[did - 1].add_posting(posting);
}

bool
InMemoryDatabase::term_exists(const om_termname & tname) const
{
    //DebugMsg("InMemoryDatabase::term_exists(`" << tname.c_str() << "'): ");
    Assert(tname.size() != 0);
    std::map<om_termname, InMemoryTerm>::const_iterator p = postlists.find(tname);

    if (p == postlists.end()) {
	//DebugMsg("not found" << endl);
	return false;
    }
    //DebugMsg("found" << endl);
    return true;
}
