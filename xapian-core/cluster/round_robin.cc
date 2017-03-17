/** @file round_robin.cc
 *  @brief Round Robin clustering API
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

using namespace Xapian;
using namespace std;

string
RoundRobin::get_description() const
{
    return "RoundRobin()";
}

ClusterSet
RoundRobin::cluster(const MSet &mset)
{
    LOGCALL(API, ClusterSet, "RoundRobin::cluster", mset);
    TermListGroup tlg(mset);
    ClusterSet cset;
    vector<Point> points;

    for (MSetIterator it = mset.begin(); it != mset.end(); ++it)
	points.push_back(Point(tlg, it.get_document()));

    unsigned int i = 0;
    while (i < num_of_clusters) {
	Cluster cluster_rr;
	cset.add_cluster(cluster_rr);
	i++;
    }

    unsigned int size = points.size();
    for (i = 0; i < size; ++i)
	cset.add_to_cluster(points[i], i % num_of_clusters);

    return cset;
}
