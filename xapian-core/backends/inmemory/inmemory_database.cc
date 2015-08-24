/* inmemory_database.cc
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010 Olly Betts
 * Copyright 2006,2009 Lemur Consulting Ltd
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

#include "inmemory_database.h"

#include "debuglog.h"

#include "expandweight.h"
#include "inmemory_document.h"
#include "inmemory_alltermslist.h"
#include "str.h"
#include "valuestats.h"

#include <string>
#include <vector>
#include <map>

#include <xapian/error.h>
#include <xapian/valueiterator.h>

using std::make_pair;

inline void
InMemoryTerm::add_posting(const InMemoryPosting & post)
{
    // Add document to right place in list
    vector<InMemoryPosting>::iterator p;
    p = lower_bound(docs.begin(), docs.end(),
		    post, InMemoryPostingLessThan());
    if (p == docs.end() || InMemoryPostingLessThan()(post, *p)) {
	docs.insert(p, post);
    } else if (!p->valid) {
	*p = post;
    } else {
	(*p).merge(post);
    }
}

inline void
InMemoryDoc::add_posting(const InMemoryTermEntry & post)
{
    // Add document to right place in list
    vector<InMemoryTermEntry>::iterator p;
    p = lower_bound(terms.begin(), terms.end(),
		    post, InMemoryTermEntryLessThan());
    if (p == terms.end() || InMemoryTermEntryLessThan()(post, *p)) {
	terms.insert(p, post);
    } else {
	(*p).merge(post);
    }
}

//////////////
// Postlist //
//////////////

InMemoryPostList::InMemoryPostList(Xapian::Internal::RefCntPtr<const InMemoryDatabase> db_,
				   const InMemoryTerm & imterm,
				   const std::string & term_)
	: LeafPostList(term_),
	  pos(imterm.docs.begin()),
	  end(imterm.docs.end()),
	  termfreq(imterm.term_freq),
	  started(false),
	  db(db_)
{
    while (pos != end && !pos->valid) ++pos;
}

Xapian::doccount
InMemoryPostList::get_termfreq() const
{
    return termfreq;
}

Xapian::docid
InMemoryPostList::get_docid() const
{
    if (db->is_closed()) InMemoryDatabase::throw_database_closed();
    Assert(started);
    Assert(!at_end());
    return (*pos).did;
}

PostList *
InMemoryPostList::next(Xapian::weight /*w_min*/)
{
    if (db->is_closed()) InMemoryDatabase::throw_database_closed();
    if (started) {
	Assert(!at_end());
	++pos;
	while (pos != end && !pos->valid) ++pos;
    } else {
	started = true;
    }
    return NULL;
}

PostList *
InMemoryPostList::skip_to(Xapian::docid did, Xapian::weight w_min)
{
    if (db->is_closed()) InMemoryDatabase::throw_database_closed();
    // FIXME - see if we can make more efficient, perhaps using better
    // data structure.  Note, though, that a binary search of
    // the remaining list may NOT be a good idea (search time is then
    // O(log {length of list}), as opposed to O(distance we want to skip)
    // Since we will frequently only be skipping a short distance, this
    // could well be worse.
    started = true;
    Assert(!at_end());
    while (!at_end() && (*pos).did < did) {
	(void) next(w_min);
    }
    return NULL;
}

bool
InMemoryPostList::at_end() const
{
    if (db->is_closed()) InMemoryDatabase::throw_database_closed();
    return (pos == end);
}

string
InMemoryPostList::get_description() const
{
    return "InMemoryPostList " + str(termfreq);
}

Xapian::termcount
InMemoryPostList::get_doclength() const
{
    if (db->is_closed()) InMemoryDatabase::throw_database_closed();
    return db->get_doclength(get_docid());
}

PositionList *
InMemoryPostList::read_position_list()
{
    if (db->is_closed()) InMemoryDatabase::throw_database_closed();
    mypositions.set_data(pos->positions);
    return &mypositions;
}

