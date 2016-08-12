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

#include <debuglog.h>

using namespace Xapian;
using namespace std;

RoundRobin::~RoundRobin()
{
    LOGCALL_DTOR(API, "RoundRobin");
}

string
RoundRobin::get_description() {
    LOGCALL(API, string, "RoundRobin::get_description()", "");
    return "Round Robin clusterer";
}

ClusterSet
RoundRobin::cluster(MSet &mset, unsigned int k) {
    LOGCALL(API, ClusterSet, "RoundRobin::cluster()", mset | k);
    MSetDocumentSource docs(mset);
    TermListGroup tlg;
    tlg.add_documents(docs);
    ClusterSet cset;
    vector<Cluster> clusters;
    int i=0;
    while (!docs.at_end()) {
	Point p;
	p.initialize(tlg, docs.next_document());
	clusters[i%k].add_cluster(p);
	i++;
    }
    int size = clusters.size();
    for(i=0;i<size;i++)
	cset.add_cluster(clusters[i]);
    return cset;
}
