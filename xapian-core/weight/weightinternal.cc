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
#include "str.h"

using namespace std;

string
TermFreqs::get_description() const {
    string desc("TermFreqs(");
    desc += str(termfreq);
    desc += ", ";
    desc += str(reltermfreq);
    desc += ")";
    return desc;
}

namespace Xapian {

Weight::Internal &
Weight::Internal::operator +=(const Weight::Internal & inc)
{
    total_length += inc.total_length;
    collection_size += inc.collection_size;
    rset_size += inc.rset_size;

    // Add termfreqs and reltermfreqs
    map<string, TermFreqs>::const_iterator i;
    for (i = inc.termfreqs.begin(); i != inc.termfreqs.end(); ++i) {
	termfreqs[i->first] += i->second;
    }
    return *this;
}

Xapian::doccount
Weight::Internal::get_termfreq(const string & term) const
{
    // We pass an empty std::string for term when calculating the extra weight.
    if (term.empty()) return 0;

    map<string, TermFreqs>::const_iterator tfreq = termfreqs.find(term);
    Assert(tfreq != termfreqs.end());
    return tfreq->second.termfreq;
}

void
Weight::Internal::set_termfreq(const string & term, Xapian::doccount tfreq)
{
    // Can be called a second time, if a term occurs multiple times in the
    // query; if this happens, the termfreq should be the same each time.
    Assert(termfreqs.find(term) == termfreqs.end() ||
	   termfreqs.find(term)->second.termfreq == 0 ||
	   termfreqs.find(term)->second.termfreq == tfreq);
    termfreqs[term].termfreq = tfreq;
}

Xapian::doccount
Weight::Internal::get_reltermfreq(const string & term) const
{
    // We pass an empty string for term when calculating the extra weight.
    if (term.empty()) return 0;

    map<string, TermFreqs>::const_iterator tfreq = termfreqs.find(term);
    Assert(tfreq != termfreqs.end());
    return tfreq->second.reltermfreq;
}

void
Weight::Internal::set_reltermfreq(const string & term, Xapian::doccount rtfreq)
{
    // Can be called a second time, if a term occurs multiple times in the
    // query; if this happens, the reltermfreq should be the same each time.
    Assert(termfreqs.find(term) == termfreqs.end() ||
	   termfreqs.find(term)->second.reltermfreq == 0 ||
	   termfreqs.find(term)->second.reltermfreq == rtfreq);
    termfreqs[term].reltermfreq = rtfreq;
}

string
Weight::Internal::get_description() const
{
    string desc = "Weight::Internal(totlen=";
    desc += str(total_length);
    desc += ", collection_size=";
    desc += str(collection_size);
    desc += ", rset_size=";
    desc += str(rset_size);
    desc += ')';
    return desc;
}

}
