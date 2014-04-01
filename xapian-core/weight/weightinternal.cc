/** @file weightinternal.cc
 * @brief Xapian::Weight::Internal class, holding database and term statistics.
 */
/* Copyright (C) 2007 Lemur Consulting Ltd
 * Copyright (C) 2009,2010,2011,2012,2013 Olly Betts
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

#include "xapian/enquire.h"

#include "omassert.h"
#include "api/omenquireinternal.h"
#include "str.h"
#include "api/termlist.h"

#include "autoptr.h"
#include <set>

using namespace std;

string
TermFreqs::get_description() const {
    string desc("TermFreqs(termfreq=");
    desc += str(termfreq);
    desc += ", reltermfreq=";
    desc += str(reltermfreq);
    desc += ", collfreq=";
    desc += str(collfreq);
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
    total_term_count += inc.total_term_count;

    // Add termfreqs and reltermfreqs
    map<string, TermFreqs>::const_iterator i;
    for (i = inc.termfreqs.begin(); i != inc.termfreqs.end(); ++i) {
	termfreqs[i->first] += i->second;
    }
    return *this;
}

void
Weight::Internal::accumulate_stats(const Xapian::Database::Internal &subdb,
				   const Xapian::RSet &rset)
{
    total_length += subdb.get_total_length();
    collection_size += subdb.get_doccount();
    rset_size += rset.size();

    total_term_count += subdb.get_doccount() * subdb.get_total_length();
    map<string, TermFreqs>::iterator t;
    for (t = termfreqs.begin(); t != termfreqs.end(); ++t) {
	const string & term = t->first;
	t->second.termfreq += subdb.get_termfreq(term);
	t->second.collfreq += subdb.get_collection_freq(term);
    }

    const set<Xapian::docid> & items(rset.internal->get_items());
    set<Xapian::docid>::const_iterator d;
    for (d = items.begin(); d != items.end(); ++d) {
	Xapian::docid did = *d;
	Assert(did);
	// The query is likely to contain far fewer terms than the documents,
	// and we can skip the document's termlist, so look for each query term
	// in the document.
	AutoPtr<TermList> tl(subdb.open_term_list(did));
	for (t = termfreqs.begin(); t != termfreqs.end(); ++t) {
	    const string & term = t->first;
	    TermList * ret = tl->skip_to(term);
	    Assert(ret == NULL);
	    (void)ret;
	    if (tl->at_end())
		break;
	    if (term == tl->get_termname())
		++t->second.reltermfreq;
	}
    }
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
    desc += ", total_term_count=";
    desc += str(total_term_count);
    desc += ')';
    return desc;
}

}