PositionList *
InMemoryPostList::open_position_list() const
{
    if (db->is_closed()) InMemoryDatabase::throw_database_closed();
    return new InMemoryPositionList(pos->positions);
}

Xapian::termcount
InMemoryPostList::get_wdf() const
{
    if (db->is_closed()) InMemoryDatabase::throw_database_closed();
    return (*pos).wdf;
}

//////////////
// Termlist //
//////////////

InMemoryTermList::InMemoryTermList(Xapian::Internal::RefCntPtr<const InMemoryDatabase> db_,
				   Xapian::docid did_,
				   const InMemoryDoc & doc,
				   Xapian::termcount len)
	: pos(doc.terms.begin()), end(doc.terms.end()), terms(doc.terms.size()),
	  started(false), db(db_), did(did_), document_length(len)
{
    LOGLINE(DB, "InMemoryTermList::InMemoryTermList(): " <<
	        terms << " terms starting from " << pos->tname);
}

Xapian::termcount
InMemoryTermList::get_wdf() const
{
    if (db->is_closed()) InMemoryDatabase::throw_database_closed();
    Assert(started);
    Assert(!at_end());
    return (*pos).wdf;
}

Xapian::doccount
InMemoryTermList::get_termfreq() const
{
    if (db->is_closed()) InMemoryDatabase::throw_database_closed();
    Assert(started);
    Assert(!at_end());

    return db->get_termfreq((*pos).tname);
}

Xapian::termcount
InMemoryTermList::get_approx_size() const
{
    if (db->is_closed()) InMemoryDatabase::throw_database_closed();
    return terms;
}

void
InMemoryTermList::accumulate_stats(Xapian::Internal::ExpandStats & stats) const
{
    if (db->is_closed()) InMemoryDatabase::throw_database_closed();
    Assert(started);
    Assert(!at_end());
    stats.accumulate(InMemoryTermList::get_wdf(), document_length,
		     InMemoryTermList::get_termfreq(),
		     db->get_doccount());
}

string
InMemoryTermList::get_termname() const
{
    if (db->is_closed()) InMemoryDatabase::throw_database_closed();
    Assert(started);
    Assert(!at_end());
    return (*pos).tname;
}

TermList *
InMemoryTermList::next()
{
    if (db->is_closed()) InMemoryDatabase::throw_database_closed();
    if (started) {
	Assert(!at_end());
	++pos;
    } else {
	started = true;
    }
    return NULL;
}

TermList *
InMemoryTermList::skip_to(const string & term)
{
    if (rare(db->is_closed()))
	InMemoryDatabase::throw_database_closed();

    while (pos != end && pos->tname < term) {
	++pos;
    }

    started = true;
    return NULL;
}

bool
InMemoryTermList::at_end() const
{
    if (db->is_closed()) InMemoryDatabase::throw_database_closed();
    Assert(started);
    return (pos == end);
}

Xapian::termcount
InMemoryTermList::positionlist_count() const
{
    if (db->is_closed()) InMemoryDatabase::throw_database_closed();
    return db->positionlist_count(did, (*pos).tname);
}

Xapian::PositionIterator
InMemoryTermList::positionlist_begin() const
{
    if (db->is_closed()) InMemoryDatabase::throw_database_closed();
    return Xapian::PositionIterator(db->open_position_list(did, (*pos).tname));
}

/////////////////////////////
// InMemoryAllDocsPostList //
/////////////////////////////

InMemoryAllDocsPostList::InMemoryAllDocsPostList(Xapian::Internal::RefCntPtr<const InMemoryDatabase> db_)
	: LeafPostList(std::string()), did(0), db(db_)
{
}

Xapian::doccount
InMemoryAllDocsPostList::get_termfreq() const
{
    if (db->is_closed()) InMemoryDatabase::throw_database_closed();
    return db->totdocs;
}

Xapian::docid
InMemoryAllDocsPostList::get_docid() const
{
    if (db->is_closed()) InMemoryDatabase::throw_database_closed();
    Assert(did > 0);
    Assert(did <= db->termlists.size());
    Assert(db->termlists[did - 1].is_valid);
    return did;
}

