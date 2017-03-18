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

class PointTermIterator : public TermIterator::Internal {
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
    return !(values.find(term) == values.end());
}

double
PointType::get_value(string term) {
    LOGCALL(API, double, "Point::get_value()", term);
    return (values.find(term) == values.end()) ? 0.0 : values[term];
}

double
PointType::get_magnitude() const {
    LOGCALL(API, double, "PointType::get_magnitude()", NO_ARGS);
    return magnitude;
}

void
PointType::add_value(string term, double value) {
    LOGCALL_VOID(API, "PointType::add_value()", term | value);
    unordered_map<string, double>::iterator it;
    it = values.find(term);
    if (it != values.end())
	it->second += value;
    else {
	termlist.push_back(Wdf(term, 1));
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
    LOGCALL(API, int, "PointType::termlist_size()", NO_ARGS);
    return termlist.size();
}

Document
Point::get_document() const {
    LOGCALL(API, Document, "Point::get_document()", NO_ARGS);
    return doc;
}

void
Point::initialize(TermListGroup &tlg, const Document &doc_) {
    LOGCALL_VOID(API, "Point::initialize()", tlg | doc);
    doccount size = tlg.get_doccount();
    doc = doc_;
    for (TermIterator it = doc.termlist_begin(); it != doc.termlist_end(); ++it) {
	doccount wdf = it.get_wdf();
	string term = *it;
	double termfreq = tlg.get_termfreq(term);
	/** If term is not a stemmed term or the term indexes only one
	 *  one document, do not compute TF-IDF scores for them
	 */
	if (wdf < 1 || term[0] != 'Z' || termfreq <= 1)
	    continue;
	Wdf term_wdf(term, wdf);
	termlist.push_back(term_wdf);
	double tf = 1 + log(wdf);
	double idf = log(size / termfreq);
	double wt = tf*idf;
	values[term] = wt;
	magnitude += wt*wt;
    }
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
Cluster::get_index(unsigned int index) const {
    LOGCALL(API, Point, "Cluster::get_index()", index);
    return cluster_docs[index];
}

doccount
ClusterSet::size() const {
    LOGCALL(API, doccount, "ClusterSet::size()", NO_ARGS);
    return clusters.size();
}

Cluster
ClusterSet::get_cluster(unsigned int index) const {
    LOGCALL(API, Cluster, "ClusterSet::get_cluster()", index);
    if (index >= clusters.size())
	throw RangeError("The mentioned cluster index was out of range");
    return clusters[index];
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

void
ClusterSet::add_to_cluster(const Point &x, unsigned int i) {
   LOGCALL_VOID(API, "ClusterSet::add_to_cluster()", x | i);
   clusters[i].add_point(x);
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
