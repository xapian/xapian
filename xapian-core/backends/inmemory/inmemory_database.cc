/* inmemory_database.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
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
#include <stdio.h>

#include "omdebug.h"
#include "inmemory_database.h"
#include "inmemory_document.h"
#include "inmemory_alltermslist.h"

#include <string>
#include <vector>
#include <map>
#include <list>

#include "xapian/error.h"

using std::make_pair;

//////////////
// Postlist //
//////////////

om_doclength
InMemoryPostList::get_doclength() const
{
    return db->get_doclength(get_docid());
}

PositionList *
InMemoryPostList::read_position_list()
{
    mypositions.set_data(pos->positions);
    return &mypositions;
}

PositionList *
InMemoryPostList::open_position_list() const
{
    return new InMemoryPositionList(pos->positions);
}

om_termcount
InMemoryPostList::get_wdf() const
{
    return (*pos).wdf;
}

///////////////////////////
// Actual database class //
///////////////////////////

InMemoryDatabase::InMemoryDatabase()
	: totdocs(0), totlen(0)
{
#if 0
    // FIXME: sort out his rather nasty error faking stuff
    //error_in_next = params.get_int("inmemory_errornext", 0);
    //abort_in_next = params.get_int("inmemory_abortnext", 0);
#endif
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
InMemoryDatabase::do_open_post_list(const string & tname) const
{
    if (!term_exists(tname)) {
	return new EmptyPostList();
    }

    map<string, InMemoryTerm>::const_iterator i = postlists.find(tname);
    Assert(i != postlists.end());

    return new InMemoryPostList(Xapian::Internal::RefCntPtr<const InMemoryDatabase>(this),
				i->second);
}

bool
InMemoryDatabase::doc_exists(om_docid did) const
{
    return (did > 0 && did <= termlists.size() && termlists[did - 1].is_valid);
}

LeafTermList *
InMemoryDatabase::open_term_list(om_docid did) const
{
    if (did == 0) throw Xapian::InvalidArgumentError("Docid 0 invalid");
    if (!doc_exists(did)) {
	// FIXME: the docid in this message will be local, not global
	throw Xapian::DocNotFoundError(string("Docid ") + om_tostring(did) +
				 string(" not found"));
    }
    return new InMemoryTermList(Xapian::Internal::RefCntPtr<const InMemoryDatabase>(this), did,
				termlists[did - 1], get_doclength(did));
}

Document *
InMemoryDatabase::open_document(om_docid did, bool /*lazy*/) const
{
    // we're never lazy so ignore that flag
    if (did == 0) throw Xapian::InvalidArgumentError("Docid 0 invalid");
    if (!doc_exists(did)) {
	// FIXME: the docid in this message will be local, not global
	throw Xapian::DocNotFoundError(string("Docid ") + om_tostring(did) +
				 string(" not found"));
    }
    return new InMemoryDocument(this, did, doclists[did - 1],
				valuelists[did - 1]);
}

PositionList * 
InMemoryDatabase::open_position_list(om_docid did,
				     const string & tname) const
{
    if (!doc_exists(did)) {
	throw Xapian::DocNotFoundError("Document id " + om_tostring(did) +
				 " doesn't exist in inmemory database");
    }
    const InMemoryDoc &doc = termlists[did-1];

    vector<InMemoryPosting>::const_iterator i;
    for (i = doc.terms.begin(); i != doc.terms.end(); ++i) {
	if (i->tname == tname) {
	    return new InMemoryPositionList(i->positions);
	}
    }
    throw Xapian::RangeError("No positionlist for term in document.");
}

void
InMemoryDatabase::add_values(om_docid /*did*/,
			     const map<om_valueno, string> &values_)
{
    valuelists.push_back(values_);
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
    throw Xapian::UnimplementedError("Transactions not implemented for InMemoryDatabase");
}

void
InMemoryDatabase::do_commit_transaction()
{
    throw Xapian::UnimplementedError("Transactions not implemented for InMemoryDatabase");
}

void
InMemoryDatabase::do_cancel_transaction()
{
    throw Xapian::UnimplementedError("Transactions not implemented for InMemoryDatabase");
}

