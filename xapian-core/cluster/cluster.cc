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

#include "cluster/clusterinternal.h"
#include "xapian/error.h"
#include "api/termlist.h"
#include "debuglog.h"
#include "omassert.h"

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

TermListGroup::TermListGroup(const MSet &docs)
{
    LOGCALL_CTOR(API, "TermListGroup", docs);
    for (MSetIterator it = docs.begin(); it != docs.end(); ++it)
	add_document(it.get_document());
    docs_num = docs.size();
}

doccount
DummyFreqSource::get_termfreq(const string &) const
{
    LOGCALL(API, doccount, "DummyFreqSource::get_termfreq", NO_ARGS);
    return 1;
}

doccount
DummyFreqSource::get_doccount() const
{
    LOGCALL(API, doccount, "DummyFreqSource::get_doccount", NO_ARGS);
    return 1;
}

void
TermListGroup::add_document(const Document &document)
{
    LOGCALL_VOID(API, "TermListGroup::add_document", document);

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

doccount
TermListGroup::get_doccount() const
{
    LOGCALL(API, doccount, "TermListGroup::get_doccount", NO_ARGS);
    return docs_num;
}

doccount
TermListGroup::get_termfreq(const string &tname) const
{
    LOGCALL(API, doccount, "TermListGroup::get_termfreq", tname);
    unordered_map<string, doccount>::const_iterator it = termfreq.find(tname);
    if (it != termfreq.end())
	return it->second;
    else
	return 0;
}

DocumentSet::DocumentSet(const DocumentSet &other)
    : internal(other.internal)
{
}

void
DocumentSet::operator=(const DocumentSet &other)
{
    internal = other.internal;
}

DocumentSet::DocumentSet()
    : internal(new Xapian::DocumentSet::Internal)
{
}

unsigned int
DocumentSet::size() const
{
    LOGCALL(API, int, "DocumentSet::size", NO_ARGS);
    return internal->size();
}

unsigned int
DocumentSet::Internal::size() const
{
    return docs.size();
}

void
DocumentSet::add_document(const Document &doc)
{
    LOGCALL_VOID(API, "DocumentSet::add_document", doc);
    internal->add_document(doc);
}
void
DocumentSet::Internal::add_document(const Document &doc)
{
    docs.push_back(doc);
}

Document
DocumentSet::operator[](doccount i)
{
    return internal->get_document(i);
}

Document
DocumentSet::Internal::get_document(doccount i)
{
    return docs[i];
}

DocumentSet::~DocumentSet()
{
}

class PointTermIterator : public TermIterator::Internal {
    unordered_map<string, double>::const_iterator i;
    unordered_map<string, double>::const_iterator end;
    termcount size;
    bool started;
  public:
    PointTermIterator(const unordered_map<string, double> &termlist) :
    i(termlist.begin()), end(termlist.end()), size(termlist.size()), started(false)
    {}
    termcount get_approx_size() const { return size; }
    termcount get_wdf() const { throw UnimplementedError("PointIterator doesn't support get_wdf()"); }
    string get_termname() const { return i->first; }
    doccount get_termfreq() const { throw UnimplementedError("PointIterator doesn't support get_termfreq()"); }
    Internal * next();
    termcount positionlist_count() const {
	 throw UnimplementedError("PointTermIterator doesn't support positionlist_count()");
    }
    bool at_end() const;
    PositionIterator positionlist_begin() const {
	throw UnimplementedError("PointTermIterator doesn't support positionlist_begin()");
    }
    Internal * skip_to(const string &term);
};

TermIterator::Internal *
PointTermIterator::next()
{
    if (!started) {
	started = true;
	return NULL;
    }
    Assert(i != end);
    ++i;
    return NULL;
}

bool
PointTermIterator::at_end() const
{
    if (!started) return false;
    return i == end;
}

TermIterator::Internal *
PointTermIterator::skip_to(const string &term)
{
    if (i->first == term)
	return NULL;
    while (i->first != term)
	i++;
    return NULL;
}

TermIterator
PointType::termlist_begin() const
{
    LOGCALL(API, TermIterator, "PointType::termlist_begin", NO_ARGS);
    return TermIterator(new PointTermIterator(values));
}

bool
PointType::contains(const string &term) const
{
    LOGCALL(API, bool, "PointType::contains", term);
    return values.find(term) != values.end();
}

double
PointType::get_value(const string &term) const
{
    LOGCALL(API, double, "Point::get_value", term);
    unordered_map<string, double>::const_iterator it = values.find(term);
    return (it == values.end()) ? 0.0 : it->second;
}

double
PointType::get_magnitude() const {
    LOGCALL(API, double, "PointType::get_magnitude", NO_ARGS);
    return magnitude;
}

void
PointType::add_value(const string &term, double value)
{
    LOGCALL_VOID(API, "PointType::add_value", term | value);
    unordered_map<string, double>::iterator it;
    it = values.find(term);
    if (it != values.end())
	it->second += value;
    else
	values[term] = value;
}

void
PointType::set_value(const string &term, double value)
{
    LOGCALL_VOID(API, "PointType::set_value", term | value);
    values[term] = value;
}

int
PointType::termlist_size() const
{
    LOGCALL(API, int, "PointType::termlist_size", NO_ARGS);
    return values.size();
}

Document
Point::get_document() const
{
    LOGCALL(API, Document, "Point::get_document", NO_ARGS);
    return doc;
}

void
Point::initialize(const TermListGroup &tlg, const Document &doc_)
{
    LOGCALL_VOID(API, "Point::initialize", tlg | doc_);
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
	double tf = 1 + log((double)wdf);
	double idf = log(size / termfreq);
	double wt = tf * idf;
	values[term] = wt;
	magnitude += wt * wt;
    }
}

