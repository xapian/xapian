/* inmemory_database.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004 Olly Betts
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

#include <xapian/error.h>
#include <xapian/valueiterator.h>

using std::make_pair;

//////////////
// Postlist //
//////////////

Xapian::doclength
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

Xapian::termcount
InMemoryPostList::get_wdf() const
{
    return (*pos).wdf;
}

///////////////////////////
// Actual database class //
///////////////////////////

InMemoryDatabase::InMemoryDatabase()
	: totdocs(0), totlen(0), positions_present(false)
{
#if 0
    // FIXME: sort out his rather nasty error faking stuff
    //error_in_next = params.get_int("inmemory_errornext", 0);
    //abort_in_next = params.get_int("inmemory_abortnext", 0);
#endif
}

InMemoryDatabase::~InMemoryDatabase()
{
    dtor_called();
}

LeafPostList *
InMemoryDatabase::do_open_post_list(const string & tname) const
{
    Assert(tname.size() != 0);
    map<string, InMemoryTerm>::const_iterator i = postlists.find(tname);
    if (i == postlists.end() || i->second.term_freq == 0)
	return new EmptyPostList();

    LeafPostList * pl;
    pl = new InMemoryPostList(Xapian::Internal::RefCntPtr<const InMemoryDatabase>(this),
			      i->second);
    Assert(!pl->at_end());
    return pl;
}

bool
InMemoryDatabase::doc_exists(Xapian::docid did) const
{
    return (did > 0 && did <= termlists.size() && termlists[did - 1].is_valid);
}

bool
InMemoryDatabase::has_positions() const
{
    return positions_present;
}

LeafTermList *
InMemoryDatabase::open_term_list(Xapian::docid did) const
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

Xapian::Document::Internal *
InMemoryDatabase::open_document(Xapian::docid did, bool /*lazy*/) const
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
InMemoryDatabase::open_position_list(Xapian::docid did,
				     const string & tname) const
{
    if (!doc_exists(did)) {
	throw Xapian::DocNotFoundError("Document id " + om_tostring(did) +
				 " doesn't exist in inmemory database");
    }
    const InMemoryDoc &doc = termlists[did-1];

    vector<InMemoryTermEntry>::const_iterator i;
    for (i = doc.terms.begin(); i != doc.terms.end(); ++i) {
	if (i->tname == tname) {
	    return new InMemoryPositionList(i->positions);
	}
    }
    throw Xapian::RangeError("No positionlist for term in document.");
}

void
InMemoryDatabase::add_values(Xapian::docid /*did*/,
			     const map<Xapian::valueno, string> &values_)
{
    valuelists.push_back(values_);
}

void
InMemoryDatabase::flush()
{
}

void
InMemoryDatabase::delete_document(Xapian::docid did)
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
    // A crude check, but it's hard to be more precise with the current
    // InMemory structure without being very inefficient.
    if (totdocs == 0) positions_present = false;

    vector<InMemoryTermEntry>::const_iterator i;
    for (i = termlists[did - 1].terms.begin();
	 i != termlists[did - 1].terms.end();
	 ++i) {
	map<string, InMemoryTerm>::iterator t = postlists.find(i->tname);
	Assert(t != postlists.end());
	t->second.collection_freq -= i->wdf;
	--t->second.term_freq;
	vector<InMemoryPosting>::iterator posting = t->second.docs.begin();
	while (posting != t->second.docs.end()) {
	    // Just zero out erased doc ids - otherwise we need to erase
	    // in a vector (inefficient) and we break any posting lists
	    // iterating over this posting list.
	    if (posting->did == did) posting->valid = false;
	    ++posting;
	}
    }
    termlists[did-1].terms.clear();
}