Xapian::termcount
InMemoryAllDocsPostList::get_doclength() const
{
    if (db->is_closed()) InMemoryDatabase::throw_database_closed();
    return db->get_doclength(did);
}

Xapian::termcount
InMemoryAllDocsPostList::get_wdf() const
{
    return 1;
}

PositionList *
InMemoryAllDocsPostList::read_position_list()
{
    throw Xapian::UnimplementedError("Can't open position list for all docs iterator");
}

PositionList *
InMemoryAllDocsPostList::open_position_list() const
{
    throw Xapian::UnimplementedError("Can't open position list for all docs iterator");
}

PostList *
InMemoryAllDocsPostList::next(Xapian::weight /*w_min*/)
{
    if (db->is_closed()) InMemoryDatabase::throw_database_closed();
    Assert(!at_end());
    do {
       ++did;
    } while (did <= db->termlists.size() && !db->termlists[did - 1].is_valid);
    return NULL;
}

PostList *
InMemoryAllDocsPostList::skip_to(Xapian::docid did_, Xapian::weight /*w_min*/)
{
    if (db->is_closed()) InMemoryDatabase::throw_database_closed();
    Assert(!at_end());
    if (did <= did_) {
	did = did_;
	while (did <= db->termlists.size() && !db->termlists[did - 1].is_valid) {
	    ++did;
	}
    }
    return NULL;
}

bool
InMemoryAllDocsPostList::at_end() const
{
    if (db->is_closed()) InMemoryDatabase::throw_database_closed();
    return (did > db->termlists.size());
}

string
InMemoryAllDocsPostList::get_description() const
{
    return "InMemoryAllDocsPostList " + str(did);
}

///////////////////////////
// Actual database class //
///////////////////////////

InMemoryDatabase::InMemoryDatabase()
	: totdocs(0), totlen(0), positions_present(false), closed(false)
{
    // Updates are applied immediately so we can't support transactions.
    transaction_state = TRANSACTION_UNIMPLEMENTED;

    // We keep an empty entry in postlists for convenience of implementing
    // allterms iteration and returning a PostList for an absent term.
    postlists.insert(make_pair(string(), InMemoryTerm()));
}

InMemoryDatabase::~InMemoryDatabase()
{
    dtor_called();
}

void
InMemoryDatabase::reopen()
{
    if (closed) InMemoryDatabase::throw_database_closed();
}

void
InMemoryDatabase::close()
{
    // Free all the resources, and mark the db as closed.
    postlists.clear();
    termlists.clear();
    doclists.clear();
    valuelists.clear();
    valuestats.clear();
    doclengths.clear();
    metadata.clear();
    closed = true;
}

LeafPostList *
InMemoryDatabase::open_post_list(const string & tname) const
{
    if (closed) InMemoryDatabase::throw_database_closed();
    if (tname.empty()) {
	Xapian::Internal::RefCntPtr<const InMemoryDatabase> ptrtothis(this);
	return new InMemoryAllDocsPostList(ptrtothis);
    }
    map<string, InMemoryTerm>::const_iterator i = postlists.find(tname);
    if (i == postlists.end() || i->second.term_freq == 0) {
	i = postlists.begin();
	// Check that our dummy entry for string() is present.
	Assert(i->first.empty());
    }
    Xapian::Internal::RefCntPtr<const InMemoryDatabase> ptrtothis(this);
    return new InMemoryPostList(ptrtothis, i->second, tname);
}

bool
InMemoryDatabase::doc_exists(Xapian::docid did) const
{
    if (closed) InMemoryDatabase::throw_database_closed();
    return (did > 0 && did <= termlists.size() && termlists[did - 1].is_valid);
}

Xapian::doccount
InMemoryDatabase::get_termfreq(const string & tname) const
{
    if (closed) InMemoryDatabase::throw_database_closed();
    map<string, InMemoryTerm>::const_iterator i = postlists.find(tname);
    if (i == postlists.end()) return 0;
    return i->second.term_freq;
}

