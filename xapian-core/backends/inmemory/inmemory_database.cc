/* inmemory_database.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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

#include "omassert.h"
#include "inmemory_database.h"
#include "inmemory_document.h"
#include "textfile_indexer.h"

#include <string>
#include <vector>
#include <map>
#include <list>

#include "om/omerror.h"

//////////////
// Postlist //
//////////////

om_weight
InMemoryPostList::get_weight() const
{
    Assert(started);
    Assert(!at_end());
    Assert(ir_wt != NULL);

    return ir_wt->get_sumpart((*pos).positions.size(), get_doclength());
}

om_doclength
InMemoryPostList::get_doclength() const
{
    return this_db->get_doclength(get_docid());
}

PositionList &
InMemoryPostList::get_position_list()
{
    throw OmUnimplementedError("InMemoryPostList::get_position_list() unimplemented");
}

///////////////////////////
// Actual database class //
///////////////////////////

InMemoryDatabase::InMemoryDatabase(const DatabaseBuilderParams & params)
	: totlen(0)
{
    // FIXME - do appropriate thing if readonly flag is set.

    // Check validity of parameters
#if 0
    if(params.paths.size() != 0) {
	throw OmInvalidArgumentError("InMemoryDatabase expects no parameters.");
    }
#endif
    if(params.subdbs.size() != 0) {
	throw OmInvalidArgumentError("InMemoryDatabase cannot have sub databases.");
    }

#if 1
    if(params.paths.size() != 0) {
	TextfileIndexer indexer;
	indexer.set_destination(this);

	for(vector<string>::const_iterator p = params.paths.begin();
	    p != params.paths.end(); p++) {
	    TextfileIndexerSource source(*p);
	    DebugMsg("Indexing `" << *p << "'" << endl);
	    indexer.add_source(source);
	}

	// Make sure that there's at least one document
	if(postlists.size() <= 0)
	    throw OmOpeningError("Document was empty or nearly empty - nothing to search");
    }
#endif
}

InMemoryDatabase::~InMemoryDatabase()
{
}

LeafPostList *
InMemoryDatabase::open_post_list(const om_termname & tname) const
{
    Assert(term_exists(tname));

    map<om_termname, InMemoryTerm>::const_iterator i = postlists.find(tname);
    Assert(i != postlists.end());

    return new InMemoryPostList(this, i->second);
}

LeafTermList *
InMemoryDatabase::open_term_list(om_docid did) const
{
    Assert(did > 0);
    if(did > termlists.size()) {
	// FIXME: the docid in this message will be local, not global, in
	// the case of a multidatabase
	throw OmDocNotFoundError(string("Docid ") + inttostring(did) +
				 string(" not found"));
    }

    return new InMemoryTermList(this, termlists[did - 1], get_doclength(did));
}

LeafDocument *
InMemoryDatabase::open_document(om_docid did) const
{
    Assert(did > 0 && did <= doclists.size());

    return new InMemoryDocument(doclists[did - 1], keylists[did - 1]);
}

void
InMemoryDatabase::add_keys(om_docid did,
	      const OmDocumentContents::document_keys &keys_)
{
    vector<OmKey> keys;
    OmDocumentContents::document_keys::const_iterator i;
    i = keys_.begin();
    om_keyno this_keyno = 0;
    while (i != keys_.end()) {
	while (this_keyno < i->first) {
	    keys.push_back(OmKey());
	    ++this_keyno;
	}
	keys.push_back(i->second);
	++i, ++this_keyno;
    }
    Assert(keys.size() == this_keyno);
    Assert(keylists.size() == did-1);
    keylists.push_back(keys);
}

om_docid
InMemoryDatabase::add_document(const struct OmDocumentContents & document)
{
    om_docid did = make_doc(document.data);
    add_keys(did, document.keys);

    
    OmDocumentContents::document_terms::const_iterator i;
    for(i = document.terms.begin(); i != document.terms.end(); i++) {
	make_term(i->second.tname);

	OmDocumentTerm::term_positions::const_iterator j;
	for(j = i->second.positions.begin();
	    j != i->second.positions.end(); j++) {
	    make_posting(i->second.tname, did, *j);
	}

	// FIXME: set the wdf
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
				    om_termpos position)
{
    Assert(postlists.find(tname) != postlists.end());
    Assert(did > 0 && did <= termlists.size());
    Assert(did > 0 && did <= doclengths.size());

    // Make the posting
    InMemoryPosting posting;
    posting.tname = tname;
    posting.did = did;
    posting.positions.push_back(position);

    // Now record the posting
    postlists[tname].add_posting(posting);
    termlists[did - 1].add_posting(posting);
    doclengths[did - 1] += posting.positions.size();
    totlen += posting.positions.size();
}

bool
InMemoryDatabase::term_exists(const om_termname & tname) const
{
    //DebugMsg("InMemoryDatabase::term_exists(`" << tname.c_str() << "'): ");
    map<om_termname, InMemoryTerm>::const_iterator p = postlists.find(tname);

    if (p == postlists.end()) {
	//DebugMsg("not found" << endl);
	return false;
    }
    //DebugMsg("found" << endl);
    return true;
}