Centroid::Centroid(Point &p) {
    LOGCALL_CTOR(API, "Centroid", p);
    for (TermIterator it = p.termlist_begin(); it != p.termlist_end(); ++it)
	values[*it] = p.get_value(*it);
    magnitude = p.get_magnitude();
}

void
Centroid::divide(double num)
{
    LOGCALL_VOID(API, "Centroid::divide", num);
    unordered_map<string, double>::iterator it;
    for (it = values.begin(); it != values.end(); ++it)
	it->second /= num;
}

void
Centroid::clear()
{
    LOGCALL_VOID(API, "Centroid::clear", NO_ARGS);
    values.clear();
}

void
Centroid::recalc_magnitude()
{
    LOGCALL_VOID(API, "Centroid::recalc_magnitude", NO_ARGS);
    magnitude = 0;
    unordered_map<string, double>::iterator it;
    for (it = values.begin(); it != values.end(); ++it)
	magnitude += it->second * it->second;
}

void
Cluster::operator=(const Cluster &other)
{
    // pointers are reference counted.
    internal = other.internal;
}

Cluster::Cluster(const Cluster &other)
	: internal(other.internal)
{
}

Cluster::Cluster() : internal(new Xapian::Cluster::Internal)
{
    LOGCALL_CTOR(API, "Cluster", NO_ARGS);
}

Cluster::Cluster(const Centroid centroid)
    : internal(new Xapian::Cluster::Internal(centroid))
{
    LOGCALL_CTOR(API, "Cluster", centroid);
}

Cluster::~Cluster()
{
    LOGCALL_DTOR(API, "Cluster");
}

Centroid::Centroid()
{
    LOGCALL_CTOR(API, "Centroid", NO_ARGS);
}

DocumentSet
Cluster::get_documents()
{
    LOGCALL(API, DocumentSet, "Cluster::get_documents", NO_ARGS);
    return internal->get_documents();
}

DocumentSet
Cluster::Internal::get_documents()
{
    DocumentSet docs;
    for (auto&& point : cluster_docs)
	docs.add_document(point.get_document());
    return docs;
}

Point
Cluster::get_index(unsigned int index) const
{
    LOGCALL(API, Point, "Cluster::get_index", index);
    return internal->get_index(index);
}

Point
Cluster::Internal::get_index(unsigned int index) const
{
    return cluster_docs[index];
}

