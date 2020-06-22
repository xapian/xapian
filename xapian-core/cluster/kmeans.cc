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
#include "xapian/error.h"

#include "debuglog.h"

#include <limits>
#include <vector>

// Threshold value for checking convergence in KMeans
#define CONVERGENCE_THRESHOLD 0.0000000001

/** Maximum number of times KMeans algorithm will iterate
 *  till it converges
 */
#define MAX_ITERS 1000

using namespace Xapian;
using namespace std;

KMeans::KMeans(unsigned int k_, unsigned int max_iters_)
    : k(k_)
{
    LOGCALL_CTOR(API, "KMeans", k_ | max_iters_);
    max_iters = (max_iters_ == 0) ? MAX_ITERS : max_iters_;
    if (k_ == 0)
	throw InvalidArgumentError("Number of required clusters should be greater than zero");
}

string
KMeans::get_description() const
{
    return "KMeans()";
}

void
KMeans::set_stopper(const Stopper *stopper_)
{
    LOGCALL_VOID(API, "KMeans::set_stopper", stopper_);
    stopper = stopper_;
}

void
KMeans::initialise_clusters(ClusterSet &cset, doccount num_of_points)
{
    LOGCALL_VOID(API, "KMeans::initialise_clusters", cset | num_of_points);
    // Initial centroids are selected by picking points at roughly even
    // intervals within the MSet. This is cheap and helps pick diverse
    // elements since the MSet is usually sorted by some sort of key
    for (unsigned int i = 0; i < k; ++i) {
	unsigned int x = (i * num_of_points) / k;
	cset.add_cluster(Cluster(Centroid(points[x])));
    }
}

void
KMeans::initialise_points(const MSet &source)
{
    LOGCALL_VOID(API, "KMeans::initialise_points", source);
    TermListGroup tlg(source, stopper.get());
    for (MSetIterator it = source.begin(); it != source.end(); ++it)
	points.push_back(Point(tlg, it.get_document()));
}

ClusterSet
KMeans::cluster(const MSet &mset)
{
    LOGCALL(API, ClusterSet, "KMeans::cluster", mset);
    doccount size = mset.size();
    if (k >= size)
	k = size;
    initialise_points(mset);
    ClusterSet cset;
    initialise_clusters(cset, size);
    CosineDistance distance;
    vector<Centroid> previous_centroids;
    for (unsigned int i = 0; i < max_iters; ++i) {
	// Assign each point to the cluster corresponding to its
	// closest cluster centroid
	cset.clear_clusters();
	for (unsigned int j = 0; j < size; ++j) {
	    double closest_cluster_distance = numeric_limits<double>::max();
	    unsigned int closest_cluster = 0;
	    for (unsigned int c = 0; c < k; ++c) {
		const Centroid& centroid = cset[c].get_centroid();
		double dist = distance.similarity(points[j], centroid);
		if (closest_cluster_distance > dist) {
		    closest_cluster_distance = dist;
		    closest_cluster = c;
		}
	    }
	    cset.add_to_cluster(points[j], closest_cluster);
	}

	// Remember the previous centroids
	previous_centroids.clear();
	for (unsigned int j = 0; j < k; ++j)
	    previous_centroids.push_back(cset[j].get_centroid());

	// Recalculate the centroids for current iteration
	cset.recalculate_centroids();

	// Check whether centroids have converged
	bool has_converged = true;
	for (unsigned int j = 0; j < k; ++j) {
	    const Centroid& centroid = cset[j].get_centroid();
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