Xapian::termcount
InMemoryDatabase::get_collection_freq(const string &tname) const
{
    if (closed) InMemoryDatabase::throw_database_closed();
    map<string, InMemoryTerm>::const_iterator i = postlists.find(tname);
    if (i == postlists.end()) return 0;
    return i->second.collection_freq;
}

Xapian::doccount
InMemoryDatabase::get_value_freq(Xapian::valueno slot) const
{
    if (closed) InMemoryDatabase::throw_database_closed();
    map<Xapian::valueno, ValueStats>::const_iterator i = valuestats.find(slot);
    if (i == valuestats.end()) return 0;
    return i->second.freq;
}

std::string
InMemoryDatabase::get_value_lower_bound(Xapian::valueno slot) const
{
    if (closed) InMemoryDatabase::throw_database_closed();
    map<Xapian::valueno, ValueStats>::const_iterator i = valuestats.find(slot);
    if (i == valuestats.end()) return string();
    return i->second.lower_bound;
}

std::string
InMemoryDatabase::get_value_upper_bound(Xapian::valueno slot) const
{
    if (closed) InMemoryDatabase::throw_database_closed();
    map<Xapian::valueno, ValueStats>::const_iterator i = valuestats.find(slot);
    if (i == valuestats.end()) return string();
    return i->second.upper_bound;
}

Xapian::doccount
InMemoryDatabase::get_doccount() const
{
    if (closed) InMemoryDatabase::throw_database_closed();
    return totdocs;
}

Xapian::docid
InMemoryDatabase::get_lastdocid() const
{
    if (closed) InMemoryDatabase::throw_database_closed();
    return termlists.size();
}

totlen_t
InMemoryDatabase::get_total_length() const
{
    return totlen;
}

Xapian::doclength
InMemoryDatabase::get_avlength() const
{
    if (closed) InMemoryDatabase::throw_database_closed();
    if (totdocs == 0) return 0;
    return Xapian::doclength(totlen) / totdocs;
}

Xapian::termcount
InMemoryDatabase::get_doclength(Xapian::docid did) const
{
    if (closed) InMemoryDatabase::throw_database_closed();
    if (!doc_exists(did)) {
	throw Xapian::DocNotFoundError(string("Docid ") + str(did) +
				 string(" not found"));
    }
    return doclengths[did - 1];
}

TermList *
InMemoryDatabase::open_term_list(Xapian::docid did) const
{
    if (closed) InMemoryDatabase::throw_database_closed();
    Assert(did != 0);
    if (!doc_exists(did)) {
	// FIXME: the docid in this message will be local, not global
	throw Xapian::DocNotFoundError(string("Docid ") + str(did) +
				 string(" not found"));
    }
    return new InMemoryTermList(Xapian::Internal::RefCntPtr<const InMemoryDatabase>(this), did,
				termlists[did - 1], doclengths[did - 1]);
}

Xapian::Document::Internal *
InMemoryDatabase::open_document(Xapian::docid did, bool lazy) const
{
    if (closed) InMemoryDatabase::throw_database_closed();
    Assert(did != 0);
    if (!doc_exists(did)) {
	if (lazy) return NULL;
	// FIXME: the docid in this message will be local, not global
	throw Xapian::DocNotFoundError(string("Docid ") + str(did) +
				 string(" not found"));
    }
    return new InMemoryDocument(this, did);
}

std::string
InMemoryDatabase::get_metadata(const std::string & key) const
{
    if (closed) InMemoryDatabase::throw_database_closed();
    map<string, string>::const_iterator i = metadata.find(key);
    if (i == metadata.end())
	return string();
    return i->second;
}
 
TermList *
InMemoryDatabase::open_metadata_keylist(const string &) const
{
    if (!closed && metadata.empty()) return NULL;
    // FIXME: nobody implemented this yet...
    throw Xapian::UnimplementedError("InMemory backend doesn't currently implement Database::metadata_keys_begin()");
}

