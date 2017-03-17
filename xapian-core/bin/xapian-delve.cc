/* xapian-delve.cc: Allow inspection of the contents of a Xapian database
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2006,2007,2008,2009,2010,2011,2012,2013,2014,2016 Olly Betts
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

#include <xapian.h>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <vector>

#include "gnu_getopt.h"

#include <cstring>
#include <cstdlib>
#include "safeerrno.h"

using namespace Xapian;
using namespace std;

static char separator = ' ';

static int verbose = 0;
static bool showvalues = false;
static bool showdocdata = false;
static bool count_zero_length_docs = false;

#define PROG_NAME "delve"
#define PROG_DESC "Inspect the contents of a Xapian database"

static void show_usage() {
    cout << "Usage: " PROG_NAME " [OPTIONS] DATABASE...\n\n"
"Options:\n"
"  -a                    show all terms in the database\n"
"  -A <prefix>           show all terms in the database with given prefix\n"
"  -r <recno>            for term list(s)\n"
"  -t <term>             for posting list(s)\n"
"  -t <term> -r <recno>  for position list(s)\n"
"  -s, --stemmer=LANG    set the stemming language, the default is 'none'\n"
"  -1                    output one list entry per line\n"
"  -V                    output values for each document referred to\n"
"  -V<valueno>           output value valueno for each document referred to\n"
"                        (or each document in the database if no -r options)\n"
"  -d                    output document data for each document referred to\n"
"  -z                    for db, count documents with length 0\n"
"  -v                    extra info (wdf and len for postlist;\n"
"                        wdf and termfreq for termlist; number of terms for db;\n"
"                        termfreq when showing all terms)\n"
"  -vv                   even more info (also show collection freq and wdf\n"
"                        upper bound for terms)\n"
"      --help            display this help and exit\n"
"      --version         output version information and exit" << endl;
}

static void
show_db_stats(Database &db)
{
    // Display a few database stats.
    cout << "UUID = " << db.get_uuid() << endl;
    cout << "number of documents = " << db.get_doccount() << endl;
    cout << "average document length = " << db.get_avlength() << endl;
    cout << "document length lower bound = " << db.get_doclength_lower_bound()
	 << endl;
    cout << "document length upper bound = " << db.get_doclength_upper_bound()
	 << endl;
    cout << "highest document id ever used = " << db.get_lastdocid() << endl;
    cout << boolalpha;
    cout << "has positional information = " << db.has_positions() << endl;
    cout << "currently open for writing = ";
    try {
	cout << db.locked() << endl;
    } catch (const Xapian::Error& e) {
	cout << e.get_description() << endl;
    }

    if (count_zero_length_docs) {
	Xapian::doccount empty_docs = 0;
	if (db.get_avlength() == 0) {
	    // All documents are empty.
	    empty_docs = db.get_doccount();
	} else {
	    Xapian::PostingIterator d = db.postlist_begin(string());
	    while (d != db.postlist_end(string())) {
		if (d.get_doclength() == 0)
		    ++empty_docs;
		++d;
	    }
	}
	cout << "number of zero-length documents = " << empty_docs << endl;
    }

    if (verbose) {
	// To find the number of terms, we have to count them!
	// This will take a few seconds or minutes, so only do it if -v
	// was specified.
	termcount terms = 0;
	TermIterator t = db.allterms_begin();
	while (t != db.allterms_end()) {
	    ++terms;
	    ++t;
	}
	cout << "number of distinct terms = " << terms << endl;
    }
}

static void
show_values(Database &db, docid docid, char sep)
{
    Document doc = db.get_document(docid);
    ValueIterator v = doc.values_begin();
    while (v != doc.values_end()) {
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
show_value(Database &db,
	   vector<docid>::const_iterator i,
	   vector<docid>::const_iterator end,
	   Xapian::valueno slot)
{
    while (i != end) {
	Xapian::docid did = *i;
	cout << "Value " << slot << " for record #" << did << ": "
	     << db.get_document(did).get_value(slot) << endl;
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
show_termlist(const Database &db, Xapian::docid did,
	      const char * all_pfx = NULL)
{
    TermIterator t, tend;
    if (all_pfx) {
	t = db.allterms_begin(all_pfx);
	tend = db.allterms_end(all_pfx);
	cout << "All terms in database";
	if (all_pfx[0])
	    cout << " with prefix \"" << all_pfx << "\"";
    } else {
	t = db.termlist_begin(did);
	tend = db.termlist_end(did);
	cout << "Term List for record #" << did;
    }
    if (verbose) {
	cout << " (";
	if (did != 0)
	    cout << "wdf, ";
	cout << "termfreq";
	if (verbose > 1)
	    cout << ", collection freq, wdf upper bound";
	cout << ')';
    }
    cout << ':';

    while (t != tend) {
	const string & term = *t;
	cout << separator << term;
	if (verbose) {
	    if (did != 0)
		cout << ' ' << t.get_wdf();
	    cout << ' ' << t.get_termfreq();
	    if (verbose > 1) {
		cout << ' ' << db.get_collection_freq(term)
		     << ' ' << db.get_wdf_upper_bound(term);
	    }
	}
	++t;
    }
    cout << endl;
}

static void
show_termlists(Database &db,
	       vector<docid>::const_iterator i,
	       vector<docid>::const_iterator end)
{
    // Display termlists
    while (i != end) {
	show_termlist(db, *i);
	++i;
    }
}

static Stem stemmer;

int
main(int argc, char **argv) try {
    if (argc > 1 && argv[1][0] == '-') {
	if (strcmp(argv[1], "--help") == 0) {
	    cout << PROG_NAME " - " PROG_DESC "\n\n";
	    show_usage();
	    exit(0);
	}
	if (strcmp(argv[1], "--version") == 0) {
	    cout << PROG_NAME " - " PACKAGE_STRING << endl;
	    exit(0);
	}
    }

    const char * all_terms = NULL;
    vector<docid> recnos;
    vector<string> terms;
    vector<string> dbs;

    valueno slot = 0; // Avoid "may be used uninitialised" warnings.
    bool slot_set = false;

    int c;
    while ((c = gnu_getopt(argc, argv, "aA:r:t:s:1vV::dz")) != -1) {
	switch (c) {
	    case 'a':
		all_terms = "";
		break;
	    case 'A':
		all_terms = optarg;
		break;
	    case 'r': {
		char * end;
		errno = 0;
		unsigned long n = strtoul(optarg, &end, 10);
		if (optarg == end || *end) {
		    cout << "Non-numeric document id: " << optarg << endl;
		    exit(1);
		}
		Xapian::docid did(n);
		if (errno == ERANGE || n == 0 || did != n) {
		    cout << "Document id out of range: " << optarg << endl;
		    exit(1);
		}
		recnos.push_back(did);
		break;
	    }
	    case 't':
		terms.push_back(optarg);
		break;
	    case 's':
		stemmer = Stem(optarg);
		break;
	    case '1':
		separator = '\n';
		break;
	    case 'V':
		if (optarg) {
		    char * end;
		    errno = 0;
		    unsigned long n = strtoul(optarg, &end, 10);
		    if (optarg == end || *end) {
			cout << "Non-numeric value slot: " << optarg << endl;
			exit(1);
		    }
		    slot = Xapian::valueno(n);
		    if (errno == ERANGE || slot != n) {
			cout << "Value slot out of range: " << optarg << endl;
			exit(1);
		    }
		    slot_set = true;
		} else {
		    showvalues = true;
		}
		break;
	    case 'd':
		showdocdata = true;
		break;
	    case 'v':
		++verbose;
		break;
	    case 'z':
		count_zero_length_docs = true;
		break;
	    default:
		show_usage();
		exit(1);
	}
    }

    while (argv[optind]) dbs.push_back(argv[optind++]);

    if (dbs.empty()) {
	show_usage();
	exit(1);
    }

    std::sort(recnos.begin(), recnos.end());

    Database db;
    {
	vector<string>::const_iterator i;
	for (i = dbs.begin(); i != dbs.end(); ++i) {
	    try {
		db.add_database(Database(*i));
	    } catch (const Error &e) {
		cerr << "Error opening database '" << *i << "': ";
		cerr << e.get_description() << endl;
		return 1;
	    }
	}
    }

    if (!all_terms && terms.empty() && recnos.empty() && !slot_set) {
	// Show some statistics about the database.
	show_db_stats(db);
	return 0;
    }

    if (all_terms) {
	show_termlist(db, 0, all_terms);
    }

    if (!recnos.empty()) {
	if (showvalues) {
	    show_values(db, recnos.begin(), recnos.end());
	} else if (slot_set) {
	    show_value(db, recnos.begin(), recnos.end(), slot);
	}

	if (showdocdata) {
	    show_docdata(db, recnos.begin(), recnos.end());
	}
    } else {
	if (slot_set) {
	    cout << "Value " << slot << " for each document:";
	    ValueIterator it = db.valuestream_begin(slot);
	    while (it != db.valuestream_end(slot)) {
		cout << separator << it.get_docid() << ':' << *it;
		++it;
	    }
	    cout << endl;
	}
    }

    if (terms.empty()) {
	show_termlists(db, recnos.begin(), recnos.end());
	return 0;
    }

    vector<string>::const_iterator i;
    for (i = terms.begin(); i != terms.end(); ++i) {
	string term = stemmer(*i);
	PostingIterator p = db.postlist_begin(term);
	PostingIterator pend = db.postlist_end(term);
	if (p == pend) {
	    cout << "term '" << term << "' not in database\n";
	    continue;
	}
	if (recnos.empty()) {
	    // Display posting list
	    cout << "Posting List for term '" << term << "' (termfreq "
		 << db.get_termfreq(term) << ", collfreq "
		 << db.get_collection_freq(term) << ", wdf_max "
		 << db.get_wdf_upper_bound(term) << "):";
	    while (p != pend) {
		cout << separator << *p;
		if (verbose) {
		    cout << ' ' << p.get_wdf() << ' ' << p.get_doclength();
		}
		if (showvalues) show_values(db, *p, ' ');
		if (showdocdata) show_docdata(db, *p, ' ');
		++p;
	    }
	    cout << endl;
	} else {
	    // Display position lists
	    vector<docid>::const_iterator j;
	    for (j = recnos.begin(); j != recnos.end(); ++j) {
		p.skip_to(*j);
		if (p == pend || *p != *j) {
		    cout << "term '" << term <<
			"' doesn't index document #" << *j << endl;
		} else {
		    cout << "Position List for term '" << term
			<< "', record #" << *j << ':';
		    try {
			PositionIterator pos = p.positionlist_begin();
			while (pos != p.positionlist_end()) {
			    cout << separator << *pos;
			    ++pos;
			}
			cout << endl;
		    } catch (const Error &e) {
			cerr << "Error: " << e.get_description() << endl;
		    }
		}
	    }
	}
    }
} catch (const Error &e) {
    cerr << "\nError: " << e.get_description() << endl;
    return 1;
}
