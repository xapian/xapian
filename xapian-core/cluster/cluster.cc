/** @file cluster.cc
 *  @brief Cluster API
 */
/* Copyright (C) 2010 Richard Boulton
 * Copyright (C) 2016 Richhiey Thomas
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

#include "xapian/cluster.h"

#include <xapian/error.h>
#include <api/termlist.h>
#include <debuglog.h>

#include <vector>
#include <map>

using namespace Xapian;
using namespace std;

MSetDocumentSource::MSetDocumentSource(const MSet &mset_)
    :mset(mset_), maxitems(mset_.size()), index(0)
{
    LOGCALL_CTOR(API,"MSetDocumentSource", mset_);
    mset.fetch();
}

MSetDocumentSource::MSetDocumentSource(const MSet & mset_, doccount maxitems_)
    :mset(mset_), maxitems(maxitems_), index(0)
{
    LOGCALL_CTOR(API, "MSetDocumentSource", mset | maxitems);
    if (maxitems > mset.size())
	maxitems = mset.size();

    if (maxitems > 0)
	mset.fetch(mset.begin(), mset[maxitems - 1]);
}

Document
MSetDocumentSource::next_document() {
    LOGCALL(API, Document, "MSetDocumentSource::next_document()", "");
    return (mset[index++].get_document());
}

bool
MSetDocumentSource::at_end() const {
    LOGCALL(API, bool, "MSetDocumentSource::at_end()", "");
    return (index >= maxitems);
}

doccount
MSetDocumentSource::size() {
    LOGCALL(API, doccount, "MSetDocumentSource::size()", "");
    return mset.size();
}

Similarity::~Similarity()
{
    LOGCALL_DTOR(API, "Similarity");
}

FreqSource::~FreqSource()
{
    LOGCALL_DTOR(API, "FreqSource");
}

doccount
DummyFreqSource::get_termfreq(const string &) {
    LOGCALL(API, doccount, "DummyFreqSource::get_termfreq()", "");
    return 1;
}

doccount
DummyFreqSource::get_doccount() {
    LOGCALL(API, doccount, "DummyFreqSource::get_doccount()", "");
    return 1;
}

void
TermListGroup::add_document(const Document &document) {
    LOGCALL_VOID(API, "TermListGroup::add_document()", document);

    TermIterator titer(document.termlist_begin());
    TermIterator end(document.termlist_end());

    for (; titer != end; titer++) {
	map<string, doccount>::iterator i;
	i = termfreq.find(*titer);
	if (i == termfreq.end())
	    termfreq[*titer] = 1;
	else
	    i->second += 1;
    }
}

void
TermListGroup::add_documents(DocumentSource &docs) {
    LOGCALL_VOID(API, "TermListGroup::add_documents()", docs);
    while (!docs.at_end()) {
	Document document = docs.next_document();
	add_document(document);
    }
    docs_num = docs.size();
}

doccount
TermListGroup::get_doccount() {
    LOGCALL(API, doccount, "TermListGroup::get_doccount()", "");
    return docs_num;
}

doccount
TermListGroup::get_termfreq(const string &tname) {
    LOGCALL(API, doccount, "TermListGroup::get_termfreq()", tname);
    return termfreq[tname];
}

doccount
ClusterSet::num_of_clusters() {
    LOGCALL(API, doccount, "ClusterSet::num_of_clusters()", "");
    return clusters.size();
}

doccount
ClusterSet::cluster_size(clusterid cid) {
    LOGCALL(API, doccount, "ClusterSet::cluster_size()", cid);
    map<clusterid, vector<Document> >::iterator i;
    i = clusters.find(cid);
    if (i == clusters.end())
	throw RangeError("The mentioned clusterid was out of range", 103);
    return ((i->second).size());
}

DocumentSet
ClusterSet::get_cluster(clusterid cid) {
    LOGCALL(API, DocumentSet, "ClusterSet::get_cluster()", cid);
    map<clusterid, vector<Document> >::iterator i;
    i = clusters.find(cid);
    if (i == clusters.end())
	throw RangeError("The mentioned clusterid was out of range", 103);
    vector<Document> docs;
    docs = i->second;
    DocumentSet docset;
    for (vector<Document>::iterator k = docs.begin(); k != docs.end(); k++)
	docset.add_document(*k);
    return docset;
}

void
ClusterSet::add_document(clusterid cid, Document &doc) {
    LOGCALL_VOID(API, "ClusterSet::add_document()", cid | doc);
    clusters[cid].push_back(doc);
}

DocumentSet
ClusterSetIterator::get_cluster() {
    LOGCALL(API, DocumentSet, "ClusterSetIterator::get_cluster()", "");
    return cset.get_cluster(index);
}

ClusterSetIterator
ClusterSet::begin() {
    LOGCALL(API, ClusterSetIterator, "ClusterSet::begin()", "");
    return ClusterSetIterator(*this, 0);
}

ClusterSetIterator
ClusterSet::end() {
    LOGCALL(API, ClusterSetIterator, "ClusterSet::end()", "");
    return ClusterSetIterator(*this, num_of_clusters());
}

ClusterSetIterator
ClusterSet::operator[](doccount i) {
    return ClusterSetIterator(*this, i);
}

void
DocumentSet::add_document(Document &doc) {
    LOGCALL_VOID(API, "DocumentSet::add_document()", doc);
    docs.push_back(doc);
}

Document
DocumentSet::operator[](doccount i) {
    Document d = docs[i];
    return d;
}

DocumentSetIterator
DocumentSet::begin() {
    LOGCALL(API, DocumentSetIterator, "DocumentSet::begin()", "");
    return DocumentSetIterator(*this, 0);
}

DocumentSetIterator
DocumentSet::end() {
    LOGCALL(API, DocumentSetIterator, "DocumentSet::end()", "");
    return DocumentSetIterator(*this, size());
}

Document
DocumentSetIterator::get_document() {
    LOGCALL(API, Document, "DocumentSetIterator::get_document()", "");
    Document d = docs[index];
    return d;
}
