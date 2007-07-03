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
    return score * 1e-6;
}

}
