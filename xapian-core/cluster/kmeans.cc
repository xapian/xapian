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

#include "debuglog.h"
#include "xapian/error.h"
#include "omassert.h"

#include <climits>
#include <random>
#include <set>
#include <unordered_map>
#include <vector>

// Threshold value for checking convergence in KMeans
#define CONVERGENCE_THRESHOLD 0.0000000001

using namespace Xapian;
using namespace std;

KMeans::KMeans(unsigned int k_, unsigned int max_iters_)
    : k(k_), max_iters(max_iters_) {
    LOGCALL_CTOR(API, "KMeans", k_ | max_iters_);
    if (k_ == 0)
	throw InvalidArgumentError("Number of required clusters should be greater than zero");
}

string
KMeans::get_description() const
{
    return "KMeans()";
}

void
KMeans::initialize_clusters(ClusterSet &cset)
{
    LOGCALL_VOID(API, "KMeans::initialize_clusters", cset);
    int size = docs.size();
    // Since we are using the Mersenne Twister 19937 pseudo-random number
    // generator, the period will be greater than the value of k
    // (period of mt19937 >> k). So we do not need to check whether
    // the same point has been selected more than once
    mt19937 generator;
    generator.seed(random_device()());
    uniform_int_distribution<mt19937::result_type> random(0, size - 1);
    for (unsigned int i = 0; i < k; ++i) {
	int x = random(generator);
	Centroid centroid(docs[x]);
	Cluster initial_cluster(centroid);
	cset.add_cluster(initial_cluster);
    }
}

void
KMeans::initialize_points(const MSet &source)
{
    LOGCALL_VOID(API, "KMeans::initialize_points", source);
    TermListGroup tlg(source);
    for (MSetIterator it = source.begin(); it != source.end(); ++it) {
	Point p;
	p.initialize(tlg, it.get_document());
	docs.push_back(p);
    }
}

ClusterSet
KMeans::cluster(MSet &mset)
{
    LOGCALL(API, ClusterSet, "KMeans::cluster", mset);
    unsigned int size = mset.size();
    if (k >= size)
	k = size;
    initialize_points(mset);
    ClusterSet cset;
    initialize_clusters(cset);
    CosineDistance distance;
    vector<Centroid> previous_centroids;
    for (unsigned int i = 0; i < max_iters; ++i) {
	// Assign each point to the cluster corresponding to its
	// closest cluster centroid
	cset.clear_clusters();
	for (unsigned int j = 0; j < size; ++j) {
	    double closest_cluster_distance = INT_MAX;
	    unsigned int closest_cluster = 0;
	    for (unsigned int c = 0; c < k; ++c) {
		Centroid centroid = cset[c].get_centroid();
		double dist = distance.similarity(docs[j], centroid);
		if (closest_cluster_distance > dist) {
		    closest_cluster_distance = dist;
		    closest_cluster = c;
		}
	    }
	    cset.add_to_cluster(docs[j], closest_cluster);
	}

	// Remember the previous centroids
	previous_centroids.clear();
	for (unsigned int j = 0; j < k; ++j)
	    previous_centroids.push_back(cset[j].get_centroid());

	// Recalculate the centroids for current iteration
	cset.recalculate_centroids();

	// Check whether we have converged
	bool has_converged = true;
	for (unsigned int j = 0; j < k; ++j) {
	    Centroid centroid = cset[j].get_centroid();
	    double dist = distance.similarity(previous_centroids[j], centroid);
	    // If distance between any two centroids has changed by
	    // more than the threshold, then KMeans hasn't converged
	    if (dist > CONVERGENCE_THRESHOLD) {
		has_converged = false;
		break;
	    }
	}
	// If converged, then break from the loop
	if (has_converged)
	    break;
    }
    return cset;
}
