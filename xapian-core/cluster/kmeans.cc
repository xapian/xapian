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
#include <xapian/error.h>
#include <omassert.h>

#include <climits>
#include <random>
#include <set>
#include <unordered_map>
#include <vector>

/** Threshold value for checking convergence in KMeans
 */
#define EPSILON 0.0000000001

using namespace Xapian;
using namespace std;

KMeans::KMeans(unsigned int k_) {
    LOGCALL_CTOR(API, "KMeans()", k_);
    if (k == 0)
	throw InvalidArgumentError("Number of required clusters should be greater than zero");
    k = k_;
    max_iters = 100;
}

KMeans::KMeans(unsigned int k_, unsigned int max_iters_) {
    LOGCALL_CTOR(API, "KMeans()", k_ | max_iters_);
    if (k == 0)
	throw InvalidArgumentError("Number of required clusters should be greater than zero");
    k = k_;
    max_iters = max_iters_;
}

string
KMeans::get_description() const {
    LOGCALL(API, string, "KMeans::get_description()", NO_ARGS);
    return "KMeans clusterer";
}

void
KMeans::initialize_clusters(ClusterSet &cset) {
    LOGCALL_VOID(API, "KMeans::initialize_clusters()", cset);
    int size = docs.size();
    mt19937 generator;
    generator.seed(random_device()());
    uniform_int_distribution<mt19937::result_type> random(0, size);
    for (unsigned int i = 0; i < k;) {
	int x = random(generator);
	Centroid centroid(docs[x]);
	Cluster cluster(centroid);
	cset.add_cluster(cluster);
	++i;
    }
}

void
KMeans::initialize_points(const MSet &source) {
    LOGCALL_VOID(API, "KMeans::initialize_points()", source);
    TermListGroup tlg(source);
    for (MSetIterator it = source.begin(); it != source.end(); ++it) {
	Point p;
	p.initialize(tlg, it.get_document());
	docs.push_back(p);
    }
}

ClusterSet
KMeans::cluster(MSet &mset) {
    LOGCALL(API, ClusterSet, "KMeans::cluster()", mset);
    unsigned int size = mset.size();
    if (k >= size)
	k = size;
    mset.fetch();
    initialize_points(mset);
    ClusterSet cset;
    initialize_clusters(cset);
    for (unsigned int i = 0; i < max_iters; ++i) {
	/** Assign each point to ihe cluster corresponding to its
	 *  closest cluster centroid
	 */
	cset.clear_clusters();
	CosineDistance distance;
	for (unsigned int j = 0; j < size; ++j) {
	    double closest_cluster_distance = INT_MAX;
	    double closest_cluster = 0;
	    for (unsigned int c = 0; c < k; ++c) {
		double dist = distance.similarity(docs[j], cset[c].get_centroid());
		if (closest_cluster_distance > dist) {
		    closest_cluster_distance = dist;
		    closest_cluster = c;
		}
	    }
	    cset.add_to_cluster(docs[j], closest_cluster);
	}

	/** Remember the previous centroids
	 */
	previous_centroids.clear();
	for (unsigned int j = 0; j < k; ++j)
	    previous_centroids.push_back(cset[j].get_centroid());

	/** Recalculate the centroids for current iteration
	 */
	cset.recalculate_centroids();

	/** Check whether we have converged?
	 */
	bool has_converged = true;
	for (unsigned int j = 0; j < k; ++j) {
	    double dist = distance.similarity(previous_centroids[j], cset[j].get_centroid());
	    /** If distance between any two centroids has changed by
	     *  more than the threshold, then KMeans hasnt converged
	     */
	    if (dist > EPSILON) {
		has_converged = false;
		break;
	    }
	}
	/** If converged, then break from the loop
	 */
	if (has_converged)
	    break;
    }
    return cset;
}
