/* delve.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002 Olly Betts
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include "om/om.h"

#include <algorithm>
#include <iostream>
#include <vector>

#include "getopt.h"

using std::string;
using std::cout;
using std::endl;
using std::vector;

static char separator = ' ';

static bool verbose = false;
static bool showvalues = false;
static bool showdocdata = false;

static void
syntax(const char *progname)
{
    cout << "Syntax: " << progname << " [<options>] <database>...\n"
	"\t-r <recno>            for term list(s)\n"
	"\t-t <term>             for posting list(s)\n"
	"\t-t <term> -r <recno>  for position list(s)\n"
	"\t-1                    output one list entry per line\n"
	"\t-k                    output values for each document referred to\n"
	"\t-d                    output document data for each document\n"
	"\t-v                    extra info (wdf and len for postlist;\n"
	"\t\t\t\twdf termfreq for termlist)\n";
    exit(1);
}

static void
show_db_stats(OmDatabase &db)
{
    // just display a few database stats
    cout << "number of documents = " << db.get_doccount() << endl;
    cout << "average document length = " << db.get_avlength() << endl;
}

static void
show_values(OmDatabase &db, om_docid docid, char sep)
{
    OmDocument doc = db.get_document(docid);
    OmValueIterator v = doc.values_begin();
    OmValueIterator vend = doc.values_end();
    while (v != vend) {
	cout << sep << v.get_valueno() << ':' << *v;
	++v;
    }
}

static void
show_values(OmDatabase &db,
	    vector<om_docid>::const_iterator i,
	    vector<om_docid>::const_iterator end)
{
    while (i != end) {
	cout << "Values for record #" << *i << ':';
	show_values(db, *i, separator);
	cout << endl;
	++i;
    }
}

static void
show_docdata(OmDatabase &db, om_docid docid, char sep)
{
    cout << sep << "[" << db.get_document(docid).get_data() << ']';
}

static void
show_docdata(OmDatabase &db,
	     vector<om_docid>::const_iterator i,
	     vector<om_docid>::const_iterator end)
{
    while (i != end) {
	cout << "Data for record #" << *i << ':' << endl;
	cout << db.get_document(*i).get_data() << endl;
	++i;
    }
}

static void
show_termlists(OmDatabase &db, vector<om_docid>::const_iterator i,
	       vector<om_docid>::const_iterator end)
{
    // Display termlists
    while (i != end) {
	OmTermIterator t = db.termlist_begin(*i);
	OmTermIterator tend = db.termlist_end(*i);
	cout << "Term List for record #" << *i << ':';
	while (t != tend) {
	    cout << separator << *t;
	    if (verbose) {
		cout << ' ' << t.get_wdf() << ' ' << t.get_termfreq();
	    }
	    ++t;
	}
	cout << endl;
	++i;
    }
}

int
main(int argc, char *argv[])
{
    vector<om_docid> recnos;
    vector<om_termname> terms;
    vector<string> dbs;

    int c;
    while ((c = getopt(argc, argv, "r:t:1vkd")) != EOF) {
	switch (c) {
	    case 'r':
		recnos.push_back(atoi(optarg));
		break;
	    case 't':
		terms.push_back(optarg);
		break;
	    case '1':
		separator = '\n';
		break;
	    case 'k':
		showvalues = true;
		break;
	    case 'd':
		showdocdata = true;
		break;
	    case 'v':
		verbose = true;
		break;
	    default:
		syntax(argv[0]);
	}
    }

    while (argv[optind]) dbs.push_back(argv[optind++]);

    if (dbs.empty()) syntax(argv[0]);

    std::sort(recnos.begin(), recnos.end());
    
    OmDatabase db;
    try {
	vector<string>::const_iterator i;
	for (i = dbs.begin(); i != dbs.end(); i++) {
	    db.add_database(OmAuto__open(*i));
	}
    } catch (const OmError &e) {
	cout << "Error opening database: " << e.get_msg() << endl;
	return 1;
    }

    try {
	if (terms.empty() && recnos.empty()) {
	    show_db_stats(db);
	    return 0;
	}

	if (!recnos.empty() && showvalues) {
	    show_values(db, recnos.begin(), recnos.end());
	}

	if (!recnos.empty() && showdocdata) {
	    show_docdata(db, recnos.begin(), recnos.end());
	}

	if (terms.empty()) {
	    show_termlists(db, recnos.begin(), recnos.end());
	    return 0;
	}

	vector<om_termname>::const_iterator i;
	for (i = terms.begin(); i != terms.end(); i++) {
	    om_termname term = *i;
	    OmStem stemmer("english");
	    if (*(term.end() - 1) == '.') {
		term = term.erase(term.size() - 1);
	    } else {
		term = stemmer.stem_word(term);
	    }
	    OmPostListIterator p = db.postlist_begin(term);
	    OmPostListIterator pend = db.postlist_end(term);
	    if (p == pend) {
		cout << "term `" << term << "' not in database\n";
		continue;
	    }
	    if (recnos.empty()) {
		// Display posting list
		cout << "Posting List for term `" << term << "':";
		while (p != pend) {
		    cout << separator << *p;
		    if (verbose) {
			cout << ' ' << p.get_wdf()
			    << ' ' << p.get_doclength();
		    }
		    if (showvalues) show_values(db, *p, ' ');
		    if (showdocdata) show_docdata(db, *p, ' ');
		    p++;
		}
		cout << endl;
	    } else {
		// Display position lists
		vector<om_docid>::const_iterator j;
		for (j = recnos.begin(); j != recnos.end(); j++) {
		    p.skip_to(*j);
		    if (p == pend || *p != *j) {
			cout << "term `" << term <<
			    "' doesn't index document #" << *j << endl;
		    } else {
			cout << "Position List for term `" << term
			    << "', record #" << *j << ':';
			try {
			    OmPositionListIterator pos = p.positionlist_begin();
			    OmPositionListIterator posend = p.positionlist_end();
			    while (pos != posend) {
				cout << separator << *pos;
				++pos;
			    }
			    cout << endl;
			} catch (const OmError &e) {
			    cout << "Error: " << e.get_msg() << endl;
			}
		    }
		}
	    }
	}
    } catch (const OmError &e) {
	cout << "Error: " << e.get_msg() << endl;
	return 1;
    }
}
