/* delve.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#include <stdio.h>

#include <om/om.h>

#include "../common/database.h"
#include "../common/termlist.h"
#include "../common/postlist.h"

#include <vector>
#include <stack>
#include <algorithm>

using std::string;
using std::cout;
using std::endl;

int
main(int argc, char *argv[])
{
    const char *progname = argv[0];
    argv++;
    argc--;
    
    std::vector<om_docid> recnos;
    std::vector<om_termname> terms;
    std::vector<string> dbs;

    char separator = ' ';

    bool verbose = false;

    bool syntax_error = false;
    while (argc && argv[0][0] == '-') {
	if (argc >= 2 && strcmp(argv[0], "-r") == 0) {
	    recnos.push_back(atoi(argv[1]));
	    argc -= 2;
	    argv += 2;
	} else if (argc >= 2 && strcmp(argv[0], "-t") == 0) {
	    terms.push_back(argv[1]);
	    argc -= 2;
	    argv += 2;
	} else if (argc >= 2 && strcmp(argv[0], "-d") == 0) {
	    // provided for backward compatibility
	    dbs.push_back(argv[1]);
	    argc -= 2;
	    argv += 2;
	} else if (strcmp(argv[0], "-1") == 0) {
	    separator = '\n';
	    argc--;
	    argv++;
	} else if (strcmp(argv[0], "-v") == 0) {
	    verbose = true;
	    argc--;
	    argv++;
	} else {
	    syntax_error = true;
	    break;
	}
    }

    while (argc && *argv[0] != '-') {
	dbs.push_back(*argv);
	argc--;
	argv++;
    }

    if (syntax_error || argc != 0 || (recnos.empty() && terms.empty())) {
	cout << "Syntax:\t" << progname << " <options> <database>...\n"
		"\t-r <recno>            for termlist(s)\n"
		"\t-t <term>             for posting list(s)\n"
		"\t-t <term> -r <recno>  for position list(s)\n"
		"\t-1                    output one list entry per line\n"
		"\t-v                    extra info (wdf len for postlist; wdf termfreq for termlist)\n";
	exit(1);
    }

    sort(recnos.begin(), recnos.end());
    
    try {
	OmDatabase db;
	{
	    std::vector<string>::const_iterator i;
	    for (i = dbs.begin(); i != dbs.end(); i++) {
		OmSettings params;
		params.set("backend", "auto");
		params.set("auto_dir", *i);
		db.add_database(params);
	    }
	}

	if (terms.empty()) {
	    // Display termlists
	    std::vector<om_docid>::const_iterator i;
	    for (i = recnos.begin(); i != recnos.end(); i++) {
		OmTermListIterator t = db.termlist_begin(*i);
		OmTermListIterator tend = db.termlist_end(*i);
		cout << "Term List for record #" << *i << ':';
		while (t != tend) {
		    cout << separator << *t;
		    if (verbose) {
			cout << ' ' << t.get_wdf()
			     << ' ' << t.get_termfreq();
		    }
		    t++;
		}
		cout << endl;
	    }
	} else {
	    std::vector<om_termname>::const_iterator j;
	    for (j = terms.begin(); j != terms.end(); j++) {
		om_termname term = *j;
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
			p++;
		    }
		    cout << endl;
		} else {
		    // Display position lists
		    std::vector<om_docid>::const_iterator i;
		    for (i = recnos.begin(); i != recnos.end(); i++) {
			p.skip_to(*i);
			if (p == pend || *p != *i) {
			    cout << "term `" << term <<
				    "' doesn't index document #" << *i << endl;
			} else {
			    cout << "Position List for term `" << term
				    << "', record #" << *i << ':';
			    try {
				OmPositionListIterator pos = p.positionlist_begin();
				OmPositionListIterator posend = p.positionlist_end();
				while (pos != posend) {
				    cout << separator << *pos;
				    pos++;
				}
				cout << endl;
			    } catch (OmError &e) {
				cout << "Error: " << e.get_msg() << endl;
			    }
			}
		    }
		}
	    }
	}
    }
    catch (OmError &e) {
	cout << "Error: " << e.get_msg() << endl;
    }
}
