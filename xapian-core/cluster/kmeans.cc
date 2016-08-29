/** @file kmeans.cc
 *  @brief KMeans clustering API
 */
/* Copyright (C) 2016 Richhiey Thomas
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
#include <debuglog.h>

#include "points.h"
#include "similarity.h"

#include <set>
#include <unordered_map>
#include <vector>

using namespace Xapian;
using namespace std;

string
KMeans::get_description() const {
    LOGCALL(API, string, "KMeans::get_description()", NO_ARGS);
    return "KMeans clusterer";
}

void
KMeans::initialize_centroids(ClusterSet &cset) {
    LOGCALL_VOID(API, "KMeans::initialize_centroids()", cset);
    if (mode == "kmeanspp")
	initialize_kmeanspp(cset);
    if (mode == "random")
	initialize_random(cset);
}

void
KMeans::initialize_random(ClusterSet &cset) {
    LOGCALL_VOID(API, "KMeans::initialize_random()", cset);
    set<docid> centroid_docs;
    int size = docs.size();
    srand(time(NULL));
    for (unsigned int i = 0; i < k;) {
	int x = rand() % size;
	if (centroid_docs.find(x) == centroid_docs.end()) {
	    Point a = docs[x];
	    Centroid centroid;
	    centroid.set_to_point(a);
	    centroids.push_back(centroid);
	    centroid_docs.insert(x);
	    Cluster cluster1(centroid);
	    cset.add_cluster(cluster1);
	    ++i;
	}
    }
}

void
KMeans::initialize_kmeanspp(ClusterSet &cset) {
    LOGCALL_VOID(API, "KMeans::initialize_kmeanspp()", cset);
    int size = docs.size();
    CosineDistance cosine;
    map<int, double> m;
    set<docid> centroid_docs;
    srand(time(NULL));
    Point a = docs[rand() % size];
    centroid_docs.insert(a.get_document().get_docid());
    Centroid centroid;
    centroid.set_to_point(a);
    centroids.push_back(centroid);
    Cluster cluster1(centroid);
    cset.add_cluster(cluster1);
    for (unsigned int i = 1; i < k; ++i) {
	double sum = 0;
	for (int j = 0; j < size; ++j) {
	    double min = 100000;
	    for (vector<Centroid>::iterator it = centroids.begin(); it != centroids.end(); ++it) {
		double dist = cosine.similarity(docs[j], *it);
		if (dist < min) {
		    min = dist;
		    m[j] = min;
		}
	    }
	    sum += min;
	}
	for (int j = 0; j < size; ++j) {
	    m[j] = m[j]/sum;
	}
	for (int j = 0; j < size; ++j) {
	    if (j==0)
		continue;
	    m[j] = m[j] + m[j-1];
	}

	double prob = rand()/double(RAND_MAX);

	bool flag = false;
	int j;
	for (j = 0; j < size; ++j) {
	    if (m[j] >= prob && centroid_docs.find(docs[j].get_document().get_docid()) == centroid_docs.end()) {
		flag = true;
		break;
	    }
	}
	if (flag) {
	    Point &x = docs[j];
	    Centroid centroid_n;
	    centroid_n.set_to_point(x);
	    centroids.push_back(centroid_n);
	    Cluster cluster_n(centroid_n);
	    cset.add_cluster(cluster_n);
	    centroid_docs.insert(x.get_document().get_docid());
	}
    }
}

void
KMeans::initialize_points(const MSet &source, TermListGroup &tlg) {
    LOGCALL_VOID(API, "KMeans::initialize_points()", source | tlg);
    for (MSetIterator it = source.begin(); it != source.end(); ++it) {
	Point p;
	p.initialize(tlg, it.get_document());
	docs.push_back(p);
    }
}

bool
KMeans::converge(vector<Centroid> &previous, vector<Centroid> &current) const {
    LOGCALL(API, bool, "KMeans::converge()", previous | current);
    CosineDistance cosine;
    for (unsigned int i = 0; i < k; ++i) {
	Centroid &prev = previous[i];
	Centroid &curr = current[i];
	double dist = cosine.similarity(prev, curr);
	if (dist > 0.0001)
	    return false;
    }
    return true;
}

ClusterSet
KMeans::cluster(MSet &mset) {
    LOGCALL(API, ClusterSet, "KMeans::cluster()", mset);
    unsigned int size = mset.size();
    mset.fetch();
    if (size <= 0)
	throw AssertionError("Size of MSet should be greater than zero");
    if (k <= 0)
	throw AssertionError("Number of clusters should be greater than zero");
    if (k > size)
	throw AssertionError("The number of clusters cannot be greater than number of documents in MSet");
    ClusterSet cset;
    TermListGroup tlg;
    tlg.add_documents(mset);
    CosineDistance cosine;
    initialize_points(mset, tlg);
    initialize_centroids(cset);
    double min1 = 10000;
    double min_cluster = 0;
    int num_iters = 100;

    vector<Centroid> prev;
    for (int i = 0; i < num_iters; ++i) {
	for (unsigned int j = 0; j < size; ++j) {
	    Point &x = docs[j];
	    min_cluster = 0;
	    min1 = 10000;
	    for (unsigned int c = 0; c < k; ++c) {
		Centroid &cent = centroids[c];
		double dist = cosine.similarity(x, cent);
		if (dist < min1) {
		    min1 = dist;
		    min_cluster = c;
		}
	    }
	    cset.add_to_cluster(x, min_cluster);
	}

	prev.clear();
	for (vector<Centroid>::iterator cit = centroids.begin(); cit != centroids.end(); ++cit)
	    prev.push_back(*cit);

	cset.recalculate_centroids();
	centroids.clear();
	for (unsigned int m = 0; m < k; ++m)
	    centroids.push_back(cset[m].get_centroid());

	if (converge(prev, centroids))
	   break;

	if (i != num_iters - 1)
	    cset.clear_clusters();
    }
    return cset;
}
