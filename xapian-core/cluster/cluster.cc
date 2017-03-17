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
#include <omassert.h>

#include <cmath>
#include <string>
#include <unordered_map>
#include <vector>

using namespace Xapian;
using namespace std;

FreqSource::~FreqSource()
{
    LOGCALL_DTOR(API, "FreqSource");
}

Similarity::~Similarity()
{
    LOGCALL_DTOR(API, "Similarity");
}

Clusterer::~Clusterer()
{
    LOGCALL_DTOR(API, "Clusterer");
}

doccount
DummyFreqSource::get_termfreq(const string &) {
    LOGCALL(API, doccount, "DummyFreqSource::get_termfreq()", NO_ARGS);
    return 1;
}

doccount
DummyFreqSource::get_doccount() const {
    LOGCALL(API, doccount, "DummyFreqSource::get_doccount()", NO_ARGS);
    return 1;
}

void
TermListGroup::add_document(const Document &document) {
    LOGCALL_VOID(API, "TermListGroup::add_document()", document);

    TermIterator titer(document.termlist_begin());
    TermIterator end(document.termlist_end());

    for (; titer != end; ++titer) {
	unordered_map<string, doccount>::iterator i;
	i = termfreq.find(*titer);
	if (i == termfreq.end())
	    termfreq[*titer] = 1;
	else
	    i->second += 1;
    }
}

void
TermListGroup::add_documents(const MSet &docs) {
    LOGCALL_VOID(API, "TermListGroup::add_documents()", docs);
    for (MSetIterator it = docs.begin(); it != docs.end(); ++it)
	add_document(it.get_document());
    docs_num = docs.size();
}

doccount
TermListGroup::get_doccount() const {
    LOGCALL(API, doccount, "TermListGroup::get_doccount()", NO_ARGS);
    return docs_num;
}

doccount
TermListGroup::get_termfreq(const string &tname) {
    LOGCALL(API, doccount, "TermListGroup::get_termfreq()", tname);
    return termfreq[tname];
}

doccount
ClusterSet::size() const {
    LOGCALL(API, doccount, "ClusterSet::size()", NO_ARGS);
    return clusters.size();
}

doccount
ClusterSet::cluster_size(clusterid cid) const {
    LOGCALL(API, doccount, "ClusterSet::cluster_size()", cid);
    unsigned int s = clusters.size();
    if (cid >= s)
	throw RangeError("The mentioned clusterid was out of range");
    return clusters[cid].size();
}

Cluster
ClusterSet::get_cluster(clusterid cid) const {
    LOGCALL(API, Cluster, "ClusterSet::get_cluster()", cid);
    unsigned int s = clusters.size();
    if (cid >= s)
	throw RangeError("The mentioned clusterid was out of range");
    return clusters[cid];
}

void
ClusterSet::add_cluster(Cluster &c) {
    LOGCALL_VOID(API, "ClusterSet::add_cluster()", c);
    clusters.push_back(c);
}

Cluster
ClusterSet::operator[](doccount i) {
    return clusters[i];
}

int
DocumentSet::size() const {
    LOGCALL(API, int, "DocumentSet::size()", NO_ARGS);
    return docs.size();
}

void
DocumentSet::add_document(Document doc) {
    LOGCALL_VOID(API, "DocumentSet::add_document()", doc);
    docs.push_back(doc);
}

Document
DocumentSet::operator[](doccount i) {
    return docs[i];
}

DocumentSetIterator
DocumentSet::begin() const {
    LOGCALL(API, DocumentSetIterator, "DocumentSet::begin()", NO_ARGS);
    return DocumentSetIterator(*this, 0);
}

DocumentSetIterator
DocumentSet::end() const {
    LOGCALL(API, DocumentSetIterator, "DocumentSet::end()", NO_ARGS);
    return DocumentSetIterator(*this, size());
}

Document
DocumentSetIterator::get_document() {
    LOGCALL(API, Document, "DocumentSetIterator::get_document()", NO_ARGS);
    return docs[index];
}