void
InMemoryDatabase::do_delete_document(om_docid did)
{
    if (!doc_exists(did)) {
	throw Xapian::DocNotFoundError(string("Docid ") + om_tostring(did) +
				 string(" not found"));
    }
    termlists[did-1].is_valid = false;
    doclists[did-1] = "";
    valuelists[did-1].clear();
    totlen -= doclengths[did-1];
    doclengths[did-1] = 0;
    totdocs--;

    vector<InMemoryPosting>::const_iterator i;
    for (i = termlists[did - 1].terms.begin();
	 i != termlists[did - 1].terms.end();
	 ++i) {
	map<string, InMemoryTerm>::iterator t = postlists.find(i->tname);
	Assert(t != postlists.end());
	t->second.collection_freq -= i->wdf;
	vector<InMemoryPosting>::iterator posting = t->second.docs.begin();
	/* FIXME: inefficient on vectors... */
	while (posting != t->second.docs.end()) {
	    if (posting->did == did) {
		posting = t->second.docs.erase(posting);
	    } else {
		++posting;
	    }
	}
	if (t->second.docs.empty()) {
	    Assert(t->second.collection_freq == 0);
	    postlists.erase(t);
	}
    }
    termlists[did-1].terms.clear();
}

void
InMemoryDatabase::do_replace_document(om_docid did,
				      const OmDocument & document)
{
    DEBUGLINE(DB, "InMemoryDatabase::do_replace_document(): replaceing doc "
	          << did);

    do_delete_document(did);

    /* resurrect this document */
    termlists[did - 1] = InMemoryDoc();
    doclengths[did - 1] = 0;
    doclists[did - 1] = document.get_data();

    finish_add_doc(did, document);
}

om_docid
InMemoryDatabase::do_add_document(const OmDocument & document)
{
    om_docid did = make_doc(document.get_data());

    DEBUGLINE(DB, "InMemoryDatabase::do_add_document(): adding doc " << did);

    finish_add_doc(did, document);

    return did;
}

void
InMemoryDatabase::finish_add_doc(om_docid did, const OmDocument &document)
{
    {
	map<om_valueno, string> values;
	OmValueIterator k = document.values_begin();
	OmValueIterator k_end = document.values_end();
	for ( ; k != k_end; ++k) {
	    values.insert(make_pair(k.get_valueno(), *k));
	    DEBUGLINE(DB, "InMemoryDatabase::do_add_document(): adding value "
		      << k.get_valueno() << " -> " << *k);
	}
	add_values(did, values);
    }

    Xapian::TermIterator i = document.termlist_begin();
    Xapian::TermIterator i_end = document.termlist_end();
    for ( ; i != i_end; ++i) {
	make_term(*i);

	DEBUGLINE(DB, "InMemoryDatabase::do_add_document(): adding term "
		  << *i);
	Xapian::PositionListIterator j = i.positionlist_begin();
	Xapian::PositionListIterator j_end = i.positionlist_end();

	if (j == j_end) {
	    /* Make sure the posting exists, even without a position. */
	    make_posting(*i, did, 0, i.get_wdf(), false);
	} else {
	    for ( ; j != j_end; ++j) {
		make_posting(*i, did, *j, i.get_wdf());
	    }
	}

	Assert(did > 0 && did <= doclengths.size());
	doclengths[did - 1] += i.get_wdf();
	totlen += i.get_wdf();
	postlists[*i].collection_freq += i.get_wdf();
    }

    totdocs++;
}

void
InMemoryDatabase::make_term(const string & tname)
{
    postlists[tname];  // Initialise, if not already there.
}

om_docid
InMemoryDatabase::make_doc(const string & docdata)
{
    termlists.push_back(InMemoryDoc());
    doclengths.push_back(0);
    doclists.push_back(docdata);

    AssertParanoid(termlists.size() == doclengths.size());

    return termlists.size();
}

void InMemoryDatabase::make_posting(const string & tname,
				    om_docid did,
				    om_termpos position,
				    om_termcount wdf,
				    bool use_position)
{
    Assert(postlists.find(tname) != postlists.end());
    Assert(did > 0 && did <= termlists.size());
    Assert(did > 0 && did <= doclengths.size());
    Assert(doc_exists(did));

    // Make the posting
    InMemoryPosting posting;
    posting.tname = tname;
    posting.did = did;
    if (use_position) {
	posting.positions.push_back(position);
    }
    posting.wdf = wdf;

    // Now record the posting
    postlists[tname].add_posting(posting);
    termlists[did - 1].add_posting(posting);
}

bool
InMemoryDatabase::term_exists(const string & tname) const
{
    Assert(tname.size() != 0);
    return postlists.find(tname) != postlists.end();
}

TermList *
InMemoryDatabase::open_allterms() const
{
    return new InMemoryAllTermsList(&postlists,
				    Xapian::Internal::RefCntPtr<const InMemoryDatabase>(this));
}