void
ClusterSet::operator=(const ClusterSet &other)
{
    // pointers are reference counted.
    internal = other.internal;
}

ClusterSet::ClusterSet(const ClusterSet &other)
	: internal(other.internal)
{
}

ClusterSet::ClusterSet() : internal(new Xapian::ClusterSet::Internal)
{
}

ClusterSet::~ClusterSet()
{
}

doccount
ClusterSet::Internal::size() const
{
    return clusters.size();
}

doccount
ClusterSet::size() const
{
    LOGCALL(API, doccount, "ClusterSet::size", NO_ARGS);
    return internal->size();
}

void
ClusterSet::Internal::add_cluster(const Cluster &c)
{
    clusters.push_back(c);
}

void
ClusterSet::add_cluster(const Cluster &c)
{
    LOGCALL_VOID(API, "ClusterSet::add_cluster", c);
    internal->add_cluster(c);
}

Cluster&
ClusterSet::Internal::get_cluster(doccount i)
{
    return clusters[i];
}

Cluster&
ClusterSet::operator[](doccount i)
{
    return internal->get_cluster(i);
}

void
ClusterSet::Internal::add_to_cluster(const Point &x, unsigned int i)
{
    clusters[i].add_point(x);
}

void
ClusterSet::add_to_cluster(const Point &x, unsigned int i)
{
   LOGCALL_VOID(API, "ClusterSet::add_to_cluster", x | i);
   internal->add_to_cluster(x, i);
}

void
ClusterSet::Internal::recalculate_centroids()
{
    for (vector<Cluster>::iterator it = clusters.begin(); it != clusters.end(); ++it)
	(*it).recalculate();
}

void
ClusterSet::recalculate_centroids()
{
    LOGCALL_VOID(API, "ClusterSet::recalculate_centroids", NO_ARGS);
    internal->recalculate_centroids();
}

void
ClusterSet::clear_clusters()
{
    LOGCALL_VOID(API, "ClusterSet::clear_clusters", NO_ARGS);
    internal->clear_clusters();
}

void
ClusterSet::Internal::clear_clusters()
{
    for (vector<Cluster>::iterator it = clusters.begin(); it !=clusters.end(); ++it)
	(*it).clear();
}

doccount
Cluster::size() const
{
    LOGCALL(API, doccount, "Cluster::size", NO_ARGS);
    return internal->size();
}

doccount
Cluster::Internal::size() const
{
    return (cluster_docs.size());
}

void
Cluster::add_point(const Point &doc)
{
    LOGCALL_VOID(API, "Cluster::add_point", doc);
    internal->add_point(doc);
}

void
Cluster::Internal::add_point(const Point &doc)
{
    cluster_docs.push_back(doc);
}

void
Cluster::clear()
{
    LOGCALL_VOID(API, "Cluster::clear", NO_ARGS);
    internal->clear();
}

void
Cluster::Internal::clear()
{
    cluster_docs.clear();
}

Centroid
Cluster::get_centroid() const
{
    LOGCALL(API, Centroid, "Cluster::get_centroid", NO_ARGS);
    return internal->get_centroid();
}

Centroid
Cluster::Internal::get_centroid() const
{
    return centroid;
}

void
Cluster::set_centroid(const Centroid centroid_)
{
    LOGCALL_VOID(API, "Cluster::set_centroid", centroid_);
    internal->set_centroid(centroid_);
}

void
Cluster::Internal::set_centroid(const Centroid centroid_)
{
    centroid = centroid_;
}

void
Cluster::recalculate()
{
    LOGCALL_VOID(API, "Cluster::recalculate", NO_ARGS);
    internal->recalculate();
}

void
Cluster::Internal::recalculate()
{
    centroid.clear();
    for (vector<Point>::iterator it = cluster_docs.begin(); it != cluster_docs.end(); ++it) {
	Point &temp = *it;
	for (TermIterator titer = temp.termlist_begin(); titer != temp.termlist_end(); ++titer)
	    centroid.add_value(*titer, temp.get_value(*titer));
    }
    centroid.divide(size());
    centroid.recalc_magnitude();
}