void
Centroid::set_to_point(Point &p) {
    LOGCALL_VOID(API, "Centroid::set_to_point()", p);
    magnitude=0;
    for (TermIterator it = p.termlist_begin(); it != p.termlist_end(); ++it) {
	termlist.push_back(Wdf(*it,1));
	values[*it] = p.get_value(*it);
	magnitude += p.get_value(*it)*p.get_value(*it);
    }
}

void
Centroid::divide(int num) {
    LOGCALL_VOID(API, "Centroid::divide()", num);
    unordered_map<string, double>::iterator it;
    for (it = values.begin(); it != values.end(); ++it)
	it->second = it->second / num;
}

void
Centroid::clear() {
    LOGCALL_VOID(API, "Centroid::clear()", NO_ARGS);
    values.clear();
    termlist.clear();
}

void
Centroid::recalc_magnitude() {
    LOGCALL_VOID(API, "Centroid::recalc_magnitude()", NO_ARGS);
    magnitude = 0;
    unordered_map<string, double>::iterator it;
    for (it = values.begin(); it != values.end(); ++it)
	magnitude += it->second*it->second;
}

class XAPIAN_VISIBILITY_DEFAULT PointTermIterator : public TermIterator::Internal {
    std::vector<Wdf>::const_iterator i;
    std::vector<Wdf>::const_iterator end;
    termcount size;
    bool started;
  public:
    PointTermIterator(const std::vector<Wdf> &termlist) : 
    i(termlist.begin()), end(termlist.end()), size(termlist.size()), started(false)
    {}
    termcount get_approx_size() const { return size; }
    termcount get_wdf() const { return i->wdf; } 
    std::string get_termname() const { return i->term; }
    doccount get_termfreq() const { throw UnimplementedError("PointIterator doesn't support get_termfreq()"); }
    Internal * next();
    termcount positionlist_count() const {
	 throw UnimplementedError("PointTermIterator doesn't support positionlist_count()");
    }
    bool at_end() const;
    PositionIterator positionlist_begin() const {
	throw UnimplementedError("PointTermIterator doesn't support positionlist_begin()");
    }
    Internal * skip_to(const std::string &term);
};

Document
Point::get_document() const {
    LOGCALL(API, Document, "Point::get_document()", NO_ARGS);
    return doc;
}

TermIterator
PointType::termlist_begin() const {
    LOGCALL(API, TermIterator, "PointType::termlist_begin()", NO_ARGS);
    return TermIterator(new PointTermIterator(termlist));
}

TermIterator
PointType::termlist_end() const {
    LOGCALL(API, TermIterator, "PointType::termlist_end()", NO_ARGS);
    return TermIterator(NULL);
}

bool
PointType::contains(string term) {
    LOGCALL(API, bool, "PointType::contains()", term);
    unordered_map<string, double>::iterator it;
    it = values.find(term);
    if (it == values.end())
	return false;
    else
	return true;
}

double
PointType::get_value(string term) {
    LOGCALL(API, double, "Point::get_value()", term);
    unordered_map<string, double>::iterator it;
    it = values.find(term);
    if (it == values.end())
	return 0.0;
    else
	return values[term];
}

void
Point::initialize(TermListGroup &tlg, const Document &doc_) {
    LOGCALL_VOID(API, "Point::initialize()", tlg | doc);
    doccount size = tlg.get_doccount();
    doc = doc_;
    for (TermIterator it = doc.termlist_begin(); it != doc.termlist_end(); ++it) {
	doccount wdf = it.get_wdf();
	if (wdf < 1)
	    wdf = 1;
	Wdf term_wdf(*it, wdf);
	termlist.push_back(term_wdf);
	double tf, wt;
	tf = 1 + log(wdf);
	double idf;
	double termfreq = tlg.get_termfreq(*it);
	idf = log(size/termfreq);
	wt = tf*idf;
	values[*it] = wt;
	magnitude += wt*wt;
    }
}

