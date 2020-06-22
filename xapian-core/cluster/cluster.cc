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

TermListGroup::TermListGroup(const MSet &docs, const Stopper *stopper)
{
    LOGCALL_CTOR(API, "TermListGroup", docs | stopper);
    for (MSetIterator it = docs.begin(); it != docs.end(); ++it)
	add_document(it.get_document(), stopper);
    num_of_documents = docs.size();
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
TermListGroup::add_document(const Document &document, const Stopper *stopper)
{
    LOGCALL_VOID(API, "TermListGroup::add_document", document | stopper);

    TermIterator titer(document.termlist_begin());

    for (; titer != document.termlist_end(); ++titer) {
	const string &term = *titer;

	// Remove stopwords by using the Xapian::Stopper object
	if (stopper && (*stopper)(term))
	    continue;

	// Remove unstemmed terms since document vector should
	// contain only stemmed terms
	if (term[0] != 'Z')
	    continue;

	unordered_map<string, doccount>::iterator i;
	i = termfreq.find(term);
	if (i == termfreq.end())
	    termfreq[term] = 1;
	else
	    ++i->second;
    }
}

doccount
TermListGroup::get_doccount() const
{
    LOGCALL(API, doccount, "TermListGroup::get_doccount", NO_ARGS);
    return num_of_documents;
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

DocumentSet::DocumentSet(const DocumentSet &) = default;

DocumentSet&
DocumentSet::operator=(const DocumentSet &) = default;

DocumentSet::DocumentSet(DocumentSet &&) = default;

DocumentSet&
DocumentSet::operator=(DocumentSet &&) = default;

DocumentSet::DocumentSet()
    : internal(new Xapian::DocumentSet::Internal)
{
}

doccount
DocumentSet::size() const
{
    LOGCALL(API, doccount, "DocumentSet::size", NO_ARGS);
    return internal->size();
}

doccount
DocumentSet::Internal::size() const
{
    return documents.size();
}

void
DocumentSet::add_document(const Document &document)
{
    LOGCALL_VOID(API, "DocumentSet::add_document", document);
    internal->add_document(document);
}
void
DocumentSet::Internal::add_document(const Document &document)
{
    documents.push_back(document);
}

Document&
DocumentSet::operator[](doccount i)
{
    return internal->get_document(i);
}

Document&
DocumentSet::Internal::get_document(doccount i)
{
    return documents[i];
}

const Document&
DocumentSet::operator[](doccount i) const
{
    return internal->get_document(i);
}

const Document&
DocumentSet::Internal::get_document(doccount i) const
{
    return documents[i];
}

DocumentSet::~DocumentSet()
{
    LOGCALL_DTOR(API, "DocumentSet");
}

class PointTermIterator : public TermIterator::Internal {
    unordered_map<string, double>::const_iterator i;
    unordered_map<string, double>::const_iterator end;
    termcount size;
    bool started;
  public:
    PointTermIterator(const unordered_map<string, double> &termlist)
	: i(termlist.begin()), end(termlist.end()),
	  size(termlist.size()), started(false)
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
    PositionList* positionlist_begin() const {
	throw UnimplementedError("PointTermIterator doesn't support positionlist_begin()");
    }
    Internal * skip_to(const string &) {
	throw UnimplementedError("PointTermIterator doesn't support skip_to()");
    }
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

TermIterator
PointType::termlist_begin() const
{
    LOGCALL(API, TermIterator, "PointType::termlist_begin", NO_ARGS);
    return TermIterator(new PointTermIterator(weights));
}

bool
PointType::contains(const string &term) const
{
    LOGCALL(API, bool, "PointType::contains", term);
    return weights.find(term) != weights.end();
}

double
PointType::get_weight(const string &term) const
{
    LOGCALL(API, double, "PointType::get_weight", term);
    unordered_map<string, double>::const_iterator it = weights.find(term);
    return (it == weights.end()) ? 0.0 : it->second;
}

double
PointType::get_magnitude() const {
    LOGCALL(API, double, "PointType::get_magnitude", NO_ARGS);
    return magnitude;
}

void
PointType::add_weight(const string &term, double weight)
{
    LOGCALL_VOID(API, "PointType::add_weight", term | weight);
    unordered_map<string, double>::iterator it;
    it = weights.find(term);
    if (it != weights.end())
	it->second += weight;
    else
	weights[term] = weight;
}

void
PointType::set_weight(const string &term, double weight)
{
    LOGCALL_VOID(API, "PointType::set_weight", term | weight);
    weights[term] = weight;
}

termcount
PointType::termlist_size() const
{
    LOGCALL(API, termcount, "PointType::termlist_size", NO_ARGS);
    return weights.size();
}

Document
Point::get_document() const
{
    LOGCALL(API, Document, "Point::get_document", NO_ARGS);
    return document;
}

Point::Point(const TermListGroup &tlg, const Document &document_)
{
    LOGCALL_CTOR(API, "Point::initialize", tlg | document_);
    doccount size = tlg.get_doccount();
    document = document_;
    for (TermIterator it = document.termlist_begin(); it != document.termlist_end(); ++it) {
	doccount wdf = it.get_wdf();
	string term = *it;
	double termfreq = tlg.get_termfreq(term);

	// If the term exists in only one document, or if it exists in
	// every document within the MSet, or if it is a filter term, then
	// these terms are not used for document vector calculations
	if (wdf < 1 || termfreq <= 1 || size == termfreq)
	    continue;

	double tf = 1 + log(double(wdf));
	double idf = log(size / termfreq);
	double wt = tf * idf;

	weights[term] = wt;
	magnitude += wt * wt;
    }
}

Centroid::Centroid(const Point &point) {
    LOGCALL_CTOR(API, "Centroid", point);
    for (TermIterator it = point.termlist_begin(); it != point.termlist_end(); ++it)
	weights[*it] = point.get_weight(*it);
    magnitude = point.get_magnitude();
}

void
Centroid::divide(double cluster_size)
{
    LOGCALL_VOID(API, "Centroid::divide", cluster_size);
    magnitude = 0;
    unordered_map<string, double>::iterator it;
    for (it = weights.begin(); it != weights.end(); ++it) {
	double new_weight = it->second / cluster_size;
	it->second = new_weight;
	magnitude += new_weight * new_weight;
    }
}

void
Centroid::clear()
{
    LOGCALL_VOID(API, "Centroid::clear", NO_ARGS);
    weights.clear();
}

Cluster&
Cluster::operator=(const Cluster &) = default;

Cluster::Cluster(const Cluster &) = default;

Cluster::Cluster(Cluster &&) = default;

Cluster&
Cluster::operator=(Cluster &&) = default;

Cluster::Cluster() : internal(new Xapian::Cluster::Internal)
{
    LOGCALL_CTOR(API, "Cluster", NO_ARGS);
}

Cluster::Cluster(const Centroid &centroid)
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
Cluster::get_documents() const
{
    LOGCALL(API, DocumentSet, "Cluster::get_documents", NO_ARGS);
    return internal->get_documents();
}

DocumentSet
Cluster::Internal::get_documents() const
{
    DocumentSet docs;
    for (auto&& point : cluster_docs)
	docs.add_document(point.get_document());
    return docs;
}

Point&
Cluster::operator[](Xapian::doccount i)
{
    return internal->get_point(i);
}

Point&
Cluster::Internal::get_point(Xapian::doccount i)
{
    return cluster_docs[i];
}

const Point&
Cluster::operator[](Xapian::doccount i) const
{
    return internal->get_point(i);
}

const Point&
Cluster::Internal::get_point(Xapian::doccount i) const
{
    return cluster_docs[i];
}

ClusterSet&
ClusterSet::operator=(const ClusterSet &) = default;

ClusterSet::ClusterSet(const ClusterSet &) = default;

ClusterSet&
ClusterSet::operator=(ClusterSet &&) = default;

ClusterSet::ClusterSet(ClusterSet &&) = default;

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
ClusterSet::Internal::add_cluster(const Cluster &cluster)
{
    clusters.push_back(cluster);
}

void
ClusterSet::add_cluster(const Cluster &cluster)
{
    LOGCALL_VOID(API, "ClusterSet::add_cluster", cluster);
    internal->add_cluster(cluster);
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

const Cluster&
ClusterSet::Internal::get_cluster(doccount i) const
{
    return clusters[i];
}

const Cluster&
ClusterSet::operator[](doccount i) const
{
    return internal->get_cluster(i);
}

void
ClusterSet::Internal::add_to_cluster(const Point &point, unsigned int index)
{
    clusters[index].add_point(point);
}

void
ClusterSet::add_to_cluster(const Point &point, unsigned int index)
{
    LOGCALL_VOID(API, "ClusterSet::add_to_cluster", point | index);
    internal->add_to_cluster(point, index);
}

void
ClusterSet::Internal::recalculate_centroids()
{
    for (auto&& cluster : clusters)
	cluster.recalculate();
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
    for (auto&& cluster : clusters)
	cluster.clear();
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
Cluster::add_point(const Point &point)
{
    LOGCALL_VOID(API, "Cluster::add_point", point);
    internal->add_point(point);
}

void
Cluster::Internal::add_point(const Point &point)
{
    cluster_docs.push_back(point);
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

const Centroid&
Cluster::get_centroid() const
{
    LOGCALL(API, Centroid, "Cluster::get_centroid", NO_ARGS);
    return internal->get_centroid();
}

const Centroid&
Cluster::Internal::get_centroid() const
{
    return centroid;
}

void
Cluster::set_centroid(const Centroid &centroid_)
{
    LOGCALL_VOID(API, "Cluster::set_centroid", centroid_);
    internal->set_centroid(centroid_);
}

void
Cluster::Internal::set_centroid(const Centroid &centroid_)
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
    for (const Point& temp : cluster_docs) {
	for (TermIterator titer = temp.termlist_begin(); titer != temp.termlist_end(); ++titer)
	    centroid.add_weight(*titer, temp.get_weight(*titer));
    }
    centroid.divide(size());
}

StemStopper::StemStopper(const Stem &stemmer_, stem_strategy strategy)
    : stem_action(strategy), stemmer(stemmer_)
{
    LOGCALL_CTOR(API, "StemStopper", stemmer_ | strategy);
}

string
StemStopper::get_description() const
{
    string desc("Xapian::StemStopper(");
    unordered_set<string>::const_iterator i;
    for (i = stop_words.begin(); i != stop_words.end(); ++i) {
	if (i != stop_words.begin()) desc += ' ';
	desc += *i;
    }
    desc += ')';
    return desc;
}

void
StemStopper::add(const string &term)
{
    LOGCALL_VOID(API, "StemStopper::add", term);
    switch (stem_action) {
	case STEM_NONE:
	    stop_words.insert(term);
	    break;
	case STEM_ALL_Z:
	    stop_words.insert('Z' + stemmer(term));
	    break;
	case STEM_ALL:
	    stop_words.insert(stemmer(term));
	    break;
	case STEM_SOME:
	case STEM_SOME_FULL_POS:
	    stop_words.insert(term);
	    stop_words.insert('Z' + stemmer(term));
	    break;
    }
}
