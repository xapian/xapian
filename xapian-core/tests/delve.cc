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
    string dbdir;

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
	    dbdir = argv[1];
	    argc -= 2;
	    argv += 2;
	} else {
	    syntax_error = true;
	    break;
	}
    }

    if (syntax_error || argc != 0 || (recnos.empty() && terms.empty())) {
	cout << "Syntax:\t" << progname << " <options>\n" <<
		"\t-d <db>               to specify database\n" <<
		"\t-r <recno>            for termlist\n" <<
		"\t-t <term>             for posting list\n" <<
		"\t-t <term> -r <recno>  for position list\n";
	exit(1);
    }

    sort(recnos.begin(), recnos.end());
    
    try {
	OmSettings params;
	params.set("backend", "auto");
	params.set("auto_dir", dbdir);
	OmDatabase db(params);

	if (terms.empty()) {
	    // Display termlists
	    std::vector<om_docid>::const_iterator i;
	    for (i = recnos.begin(); i != recnos.end(); i++) {
		OmTermListIterator t = db.termlist_begin(*i);
		OmTermListIterator tend = db.termlist_end(*i);
		cout << "Term List for record #" << *i << ':';
		while (t != tend) {
		    cout << '\n' << *t;
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
			cout << '\n' << *p;
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
				    cout << '\n' << *pos;
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
