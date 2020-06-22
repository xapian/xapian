/** @file lcd_clusterer.cc
 *  @brief LCD clustering API
 */
/* Copyright (C) 2018 Uppinder Chugh
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
#include "omassert.h"

#include <algorithm>
#include <set>
#include <vector>

using namespace Xapian;
using namespace std;

struct pcompare {
    bool operator()(const pair<Point, double>& a,
		    const pair<Point, double>& b) const {
	return a.second > b.second;
    }
};

typedef set<pair<Point, double>, pcompare> PSet;

struct dcompare {
    bool operator()(const pair<PSet::iterator, double>& a,
		    const pair<PSet::iterator, double>& b) const {
	return a.second < b.second;
    }
};

LCDClusterer::LCDClusterer(unsigned int k_)
    : k(k_)
{
    LOGCALL_CTOR(API, "LCDClusterer", k_);
    if (k_ == 0)
	throw InvalidArgumentError("Number of required clusters should be "
				   "greater than zero");
}

string
LCDClusterer::get_description() const
{
    return "LCDClusterer()";
}

ClusterSet
LCDClusterer::cluster(const MSet &mset)
{
    LOGCALL(API, ClusterSet, "LCDClusterer::cluster", mset);

    doccount size = mset.size();
    unsigned int k_ = k;
    if (k_ >= size)
	k_ = size;

    // Store each document and its rel score from given mset
    set<pair<Point, double>, pcompare> points;

    // Initialise points
    TermListGroup tlg(mset);
    for (MSetIterator it = mset.begin(); it != mset.end(); ++it)
	points.emplace(Point(tlg, it.get_document()), it.get_weight());

    // Container for holding the clusters
    ClusterSet cset;

    // Instantiate this for computing distance between points
    CosineDistance distance;

    // First cluster center
    PSet::iterator cluster_center = points.begin();

    // The original algorithm accepts a parameter k which is the number
    // of documents in each cluster, which can be hard to tune and
    // especially when using this in conjunction with the diversification
    // module. So, for now LCD clusterer accepts k as the number of
    // clusters and divides the documents into k clusters such that among k
    // clusters, n clusters have x - 1 points and (k - n) have x points. This
    // needs to be tested on a dataset to see how well this works.
    // n * (x - 1) + (k_ - n) * x = size, where 0 <= n < k
    unsigned n = k_ - size % k_;
    unsigned x = (size / k_) + 1;
    AssertEq(n * (x - 1) + (k_ - n) * x, size);

    unsigned cnum = 1;
    while (true) {
	// Container for new cluster
	Cluster new_cluster;

	// Select (num_points - 1) nearest points to cluster_center from
	// 'points' and form a new cluster
	unsigned int num_points = cnum <= n ? x - 1 : x;

	// Store distances of each point from current cluster center
	// Iterator of each point is stored for fast deletion from 'points'
	vector<pair<PSet::iterator, double>> dist_vector;
	for (auto it = points.begin(); it != points.end(); ++it) {
	    if (it == cluster_center)
		continue;

	    double dist = distance.similarity(cluster_center->first, it->first);
	    dist_vector.push_back(make_pair(it, dist));
	}

	// Sort dist_vector in ascending order of distance
	sort(dist_vector.begin(), dist_vector.end(), dcompare());

	// Add first num_points-1 to cluster
	for (unsigned int i = 0; i < num_points - 1; ++i) {
	    auto piterator = dist_vector[i].first;
	    // Add to cluster
	    new_cluster.add_point(piterator->first);
	    // Remove from 'points'
	    points.erase(piterator);
	}

	// Add cluster_center to current cluster
	new_cluster.add_point(cluster_center->first);

	// Add cluster to cset
	cset.add_cluster(new_cluster);

	if (cnum == k_) break;

	// Remove current cluster_center from points
	points.erase(cluster_center);

	// Select a new cluster center which is the point that is farthest away
	// from the current cluster center
	cluster_center = dist_vector.back().first;

	++cnum;
    }

    return cset;
}
