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
#include <memory>

using std::string;
using std::cout;
using std::endl;

int
main(int argc, char *argv[])
{
    const char *progname = argv[0];
    argv++;
    argc--;
    om_docid recno = 0;
    om_termname term;
    bool syntax_error = false;
    while (argc && argv[0][0] == '-') {
	if (argc >= 2 && strcmp(argv[0], "-r") == 0) {
	    recno = atoi(argv[1]);
	    argc -= 2;
	    argv += 2;
	} else if (argc >= 2 && strcmp(argv[0], "-t") == 0) {
	    term = argv[1];
	    argc -= 2;
	    argv += 2;
	} else {
	    syntax_error = true;
	    break;
	}
    }

    if (syntax_error || argc != 1 || (recno == 0 && term.empty())) {
	cout << "Syntax:\t" << progname << " -r <recno> <db>  for termlist\n";
	cout << "or:\t" << progname << " -t <term> <db>  for posting list\n";
	cout << "or:\t" << progname << " -t <term> -r <recno> <db> for position list\n";
	exit(1);
    }
    try {
	OmSettings params;
	params.set("backend", "auto");
	params.set("auto_dir", argv[0]);
	OmDatabase db(params);

	if (!term.empty()) {
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
		exit(0);
	    }
	    if (recno == 0) {
		cout << "Posting List for term `" << term << "':";
		while (p != pend) {
		    cout << ' ' << *p;
		    p++;
		}
		cout << endl;
	    } else {
		p.skip_to(recno);
		if (p == pend || *p != recno) {
		    cout << "term `" << term << "' doesn't index document #"
			 << recno << endl;
		    exit(0);
		}
		cout << "Position List for term `" << term
		     << "', record #" << recno << ':';
		OmPositionListIterator pos = p.positionlist_begin();
		OmPositionListIterator posend = p.positionlist_end();
		while (pos != posend) {
		    cout << ' ' << *pos;
		    pos++;
		}
		cout << endl;
	    }
	} else {
	    OmTermListIterator t = db.termlist_begin(recno);
	    OmTermListIterator tend = db.termlist_end(recno);
	    cout << "Term List for record #" << recno << ':';
	    while (t != tend) {
		cout << ' ' << *t;
		t++;
	    }
	    cout << endl;
	}
    }
    catch (OmError &e) {
	cout << e.get_msg() << endl;
    }
}