void
InMemoryDatabase::replace_document(Xapian::docid did,
				   const Xapian::Document & document)
{
    DEBUGLINE(DB, "InMemoryDatabase::replace_document(): replacing doc "
	          << did);

    if (doc_exists(did)) { 
	doclists[did - 1] = "";
	valuelists[did - 1].clear();
	totlen -= doclengths[did - 1];
	totdocs--;
    } else if (did > termlists.size()) {
	termlists.resize(did);
	doclengths.resize(did);
	doclists.resize(did);
	valuelists.resize(did);
    }

    vector<InMemoryTermEntry>::const_iterator i;
    for (i = termlists[did - 1].terms.begin();
	 i != termlists[did - 1].terms.end();
	 ++i) {
	map<string, InMemoryTerm>::iterator t = postlists.find(i->tname);
	Assert(t != postlists.end());
	t->second.collection_freq -= i->wdf;
	--t->second.term_freq;
	vector<InMemoryPosting>::iterator posting = t->second.docs.begin();
	while (posting != t->second.docs.end()) {
	    // Just invalidate erased doc ids - otherwise we need to erase
	    // in a vector (inefficient) and we break any posting lists
	    // iterating over this posting list.
	    if (posting->did == did) posting->valid = false;
	    ++posting;
	}
    }
    termlists[did - 1] = InMemoryDoc();

    doclengths[did - 1] = 0;
    doclists[did - 1] = document.get_data();

    finish_add_doc(did, document);
}

Xapian::docid
InMemoryDatabase::add_document(const Xapian::Document & document)
{
    Xapian::docid did = make_doc(document.get_data());

    DEBUGLINE(DB, "InMemoryDatabase::add_document(): adding doc " << did);

    finish_add_doc(did, document);

    return did;
}

void
InMemoryDatabase::finish_add_doc(Xapian::docid did, const Xapian::Document &document)
{
    {
	map<Xapian::valueno, string> values;
	Xapian::ValueIterator k = document.values_begin();
	Xapian::ValueIterator k_end = document.values_end();
	for ( ; k != k_end; ++k) {
	    values.insert(make_pair(k.get_valueno(), *k));
	    DEBUGLINE(DB, "InMemoryDatabase::add_document(): adding value "
		      << k.get_valueno() << " -> " << *k);
	}
	add_values(did, values);
    }

    Xapian::TermIterator i = document.termlist_begin();
    Xapian::TermIterator i_end = document.termlist_end();
    for ( ; i != i_end; ++i) {
	make_term(*i);

	DEBUGLINE(DB, "InMemoryDatabase::add_document(): adding term "
		  << *i);
	Xapian::PositionIterator j = i.positionlist_begin();
	Xapian::PositionIterator j_end = i.positionlist_end();

	if (j == j_end) {
	    /* Make sure the posting exists, even without a position. */
	    make_posting(*i, did, 0, i.get_wdf(), false);
	} else {
	    positions_present = true;
	    for ( ; j != j_end; ++j) {
		make_posting(*i, did, *j, i.get_wdf());
	    }
	}

	Assert(did > 0 && did <= doclengths.size());
	doclengths[did - 1] += i.get_wdf();
	totlen += i.get_wdf();
	postlists[*i].collection_freq += i.get_wdf();
	++postlists[*i].term_freq;
    }

    totdocs++;
}

void
InMemoryDatabase::make_term(const string & tname)
{
    postlists[tname];  // Initialise, if not already there.
}

Xapian::docid
InMemoryDatabase::make_doc(const string & docdata)
{
    termlists.push_back(InMemoryDoc());
    doclengths.push_back(0);
    doclists.push_back(docdata);

    AssertParanoid(termlists.size() == doclengths.size());

    return termlists.size();
}

void InMemoryDatabase::make_posting(const string & tname,
				    Xapian::docid did,
				    Xapian::termpos position,
				    Xapian::termcount wdf,
				    bool use_position)
{
    Assert(postlists.find(tname) != postlists.end());
    Assert(did > 0 && did <= termlists.size());
    Assert(did > 0 && did <= doclengths.size());
    Assert(doc_exists(did));

    // Make the posting
    InMemoryPosting posting;
    posting.did = did;
    if (use_position) {
	posting.positions.push_back(position);
    }
    posting.wdf = wdf;
    posting.valid = true;

    // Now record the posting
    postlists[tname].add_posting(posting);

    // Make the termentry
    InMemoryTermEntry termentry;
    termentry.tname = tname;
    if (use_position) {
	termentry.positions.push_back(position);
    }
    termentry.wdf = wdf;

    // Now record the termentry
    termlists[did - 1].add_posting(termentry);
}

bool
InMemoryDatabase::term_exists(const string & tname) const
{
    Assert(tname.size() != 0);
    map<string, InMemoryTerm>::const_iterator i = postlists.find(tname);
    if (i == postlists.end()) return false;
    return (i->second.term_freq != 0);
}

TermList *
InMemoryDatabase::open_allterms() const
{
    return new InMemoryAllTermsList(&postlists,
				    Xapian::Internal::RefCntPtr<const InMemoryDatabase>(this));
}