void
InMemoryDatabase::set_metadata(const std::string & key,
			       const std::string & value)
{
    if (closed) InMemoryDatabase::throw_database_closed();
    if (!value.empty()) {
	metadata[key] = value;
    } else {
	metadata.erase(key);
    }
}

Xapian::termcount
InMemoryDatabase::positionlist_count(Xapian::docid did,
				     const string & tname) const
{
    if (closed) InMemoryDatabase::throw_database_closed();
    if (!doc_exists(did)) {
	return 0;
    }
    const InMemoryDoc &doc = termlists[did-1];

    vector<InMemoryTermEntry>::const_iterator i;
    for (i = doc.terms.begin(); i != doc.terms.end(); ++i) {
	if (i->tname == tname) {
	    return i->positions.size();
	}
    }
    return 0;
}

PositionList * 
InMemoryDatabase::open_position_list(Xapian::docid did,
				     const string & tname) const
{
    if (closed) InMemoryDatabase::throw_database_closed();
    if (usual(doc_exists(did))) {
	const InMemoryDoc &doc = termlists[did-1];

	vector<InMemoryTermEntry>::const_iterator i;
	for (i = doc.terms.begin(); i != doc.terms.end(); ++i) {
	    if (i->tname == tname) {
		return new InMemoryPositionList(i->positions);
	    }
	}
    }
    return new InMemoryPositionList(false);
}

void
InMemoryDatabase::add_values(Xapian::docid did,
			     const map<Xapian::valueno, string> &values_)
{
    if (closed) InMemoryDatabase::throw_database_closed();
    if (did > valuelists.size()) {
	valuelists.resize(did);
    }
    valuelists[did-1] = values_;

    // Update the statistics.
    map<Xapian::valueno, string>::const_iterator j;
    for (j = values_.begin(); j != values_.end(); ++j) {
	std::pair<map<Xapian::valueno, ValueStats>::iterator, bool> i;
	i = valuestats.insert(make_pair(j->first, ValueStats()));

	// Now, modify the stored statistics.
	if ((i.first->second.freq)++ == 0) {
	    // If the value count was previously zero, set the upper and lower
	    // bounds to the newly added value.
	    i.first->second.lower_bound = j->second;
	    i.first->second.upper_bound = j->second;
	} else {
	    // Otherwise, simply make sure they reflect the new value.
	    if (j->second < i.first->second.lower_bound) {
		i.first->second.lower_bound = j->second;
	    }
	    if (j->second > i.first->second.upper_bound) {
		i.first->second.upper_bound = j->second;
	    }
	}
    }
}

// We implicitly commit each modification right away, so nothing to do here.
void
InMemoryDatabase::commit()
{
}

// We implicitly commit each modification right away, so nothing to do here.
void
InMemoryDatabase::cancel()
{
}

