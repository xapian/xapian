/** @file
 * @brief test database contents and consistency.
 */
/* Copyright 2009 Richard Boulton
 * Copyright 2010,2015 Olly Betts
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

#include "dbcheck.h"

#include "str.h"
#include "testsuite.h"

using namespace std;

string
positions_to_string(Xapian::PositionIterator & it,
		    const Xapian::PositionIterator & end,
		    Xapian::termcount * count)
{
    string result;
    bool need_comma = false;
    Xapian::termcount c = 0;
    while (it != end) {
	if (need_comma)
	    result += ", ";
	result += str(*it);
	need_comma = true;
	++it;
	++c;
    }
    if (count) {
	*count = c;
    }
    return result;
}

string
postlist_to_string(const Xapian::Database & db, const string & tname)
{
    string result;
    bool need_comma = false;

    for (Xapian::PostingIterator p = db.postlist_begin(tname);
	 p != db.postlist_end(tname);
	 ++p) {
	if (need_comma)
	    result += ", ";

	Xapian::PositionIterator it(p.positionlist_begin());
	string posrepr = positions_to_string(it, p.positionlist_end());
	if (!posrepr.empty()) {
	    posrepr = ", pos=[" + posrepr + "]";
	}

	result += "(" + str(*p) +
		", doclen=" + str(p.get_doclength()) +
		", wdf=" + str(p.get_wdf()) +
		posrepr + ")";
	need_comma = true;
    }
    return result;
}

string
docterms_to_string(const Xapian::Database & db, Xapian::docid did)
{
    string result;
    bool need_comma = false;

    for (Xapian::TermIterator t = db.termlist_begin(did);
	 t != db.termlist_end(did);
	 ++t) {
	Xapian::PositionIterator it(t.positionlist_begin());
	string posrepr = positions_to_string(it, t.positionlist_end());
	if (!posrepr.empty()) {
	    posrepr = ", pos=[" + posrepr + "]";
	}
	if (need_comma)
	    result += ", ";
	result += "Term(" + *t + ", wdf=" + str(t.get_wdf()) + posrepr;
	result += ")";
	need_comma = true;
    }
    return result;
}

string
docstats_to_string(const Xapian::Database & db, Xapian::docid did)
{
    string result;

    result += "len=" + str(db.get_doclength(did));

    return result;
}

string
termstats_to_string(const Xapian::Database & db, const string & term)
{
    string result;

    result += "tf=" + str(db.get_termfreq(term));
    result += ",cf=" + str(db.get_collection_freq(term));

    return result;
}

void
dbcheck(const Xapian::Database & db,
	Xapian::doccount expected_doccount,
	Xapian::docid expected_lastdocid)
{
    TEST_EQUAL(db.get_doccount(), expected_doccount);
    TEST_EQUAL(db.get_lastdocid(), expected_lastdocid);

    // Note - may not be a very big type, but we're only expecting to use this
    // for small databases, so should be fine.
    unsigned long totlen = 0;

    // A map from term to a representation of the posting list for that term.
    // We build this up from the documents, and then check it against the
    // equivalent built up from the posting lists.
    map<string, string> posting_reprs;
    map<Xapian::valueno, string> value_reprs;

    Xapian::termcount doclen_lower_bound = Xapian::termcount(-1);
    Xapian::termcount doclen_upper_bound = 0;

    for (Xapian::PostingIterator dociter = db.postlist_begin(string());
	 dociter != db.postlist_end(string());
	 ++dociter) {
	Xapian::docid did = *dociter;
	TEST_EQUAL(dociter.get_wdf(), 1);
	Xapian::Document doc(db.get_document(did));
	Xapian::termcount doclen(db.get_doclength(did));
	Xapian::termcount unique_terms(db.get_unique_terms(did));
	if (doclen < doclen_lower_bound)
	    doclen_lower_bound = doclen;
	if (doclen > doclen_upper_bound)
	    doclen_upper_bound = doclen;
	totlen += doclen;

	Xapian::termcount found_termcount = 0;
	Xapian::termcount found_unique_terms = 0;
	Xapian::termcount wdf_sum = 0;
	Xapian::TermIterator t, t2;
	for (t = doc.termlist_begin(), t2 = db.termlist_begin(did);
	     t != doc.termlist_end();
	     ++t, ++t2) {
	    TEST(t2 != db.termlist_end(did));

	    ++found_termcount;
	    auto wdf = t.get_wdf();
	    if (wdf) ++found_unique_terms;
	    wdf_sum += wdf;

	    TEST_EQUAL(*t, *t2);
	    TEST_EQUAL(t.get_wdf(), t2.get_wdf());
	    TEST_EQUAL(db.get_termfreq(*t), t.get_termfreq());
	    TEST_EQUAL(db.get_termfreq(*t), t2.get_termfreq());

	    // Check the position lists are equal.
	    Xapian::termcount tc1, tc2;
	    Xapian::PositionIterator it1(t.positionlist_begin());
	    string posrepr = positions_to_string(it1, t.positionlist_end(), &tc1);
	    Xapian::PositionIterator it2(t2.positionlist_begin());
	    string posrepr2 = positions_to_string(it2, t2.positionlist_end(), &tc2);
	    TEST_EQUAL(posrepr, posrepr2);
	    TEST_EQUAL(tc1, tc2);
	    try {
		TEST_EQUAL(tc1, t.positionlist_count());
	    } catch (const Xapian::UnimplementedError &) {
		// positionlist_count() isn't implemented for remote databases.
	    }

	    // Make a representation of the posting.
	    if (!posrepr.empty()) {
		posrepr = ",[" + posrepr + "]";
	    }
	    string posting_repr = "(" + str(did) + "," +
		    str(t.get_wdf()) + "/" + str(doclen) +
		    posrepr + ")";

	    // Append the representation to the list for the term.
	    map<string, string>::iterator i = posting_reprs.find(*t);
	    if (i == posting_reprs.end()) {
		posting_reprs[*t] = posting_repr;
	    } else {
		i->second += "," + posting_repr;
	    }
	}

	Xapian::termcount vcount = 0;
	for (Xapian::ValueIterator v = doc.values_begin();
	     v != doc.values_end();
	     ++v, ++vcount) {
	    TEST((*v).size() != 0);
	    string value_repr = "(" + str(did) + "," + *v + ")";

	    // Append the values to the value lists.
	    map<Xapian::valueno, string>::iterator i;
	    i = value_reprs.find(v.get_valueno());
	    if (i == value_reprs.end()) {
		value_reprs[v.get_valueno()] = value_repr;
	    } else {
		i->second += "," + value_repr;
	    }
	}
	TEST_EQUAL(vcount, doc.values_count());
	TEST(t2 == db.termlist_end(did));
	Xapian::termcount expected_termcount = doc.termlist_count();
	TEST_EQUAL(expected_termcount, found_termcount);
	// Ideally this would be equal, but currently we don't store the
	// unique_terms values but calculate them, and scanning the termlist
	// of each document would be slow, so instead get_unique_terms(did)
	// returns min(doclen, termcount) at present.
	TEST_REL(unique_terms, >=, found_unique_terms);
	TEST_REL(unique_terms, <=, found_termcount);
	TEST_REL(unique_terms, <=, doclen);
	TEST_EQUAL(doclen, wdf_sum);
    }

    TEST_REL(doclen_lower_bound, >=, db.get_doclength_lower_bound());
    TEST_REL(doclen_upper_bound, <=, db.get_doclength_upper_bound());

    Xapian::TermIterator t;
    map<string, string>::const_iterator i;
    for (t = db.allterms_begin(), i = posting_reprs.begin();
	 t != db.allterms_end();
	 ++t, ++i) {
	TEST(db.term_exists(*t));
	TEST(i != posting_reprs.end());
	TEST_EQUAL(i->first, *t);

	Xapian::doccount tf_count = 0;
	Xapian::termcount cf_count = 0;
	Xapian::termcount wdf_upper_bound = 0;
	string posting_repr;
	bool need_comma = false;
	for (Xapian::PostingIterator p = db.postlist_begin(*t);
	     p != db.postlist_end(*t);
	     ++p) {
	    if (need_comma) {
		posting_repr += ",";
	    }

	    ++tf_count;
	    cf_count += p.get_wdf();

	    Xapian::PositionIterator it(p.positionlist_begin());
	    string posrepr = positions_to_string(it, p.positionlist_end());
	    if (!posrepr.empty()) {
		posrepr = ",[" + posrepr + "]";
	    }
	    posting_repr += "(" + str(*p) + "," +
		    str(p.get_wdf()) + "/" +
		    str(p.get_doclength()) + posrepr + ")";
	    if (wdf_upper_bound < p.get_wdf())
		wdf_upper_bound = p.get_wdf();
	    need_comma = true;
	}

	TEST_EQUAL(posting_repr, i->second);
	TEST_EQUAL(tf_count, t.get_termfreq());
	TEST_EQUAL(tf_count, db.get_termfreq(*t));
	TEST_EQUAL(cf_count, db.get_collection_freq(*t));
	TEST_REL(wdf_upper_bound, <=, db.get_wdf_upper_bound(*t));
    }
    TEST(i == posting_reprs.end());

    map<Xapian::valueno, string>::const_iterator j;
    for (j = value_reprs.begin(); j != value_reprs.end(); ++j) {
	string value_repr;
	string value_lower_bound;
	string value_upper_bound;
	bool first = true;
	for (Xapian::ValueIterator v = db.valuestream_begin(j->first);
	     v != db.valuestream_end(j->first); ++v) {
	    if (first) {
		value_lower_bound = *v;
		value_upper_bound = *v;
		first = false;
	    } else {
		value_repr += ",";
		if (*v > value_upper_bound) {
		    value_upper_bound = *v;
		}
		if (*v < value_lower_bound) {
		    value_lower_bound = *v;
		}
	    }
	    value_repr += "(" + str(v.get_docid()) + "," + *v + ")";
	}
	TEST_EQUAL(value_repr, j->second);
	try {
	    TEST_REL(value_upper_bound, <=, db.get_value_upper_bound(j->first));
	    TEST_REL(value_lower_bound, >=, db.get_value_lower_bound(j->first));
	} catch (const Xapian::UnimplementedError &) {
	    // Skip the checks if the methods to get the bounds aren't
	    // implemented for this backend.
	}
    }

    if (expected_doccount == 0) {
	TEST_EQUAL(0, db.get_avlength());
    } else {
	TEST_EQUAL_DOUBLE(double(totlen) / expected_doccount,
			  db.get_avlength());
    }
}
