/* delve.cc - Allow inspection of the contents of a Xapian database
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004 Olly Betts
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

#include <xapian.h>

#include <algorithm>
#include <iostream>
#include <vector>

#include "gnu_getopt.h"

using namespace Xapian;
using namespace std;

static char separator = ' ';

static bool verbose = false;
static bool showvalues = false;
static bool showdocdata = false;

static void
syntax(const char *progname)
{
    cout << "Syntax: " << progname << " [<options>] <database>...\n"
	"  -r <recno>            for term list(s)\n"
	"  -t <term>             for posting list(s)\n"
	"  -t <term> -r <recno>  for position list(s)\n"
	"  -1                    output one list entry per line\n"
	"  -k                    output values for each document referred to\n"
	"  -d                    output document data for each document\n"
	"  -v                    extra info (wdf and len for postlist;\n"
	"                        wdf termfreq for termlist; number of terms for db)\n";
    exit(1);
}

static void
show_db_stats(Database &db)
{
    // Display a few database stats.
    cout << "number of documents = " << db.get_doccount() << endl;
    cout << "average document length = " << db.get_avlength() << endl;
    if (verbose) {
	// To find the number of terms, we have to count them!
	// This will take a few seconds or minutes, so only do it if -v
	// was specified.
	termcount terms = 0;
	TermIterator t = db.allterms_begin();
	const TermIterator end = db.allterms_end();
	while (t != end) {
	    ++terms;
	    ++t;
	}
	cout << "number of unique terms = " << terms << endl;
    }
}

static void
show_values(Database &db, docid docid, char sep)
{
    Document doc = db.get_document(docid);
    ValueIterator v = doc.values_begin();
    ValueIterator vend = doc.values_end();
    while (v != vend) {
	cout << sep << v.get_valueno() << ':' << *v;
	++v;
    }
}

static void
show_values(Database &db,
	    vector<docid>::const_iterator i,
	    vector<docid>::const_iterator end)
{
    while (i != end) {
	cout << "Values for record #" << *i << ':';
	show_values(db, *i, separator);
	cout << endl;
	++i;
    }
}

static void
show_docdata(Database &db, docid docid, char sep)
{
    cout << sep << "[" << db.get_document(docid).get_data() << ']';
}

static void
show_docdata(Database &db,
	     vector<docid>::const_iterator i,
	     vector<docid>::const_iterator end)
{
    while (i != end) {
	cout << "Data for record #" << *i << ':' << endl;
	cout << db.get_document(*i).get_data() << endl;
	++i;
    }
}

static void
show_termlists(Database &db,
	       vector<docid>::const_iterator i,
	       vector<docid>::const_iterator end)
{
    // Display termlists
    while (i != end) {
	TermIterator t = db.termlist_begin(*i);
	TermIterator tend = db.termlist_end(*i);
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
main(int argc, char **argv)
{
    vector<docid> recnos;
    vector<string> terms;
    vector<string> dbs;

    int c;
    while ((c = gnu_getopt(argc, argv, "r:t:1vkd")) != EOF) {
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
    
    Database db;
    {
	vector<string>::const_iterator i;
	for (i = dbs.begin(); i != dbs.end(); i++) {
	    try {
		db.add_database(Auto::open(*i));
	    } catch (const Error &e) {
		cout << "Error opening database `" << *i << "': " << e.get_msg()
		     << endl;
		return 1;
	    }
	}
    }

    try {
	if (terms.empty() && recnos.empty()) {
	    show_db_stats(db);
	    return 0;
	}

	if (!recnos.empty()) {
	    if (showvalues) {
		show_values(db, recnos.begin(), recnos.end());
	    }

	    if (showdocdata) {
		show_docdata(db, recnos.begin(), recnos.end());
	    }
	}

	if (terms.empty()) {
	    show_termlists(db, recnos.begin(), recnos.end());
	    return 0;
	}

	vector<string>::const_iterator i;
	for (i = terms.begin(); i != terms.end(); i++) {
	    string term = *i;
	    Stem stemmer("english");
	    if (*(term.end() - 1) == '.') {
		term.erase(term.size() - 1);
	    } else {
		term = stemmer.stem_word(term);
	    }
	    PostingIterator p = db.postlist_begin(term);
	    PostingIterator pend = db.postlist_end(term);
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
		vector<docid>::const_iterator j;
		for (j = recnos.begin(); j != recnos.end(); j++) {
		    p.skip_to(*j);
		    if (p == pend || *p != *j) {
			cout << "term `" << term <<
			    "' doesn't index document #" << *j << endl;
		    } else {
			cout << "Position List for term `" << term
			    << "', record #" << *j << ':';
			try {
			    PositionIterator pos = p.positionlist_begin();
			    PositionIterator posend = p.positionlist_end();
			    while (pos != posend) {
				cout << separator << *pos;
				++pos;
			    }
			    cout << endl;
			} catch (const Error &e) {
			    cout << "Error: " << e.get_msg() << endl;
			}
		    }
		}
	    }
	}
    } catch (const Error &e) {
	cout << "Error: " << e.get_msg() << endl;
	return 1;
    }
}