double
PointType::get_magnitude() const {
    LOGCALL(API, double, "PointType::get_magnitude()", NO_ARGS);
    return magnitude;
}

void
PointType::add_value (string term, double value) {
    LOGCALL_VOID(API, "PointType::add_value()", term | value);
    unordered_map<string, double>::iterator it;
    it = values.find(term);
    if (it != values.end())
	it->second += value;
    else {
	termlist.push_back(Wdf(term,1));
	values[term] = value;
    }
}

void
PointType::set_value(string term, double value) {
    LOGCALL_VOID(API, "PointType::set_value()", term | value);
    values[term] = value;
}

int
PointType::termlist_size() const {
    return termlist.size();
}

TermIterator::Internal *
PointTermIterator::next() {
    if (!started) {
	started = true;
	return NULL;
    }
    Assert(i != end);
    ++i; return NULL;
}

bool
PointTermIterator::at_end() const
{
    if (!started) return false;
    return i == end;
}

TermIterator::Internal *
PointTermIterator::skip_to(const string &term) {
    if (i->term == term)
	return NULL;
    while (i->term != term)
	i++;
    return NULL;
}

void
ClusterSet::add_to_cluster(const Point &x, clusterid i) {
   LOGCALL_VOID(API, "ClusterSet::add_to_cluster()", x | i);
   clusters[i].add_point(x);

}

void
ClusterSet::clear_clusters() {
    LOGCALL_VOID(API, "ClusterSet::clear_clusters()", NO_ARGS);
    for (vector<Cluster>::iterator it = clusters.begin(); it !=clusters.end(); ++it)
	(*it).clear();
}

Cluster::Cluster() {
    LOGCALL_CTOR(API, "Cluster()", NO_ARGS);
}

Cluster::~Cluster() {
    LOGCALL_DTOR(API, "Cluster()");
}

DocumentSet
Cluster::get_documents() {
    LOGCALL(API, DocumentSet, "Cluster::get_documents()", NO_ARGS);
    DocumentSet docs;
    int s = size();
    for (int i = 0; i < s; ++i) {
	Point x = get_index(i);
	docs.add_document(x.get_document());
    }
    return docs;
}

Point
Cluster::get_index(int index) const {
    LOGCALL(API, Point, "Cluster::get_index()", index);
    return cluster_docs[index];
}

doccount
Cluster::size() const {
    LOGCALL(API, doccount, "Cluster::size()", NO_ARGS);
    return (cluster_docs.size());
}

void
Cluster::add_point(const Point &doc) {
    LOGCALL_VOID(API, "Cluster::add_point()", doc);
    cluster_docs.push_back(doc);
}


void
Cluster::clear() {
    LOGCALL_VOID(API, "Cluster::clear()", NO_ARGS);
    cluster_docs.clear();
}

Centroid
Cluster::get_centroid() const {
    LOGCALL(API, Centroid, "Cluster::get_centroid()", NO_ARGS);
    return centroid;
}

void
Cluster::set_centroid(const Centroid centroid_) {
    LOGCALL_VOID(API, "Cluster::set_centroid()", centroid_);
    centroid = centroid_;
}

void
ClusterSet::recalculate_centroids() {
    LOGCALL_VOID(API, "ClusterSet::recalculate_centroids()", NO_ARGS);
    for (vector<Cluster>::iterator it = clusters.begin(); it != clusters.end(); ++it)
	(*it).recalculate();
}

void
Cluster::recalculate() {
    LOGCALL_VOID(API, "Cluster::recalculate()", NO_ARGS);
    centroid.clear();
    for (vector<Point>::iterator it = cluster_docs.begin(); it != cluster_docs.end(); ++it) {
	Point &temp = *it;
	for (TermIterator titer = temp.termlist_begin(); titer != temp.termlist_end(); ++titer)
	    centroid.add_value(*titer, temp.get_value(*titer));
    }
    centroid.divide(size());
    centroid.recalc_magnitude();
}
