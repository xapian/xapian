/** @file euclidian_sim.cc
 *  @brief Document similarity calculation
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

#include <debuglog.h>
#include <api/termlist.h>

#include <vector>
#include <map>
#include <cmath>

using namespace std;
using namespace Xapian;

string
EuclidianDistance::get_description() {
    LOGCALL(API, string, "EuclidianDistance::get_description()", "");
    return "Euclidian Distance metric";
}

double
EuclidianDistance::similarity(TermListGroup tlg,
			      TermIterator a_begin,
			      TermIterator a_end,
			      TermIterator b_begin,
			      TermIterator b_end)
{
    LOGCALL(API, double, "EuclidianDistance::similarity()", tlg | a_begin | a_end | b_begin | b_end);
    vector<pair<string, double> > docvec1;
    vector<pair<string, double> > docvec2;
    docvec1.clear();
    docvec2.clear();

    double doccount = tlg.get_doccount();
    TermIterator titer = a_begin;

    // Creating document vector for first document
    for (; titer != a_end; titer++) {
	double tf, wt;
	tf = 1 + log(titer.get_wdf());
	double idf;
	double termfreq = tlg.get_termfreq(*titer);
	idf = log(doccount/termfreq);
	wt = tf*idf;
	docvec1.push_back(make_pair(*titer, wt));
    }

    titer = b_begin;
    // Creating document vector for the second document
    for (; titer != b_end; titer++) {
	double tf, wt;
	tf = 1 + log(titer.get_wdf());
	double idf;
	double termfreq = tlg.get_termfreq(*titer);
	idf = log(doccount/termfreq);
	wt = tf*idf;
	docvec2.push_back(make_pair(*titer, wt));
    }

    vector<pair<string, double> >::iterator ta;
    vector<pair<string, double> >::iterator tb;
    double result = 0;

    // Finding euclidian distance between the above document vectors
    for (ta = docvec1.begin(); ta != docvec1.end(); ta++) {
	bool found = false;
	double sum = 0;
	for (tb = docvec2.begin(); tb != docvec2.end(); tb++) {
	    if (tb->first == ta->first) {
		found = true;
		break;
	    }
	}

	if (found)
	    sum += (tb->second - ta->second)*(tb->second - ta->second);
	else
	    sum += (ta->second)*(ta->second);
	result += sum;
    }

    for (ta = docvec2.begin(); ta != docvec2.end(); ta++) {
	bool found = false;
	double sum = 0;
	for (tb = docvec1.begin(); tb != docvec1.end(); tb++) {
	    if (tb->first == ta->first) {
		found = true;
		break;
	    }
	}
	if (!found)
	    sum += (ta->second)*(ta->second);
	result += sum;
    }
    return result;
}