void
InMemoryDatabase::delete_document(Xapian::docid did)
{
    if (closed) InMemoryDatabase::throw_database_closed();
    if (!doc_exists(did)) {
	throw Xapian::DocNotFoundError(string("Docid ") + str(did) +
				 string(" not found"));
    }
    termlists[did-1].is_valid = false;
    doclists[did-1] = string();
    map<Xapian::valueno, string>::const_iterator j;
    for (j = valuelists[did-1].begin(); j != valuelists[did-1].end(); ++j) {
	map<Xapian::valueno, ValueStats>::iterator i;
	i = valuestats.find(j->first);
	if (--(i->second.freq) == 0) {
	    i->second.lower_bound.resize(0);
	    i->second.upper_bound.resize(0);
	}
    }
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
    LOGCALL_VOID(DB, "InMemoryDatabase::replace_document", did | document);

    if (closed) InMemoryDatabase::throw_database_closed();

    if (doc_exists(did)) { 
	map<Xapian::valueno, string>::const_iterator j;
	for (j = valuelists[did-1].begin(); j != valuelists[did-1].end(); ++j) {
	    map<Xapian::valueno, ValueStats>::iterator i;
	    i = valuestats.find(j->first);
	    if (--(i->second.freq) == 0) {
		i->second.lower_bound.resize(0);
		i->second.upper_bound.resize(0);
	    }
	}

	totlen -= doclengths[did - 1];
	totdocs--;
    } else if (did > termlists.size()) {
	termlists.resize(did);
	termlists[did - 1].is_valid = true;
	doclengths.resize(did);
	doclists.resize(did);
	valuelists.resize(did);
    } else {
	termlists[did - 1].is_valid = true;
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

    doclengths[did - 1] = 0;
    doclists[did - 1] = document.get_data();

    finish_add_doc(did, document);
}

Xapian::docid
InMemoryDatabase::add_document(const Xapian::Document & document)
{
    LOGCALL(DB, Xapian::docid, "InMemoryDatabase::add_document", document);
    if (closed) InMemoryDatabase::throw_database_closed();

    Xapian::docid did = make_doc(document.get_data());

    finish_add_doc(did, document);

    RETURN(did);
}

void
InMemoryDatabase::finish_add_doc(Xapian::docid did, const Xapian::Document &document)
{
    {
	map<Xapian::valueno, string> values;
	Xapian::ValueIterator k = document.values_begin();
	for ( ; k != document.values_end(); ++k) {
	    values.insert(make_pair(k.get_valueno(), *k));
	    LOGLINE(DB, "InMemoryDatabase::finish_add_doc(): adding value " <<
			k.get_valueno() << " -> " << *k);
	}
	add_values(did, values);
    }

    InMemoryDoc doc(true);
    Xapian::TermIterator i = document.termlist_begin();
    Xapian::TermIterator i_end = document.termlist_end();
    for ( ; i != i_end; ++i) {
	make_term(*i);

	LOGLINE(DB, "InMemoryDatabase::finish_add_doc(): adding term " << *i);
	Xapian::PositionIterator j = i.positionlist_begin();
	Xapian::PositionIterator j_end = i.positionlist_end();

	if (j == j_end) {
	    /* Make sure the posting exists, even without a position. */
	    make_posting(&doc, *i, did, 0, i.get_wdf(), false);
	} else {
	    positions_present = true;
	    for ( ; j != j_end; ++j) {
		make_posting(&doc, *i, did, *j, i.get_wdf());
	    }
	}

	Assert(did > 0 && did <= doclengths.size());
	doclengths[did - 1] += i.get_wdf();
	totlen += i.get_wdf();
	postlists[*i].collection_freq += i.get_wdf();
	++postlists[*i].term_freq;
    }
    swap(termlists[did - 1], doc);

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
    termlists.push_back(InMemoryDoc(true));
    doclengths.push_back(0);
    doclists.push_back(docdata);

    AssertEqParanoid(termlists.size(), doclengths.size());

    return termlists.size();
}

void InMemoryDatabase::make_posting(InMemoryDoc * doc,
				    const string & tname,
				    Xapian::docid did,
				    Xapian::termpos position,
				    Xapian::termcount wdf,
				    bool use_position)
{
    Assert(doc);
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
    doc->add_posting(termentry);
}

bool
InMemoryDatabase::term_exists(const string & tname) const
{
    if (closed) InMemoryDatabase::throw_database_closed();
    Assert(!tname.empty());
    map<string, InMemoryTerm>::const_iterator i = postlists.find(tname);
    if (i == postlists.end()) return false;
    return (i->second.term_freq != 0);
}

bool
InMemoryDatabase::has_positions() const
{
    if (closed) InMemoryDatabase::throw_database_closed();
    return positions_present;
}

TermList *
InMemoryDatabase::open_allterms(const string & prefix) const
{
    if (closed) InMemoryDatabase::throw_database_closed();
    return new InMemoryAllTermsList(&postlists,
				    Xapian::Internal::RefCntPtr<const InMemoryDatabase>(this),
				    prefix);
}

void
InMemoryDatabase::throw_database_closed()
{
    throw Xapian::DatabaseError("Database has been closed");
}
