/** @file weightinternal.cc
 * @brief Xapian::Weight::Internal class, holding database and term statistics.
 */
/* Copyright (C) 2007 Lemur Consulting Ltd
 * Copyright (C) 2009 Olly Betts
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <config.h>

#include "weightinternal.h"

#include "omassert.h"
#include "utils.h" // For om_tostring().

using namespace std;

namespace Xapian {

Weight::Internal &
Weight::Internal::operator +=(const Weight::Internal & inc)
{
    total_length += inc.total_length;
    collection_size += inc.collection_size;
    rset_size += inc.rset_size;

    // Add termfreqs and reltermfreqs
    map<string, Xapian::doccount>::const_iterator i;
    for (i = inc.termfreq.begin(); i != inc.termfreq.end(); ++i) {
	termfreq[i->first] += i->second;
    }
    for (i = inc.reltermfreq.begin(); i != inc.reltermfreq.end(); ++i) {
	reltermfreq[i->first] += i->second;
    }
    return *this;
}

Xapian::doccount
Weight::Internal::get_termfreq(const string & term) const
{
    // We pass an empty std::string for term when calculating the extra weight.
    if (term.empty()) return 0;

    map<string, Xapian::doccount>::const_iterator tfreq;
    tfreq = termfreq.find(term);
    Assert(tfreq != termfreq.end());
    return tfreq->second;
}

void
Weight::Internal::set_termfreq(const string & term, Xapian::doccount tfreq)
{
    // Can be called a second time, if a term occurs multiple times in the
    // query; if this happens, the termfreq should be the same each time.
    Assert(termfreq.find(term) == termfreq.end() ||
	   termfreq.find(term)->second == tfreq);
    termfreq[term] = tfreq;
}

Xapian::doccount
Weight::Internal::get_reltermfreq(const string & term) const
{
    // We pass an empty string for term when calculating the extra weight.
    if (term.empty()) return 0;

    map<string, Xapian::doccount>::const_iterator rtfreq;
    rtfreq = reltermfreq.find(term);
    Assert(rtfreq != reltermfreq.end());
    return rtfreq->second;
}

void
Weight::Internal::set_reltermfreq(const string & term, Xapian::doccount rtfreq)
{
    // Can be called a second time, if a term occurs multiple times in the
    // query; if this happens, the termfreq should be the same each time.
    Assert(reltermfreq.find(term) == reltermfreq.end() ||
	   reltermfreq.find(term)->second == rtfreq);
    reltermfreq[term] = rtfreq;
}

string
Weight::Internal::get_description() const
{
    string desc = "Weight::Internal(totlen=";
    desc += om_tostring(total_length);
    desc += ", collection_size=";
    desc += om_tostring(collection_size);
    desc += ", rset_size=";
    desc += om_tostring(rset_size);
    desc += ')';
    return desc;
}

}
