/* cluster.cc: document clustering interface.
 *
 * Copyright 2007 Lemur Consulting Ltd
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include <config.h>

#include "xapian/cluster.h"
#include "xapian/error.h"

#include "debuglog.h"
#include "omassert.h"
#include "termlist.h"

#include <algorithm>

using namespace Xapian;
using namespace std;

DocumentSource::~DocumentSource()
{
    LOGCALL_DTOR(API, "DocumentSource");
}

MSetDocumentSource::MSetDocumentSource(const MSet & mset_)
	: mset(mset_),
	  maxitems(mset_.size()),
	  index(0)
{
    LOGCALL_CTOR(API, "MSetDocumentSource", mset);

    // We're going to be accessing the contents of all the items in the mset,
    // so tell the mset to start fetching them.
    mset.fetch();
}

MSetDocumentSource::MSetDocumentSource(const MSet & mset_, doccount maxitems_)
	: mset(mset_),
	  maxitems(maxitems_),
	  index(0)
{
    LOGCALL_CTOR(API, "MSetDocumentSource", mset_ | maxitems_);
    if (maxitems > mset.size()) {
	maxitems = mset.size();
    }

    // We're going to be accessing the contents of the first "maxitems" items
    // in the mset, so tell the mset to start fetching them.
    if (maxitems > 0) {
	mset.fetch(mset.begin(), mset[maxitems - 1]);
    }
}

Document
MSetDocumentSource::next_document()
{
    LOGCALL(API, Document, "MSetDocumentSource::next_document", "");
    AssertRel(index, <, maxitems);
    RETURN(mset[index++].get_document());
}

bool
MSetDocumentSource::at_end() const
{
    LOGCALL(API, bool, "MSetDocumentSource::at_end", "");
    RETURN(index >= maxitems);
}

TermFreqSource::~TermFreqSource()
{
    LOGCALL_DTOR(API, "TermFreqSource");
}

doccount
DummyTermFreqSource::get_termfreq(const string &) const
{
    LOGCALL(API, doccount, "DummyTermFreqSource::get_termfreq", "");
    RETURN(1);
}

doccount
DummyTermFreqSource::get_doccount() const
{
    LOGCALL(API, doccount, "DummyTermFreqSource::get_doccount", "");
    RETURN(1);
}

doccount
DatabaseTermFreqSource::get_termfreq(const string &tname) const
{
    LOGCALL(API, doccount, "DatabaseTermFreqSource::get_termfreq", "");
    RETURN(db.get_termfreq(tname));
}

doccount
DatabaseTermFreqSource::get_doccount() const
{
    LOGCALL(API, doccount, "DatabaseTermFreqSource::get_doccount", "");
    RETURN(db.get_doccount());
}

void
TermListGroup::add_document(const Document & document,
			    const ExpandDecider * decider)
{
    LOGCALL_VOID(API, "TermListGroup::add_document", document | decider);
    vector<TermWdf> & tl = termlists[document.get_docid()];
    tl.clear();
    TermIterator titer(document.termlist_begin());
    TermIterator end(document.termlist_end());
    for (; titer != end; ++titer) {
	if (decider != NULL && !(*decider)(*titer))
	    continue;
	tl.push_back(TermWdf(*titer, titer.get_wdf()));
	map<string, doccount>::iterator i;
	i = termfreqs.find(*titer);
	if (i == termfreqs.end()) {
	    termfreqs[*titer] = 0;
	} else {
	    i->second += 1;
	}
    }
}

void
TermListGroup::add_document(const Document & document,
			    Xapian::valueno slot)
{
    LOGCALL_VOID(API, "TermListGroup::add_document", document | slot);
    vector<TermWdf> & tl = termlists[document.get_docid()];
    tl.clear();
    std::string value = document.get_value(slot);
    tl.push_back(TermWdf(value, 1));

    map<string, doccount>::iterator i;
    i = termfreqs.find(value);
    if (i == termfreqs.end()) {
	termfreqs[value] = 0;
    } else {
	i->second += 1;
    }
}

void
TermListGroup::add_documents(DocumentSource & source,
			     const ExpandDecider * decider)
{
    LOGCALL_VOID(API, "TermListGroup::add_documents", source | decider);
    while (!source.at_end()) {
	add_document(source.next_document(), decider);
    }
}

void
TermListGroup::add_documents(DocumentSource & source,
			     Xapian::valueno slot)
{
    LOGCALL_VOID(API, "TermListGroup::add_documents", source | slot);
    while (!source.at_end()) {
	add_document(source.next_document(), slot);
    }
}


doccount
TermListGroup::get_termfreq(const string &tname) const
{
    LOGCALL(API, doccount, "TermListGroup::get_termfreq", tname);
    map<string, doccount>::const_iterator i;
    i = termfreqs.find(tname);
    if (i == termfreqs.end())
	RETURN(0);
    RETURN(i->second);
}

doccount
TermListGroup::get_doccount() const
{
    LOGCALL(API, doccount, "TermListGroup::get_doccount", "");
    RETURN(termlists.size());
}

class VectorTermWdfIterator : public TermIterator::Internal
{
    vector<TermWdf>::const_iterator i;
    vector<TermWdf>::const_iterator end;
    termcount size;
    bool started;
  public:
    VectorTermWdfIterator(const vector<TermWdf> & termlist)
	: i(termlist.begin()), end(termlist.end()), size(termlist.size()),
	  started(false)
    {}

    termcount get_approx_size() const { return size; }
    string get_termname() const { return i->tname; }
    termcount get_wdf() const { return i->wdf; }
    doccount get_termfreq() const {
	throw UnimplementedError("VectorTermWdfIterator doesn't support get_termfreq()");
    }
    Internal * next();
    bool at_end() const;
    termcount positionlist_count() const;
    PositionIterator positionlist_begin() const;
};

TermIterator::Internal *
VectorTermWdfIterator::next() {
    if (!started) {
	started = true;
	return NULL;
    }
    Assert(i != end);
    ++i; return NULL;
}

bool
VectorTermWdfIterator::at_end() const
{
    if (!started) return false;
    return i == end;
}

termcount
VectorTermWdfIterator::positionlist_count() const
{
    throw UnimplementedError("VectorTermWdfIterator doesn't support positionlist_count()");
}

PositionIterator
VectorTermWdfIterator::positionlist_begin() const
{
    throw UnimplementedError("VectorTermWdfIterator doesn't support positionlist_begin()");
}

TermIterator
TermListGroup::termlist_begin(docid did) const
{
    LOGCALL(API, TermIterator, "TermListGroup::termlist_begin", did);
    map<docid, vector<TermWdf> >::const_iterator i = termlists.find(did);
    if (i == termlists.end()) {
	throw DocNotFoundError("Document not found in TermListGroup");
    }
    return TermIterator(new VectorTermWdfIterator(i->second));
}

struct ClusterMerge {
    int i;
    int j;
    double dist;
    ClusterMerge(int i_, int j_, double dist_) : i(i_), j(j_), dist(dist_) {}
};

static bool operator<(const ClusterMerge & a, const ClusterMerge & b)
{
    return a.dist < b.dist;
}

static double get_distance(const vector<double> & distance, int docs,
			   int i, int j)
{
    if (i == j) return 0.0;
    if (i < j) {
	int tmp = i;
	i = j;
	j = tmp;
    }
    return distance[i * docs + j];
}

static void set_distance(vector<double> & distance, int docs, int i, int j,
			 double dist)
{
    if (i < j) {
	int tmp = i;
	i = j;
	j = tmp;
    }
    distance[i * docs + j] = dist;
}

void
ClusterSingleLink::cluster(ClusterAssignments & clusters,
			   DocSim & docsim,
			   DocumentSource & docsource,
			   const ExpandDecider * decider,
			   int num_clusters)
{
    TermListGroup tlg;
    vector<docid> docids;
    while (!docsource.at_end())
    {
	Document doc = docsource.next_document();
	tlg.add_document(doc, decider);
	docids.push_back(doc.get_docid());
    }

    do_cluster(clusters, docsim, num_clusters, tlg, docids);
}

void
ClusterSingleLink::cluster(ClusterAssignments & clusters,
			   DocSim & docsim,
			   DocumentSource & docsource,
			   Xapian::valueno slot,
			   int num_clusters)
{
    TermListGroup tlg;
    vector<docid> docids;
    while (!docsource.at_end())
    {
	Document doc = docsource.next_document();
	tlg.add_document(doc, slot);
	docids.push_back(doc.get_docid());
    }

    do_cluster(clusters, docsim, num_clusters, tlg, docids);
}

void
ClusterSingleLink::do_cluster(ClusterAssignments & clusters,
			      DocSim & docsim,
			      int num_clusters,
			      const TermListGroup & tlg,
			      const vector<docid> & docids)
{
    int docs = docids.size();
    docsim.set_termfreqsource(&tlg);

    vector<double> distance;
    distance.resize(docs * docs, 0.0);
    for (int i = 1; i < docs; ++i) {
	for (int j = 0; j < i; ++j) {
	    double dist;
	    docid docid_i = docids[i];
	    docid docid_j = docids[j];
	    dist = 1.0 - docsim.similarity(tlg.termlist_begin(docid_i),
					   tlg.termlist_end(docid_i),
					   tlg.termlist_begin(docid_j),
					   tlg.termlist_end(docid_j));
	    set_distance(distance, docs, i, j, dist);
	}
    }

    // Repeatedly find the shortest distance, and then merge the two items at
    // the minimum distance.
    int groups = docs;
    vector<ClusterMerge> merges;
    merges.reserve(docs);
    vector<bool> merged;
    merged.resize(docs, false);
    while (groups > 1)
    {
	int mindist_i = -1;
	int mindist_j = -1;
	double mindist = 2.0;
	for (int i = 1; i != docs; ++i) {
	    if (merged[i]) continue;
	    for (int j = 0; j < i; ++j) {
	    	if (merged[j]) continue;
		double dist = get_distance(distance, docs, i, j);
		AssertRel(dist, >=, 0.0);
		AssertRel(dist, <=, 1.0);
		if (dist < mindist) {
		    mindist_i = i;
		    mindist_j = j;
		    mindist = dist;
		}
	    }
	}

	// Merge mindist_i and mindist_j
	for (int k = 0; k != docs; ++k) {
	    double dist_i = get_distance(distance, docs, mindist_i, k);
	    double dist_j = get_distance(distance, docs, mindist_j, k);
	    // Take the minimum distance (for single-link clustering).
	    double dist = min(dist_i, dist_j);
	    //set_distance(distance, docs, mindist_i, k, dist);
	    set_distance(distance, docs, mindist_j, k, dist);
	}
	merged[mindist_i] = true;
	merges.push_back(ClusterMerge(mindist_i, mindist_j, mindist));
	groups -= 1;
    }

    // Sort the merges so that the smallest distances are first.
    sort(merges.begin(), merges.end());

    clusters.ids.clear();
    int cluster_count = 0;
    for (int i = 0; i != docs; ++i) {
	clusters.ids[docids[i]] = i;
	cluster_count += 1;
    }

    vector<ClusterMerge>::const_iterator merge = merges.begin();
    while (merge != merges.end() && cluster_count > num_clusters)
    {
	docid docid_i = docids[merge->i];
	docid docid_j = docids[merge->j];
	Assert(clusters.ids[docid_i] != clusters.ids[docid_j]);

	// Replace any documents with cluster ID old_id with ID new_id.
	int new_id = clusters.ids[docid_i];
	int old_id = clusters.ids[docid_j];

	std::map<Xapian::docid, int>::iterator citer;
	for (citer = clusters.ids.begin(); citer != clusters.ids.end(); ++citer)
	{
	    if (citer->second == old_id)
		citer->second = new_id;
	}

	++merge;
	--cluster_count;
    }
}
