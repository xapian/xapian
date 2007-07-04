/** @file matchspy.cc
 * @brief MatchDecider subclasses for use as "match spies".
 */
/* Copyright (C) 2007 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include <xapian/document.h>
#include <xapian/matchspy.h>
#include <xapian/queryparser.h>

#include <float.h>
#include <math.h>

#include <map>
#include <vector>
#include <string>

using namespace std;

namespace Xapian {

bool
MatchSpy::operator()(const Document &doc) const
{
    ++total;
    map<Xapian::valueno, map<string, size_t> >::iterator i;
    for (i = categories.begin(); i != categories.end(); ++i) {
	Xapian::valueno valno = i->first;
	map<string, size_t> & tally = i->second;

	string val(doc.get_value(valno));
	if (!val.empty()) ++tally[val];
    }
    return true;
}

inline double sqrd(double x) { return x * x; }

double
MatchSpy::score_categorisation(Xapian::valueno valno,
			       double desired_no_of_categories)
{
    if (total == 0) return 0.0;

    const map<string, size_t> & cat = categories[valno];
    size_t total_unset = total;
    double score = 0.0;

    if (desired_no_of_categories <= 0.0)
	desired_no_of_categories = cat.size();

    double avg = double(total) / desired_no_of_categories;

    map<string, size_t>::const_iterator i;
    for (i = cat.begin(); i != cat.end(); ++i) {
	size_t count = i->second;
	total_unset -= count;
	score += sqrd(count - avg);
    }
    if (total_unset) score += sqrd(total_unset - avg);

    // Scale down so the total number of items doesn't make a difference.
    score /= sqrd(total);

    // Bias towards returning the number of categories requested.
    score += 0.01 * sqrd(desired_no_of_categories - cat.size());

    return score;
}

struct bucketval {
    bucketval()
	: count(0), min_orig(), max_orig(),
	  min_dbl(DBL_MAX), max_dbl(DBL_MIN) { }
    size_t count;
    string min_orig, max_orig;
    double min_dbl, max_dbl;
    void update(size_t n, double dbl) {
	count += n;
	if (dbl < min_dbl) {
	    min_dbl = dbl;
	}
	if (dbl > max_dbl) {
	    max_dbl = dbl;
	}
    }
};

bool
MatchSpy::build_numeric_ranges(Xapian::valueno valno, size_t max_ranges)
{
    const map<string, size_t> & cat = categories[valno];

    double lo = DBL_MAX, hi = -DBL_MAX;

    map<double, size_t> histo;
    size_t total_set = 0;
    map<string, size_t>::const_iterator i;
    for (i = cat.begin(); i != cat.end(); ++i) {
	double v = Xapian::NumberValueRangeProcessor::string_to_float(i->first.c_str());
	if (v < lo) lo = v;
	if (v > hi) hi = v;
	size_t count = i->second;
	histo[v] = count;
	total_set += count;
    }

    if (total_set == 0) {
	// No set values.
	return false;
    }
    if (lo == hi) {
	// All set values are the same.
	return false;
    }

    double sizeby = max(fabs(hi), fabs(lo));
    // E.g. if sizeby = 27.4 and max_ranges = 7, we want to split into units of
    // width 1.0 which we may then coalesce if there are too many used buckets.
    double unit = pow(10.0, floor(log10(sizeby / max_ranges) - 0.2));
    double start = floor(lo / unit) * unit;
    size_t n_buckets = size_t(ceil(hi / unit) - floor(lo / unit));

    bool scaleby2 = true;
    vector<bucketval> bucket(n_buckets + 1);
    while (true) {
	size_t n_used = 0;
	map<double, size_t>::const_iterator j;
	for (j = histo.begin(); j != histo.end(); ++j) {
	    double v = j->first;
	    size_t b = size_t(floor((v - start) / unit));
	    if (bucket[b].count == 0) ++n_used;
	    bucket[b].update(j->second, v);
	}

	if (n_used <= max_ranges) break;

	unit *= scaleby2 ? 2.0 : 2.5;
	scaleby2 = !scaleby2;
	start = floor(lo / unit) * unit;
	n_buckets = size_t(ceil(hi / unit) - floor(lo / unit));
	bucket.resize(0);
	bucket.resize(n_buckets + 1);
    }

    map<string, size_t> discrete_categories;
    for (size_t b = 0; b < bucket.size(); ++b) {
	if (bucket[b].count == 0) continue;
	string desc;
	if (bucket[b].min_dbl == bucket[b].max_dbl) {
	    desc = bucket[b].min_orig;
	} else {
	    desc = bucket[b].min_orig + " to " + bucket[b].max_orig;
	}
	discrete_categories[desc] = bucket[b].count;
	//cout << "  " << desc << ": " << bucket[b].count << endl;
    }

    size_t total_unset = total - total_set;
    if (total_unset) {
	//cout << "  Unset " << total_unset << endl;
    }

    return true; //return score_split(discrete_categories, total_unset, total);
}

}
