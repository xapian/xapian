/* msearch.cc
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
#include <om/omstem.h>

#ifdef MUS_DEBUG_VERBOSE
// Verbose debugging output
#define DebugMsg(a) cout << a ; cout.flush()
#else
#define DebugMsg(a)
#endif

#ifdef MUS_DEBUG
// Assertions to put in debug builds
// NB use an else clause to avoid dangling else damage
#define Assert(a) if (a) { } else throw OmAssertionError(ASSERT_LOCN(a))
#else
#define Assert(a)
#endif

#include <vector>
#include <stack>
#include <memory>

int
main(int argc, char *argv[])
{
    int msize = 10;
    int mfirst = 0;
    const char *progname = argv[0];
    OmRSet rset;
    vector<OmSettings *> dbs;
    bool showmset = true;
    bool applystem = false;
    OmQuery::op default_op = OmQuery::OP_OR;
    int collapse_key = -1;

    bool syntax_error = false;
    argv++;
    argc--;
    while (argc && argv[0][0] == '-') {
	if (argc >= 2 && strcmp(argv[0], "--msize") == 0) {
	    msize = atoi(argv[1]);
	    argc -= 2;
	    argv += 2;
	} else if (argc >= 2 && strcmp(argv[0], "--mfirst") == 0) {
	    mfirst = atoi(argv[1]);
	    argc -= 2;
	    argv += 2;
	} else if (argc >= 2 && strcmp(argv[0], "--key") == 0) {
	    collapse_key = atoi(argv[1]);
	    argc -= 2;
	    argv += 2;
	} else if (argc >= 2 && strcmp(argv[0], "--dbdir") == 0) {
	    OmSettings *params = new OmSettings;
	    params->set("backend", "auto");
	    params->set("auto_dir", argv[1]);
	    dbs.push_back(params);
	    argc -= 2;
	    argv += 2;
	} else if (strcmp(argv[0], "--stem") == 0) {
	    applystem = true;
	    argc--;
	    argv++;
	} else if (strcmp(argv[0], "--nostem") == 0) {
	    applystem = false;
	    argc--;
	    argv++;
	} else if (strcmp(argv[0], "--showmset") == 0) {
	    showmset = true;
	    argc--;
	    argv++;
	} else if (strcmp(argv[0], "--hidemset") == 0) {
	    showmset = false;
	    argc--;
	    argv++;
	} else if (strcmp(argv[0], "--matchall") == 0) {
	    default_op = OmQuery::OP_AND;
	    argc--;
	    argv++;
	} else if (strcmp(argv[0], "--rel") == 0) {
	    rset.add_document(atoi(argv[1]));
	    argc -= 2;
	    argv += 2;
	} else {
	    syntax_error = true;
	    break;
	}
    }
	
    if (syntax_error || argc < 1 || dbs.empty()) {
	cout << "Syntax: " << progname << " [OPTIONS] TERM ..." << endl <<
		"\t--msize <msize>\n" <<
		"\t--mfirst <first mitem to return>\n" <<
		"\t--key <key to collapse mset on>\n" <<
		"\t--dbdir DIRECTORY\n" <<
		"\t--rel DOCID\n" <<
		"\t--showmset (default)\n" <<
		"\t--hidemset\n" <<
		"\t--matchall\n" <<
		"\t--stem\n" <<
		"\t--nostem (default)\n";
	exit(1);
    }

    try {
        OmDatabase mydbs;

	vector<OmSettings *>::const_iterator p;
	for (p = dbs.begin(); p != dbs.end(); p++) {
	    mydbs.add_database(**p);
	    delete *p;
	}

	OmEnquire enquire(mydbs);

	OmStem stemmer("english");

	OmQuery query;
	stack<OmQuery> boolquery;
	// Parse query into OmQuery object
	bool boolean = false;
        for (char **p = argv; *p; p++) {
	    string term = *p;
	    if (term == "B") {
		boolean = true;
		continue;
	    } else if (term == "P") {
		if (boolquery.size() >= 1) {
		    query = OmQuery(OmQuery::OP_FILTER, query, boolquery.top());
		}
		boolean = false;
		continue;
	    } else {
		if (boolean) {
		    bool doop = false;
		    OmQuery::op boolop = default_op;
		    if (term == "OR") {
			boolop = OmQuery::OP_OR;
			doop = true;
		    } else if (term == "NOT") {
			boolop = OmQuery::OP_AND_NOT;
			doop = true;
		    } else if (term == "AND") {
			boolop = OmQuery::OP_AND;
			doop = true;
		    } else if (term == "XOR") {
			boolop = OmQuery::OP_XOR;
			doop = true;
		    } else if (term == "ANDMAYBE") {
			boolop = OmQuery::OP_AND_MAYBE;
			doop = true;
		    } else if (term == "ANDNOT") {
			boolop = OmQuery::OP_AND_NOT;
			doop = true;
		    }
// FIXME: NEAR (and PHRASE) take a window size and multiple terms
//		    else if (term == "NEAR") {
//			boolop = OmQuery::OP_NEAR;
//			doop = true;
//		    }
		    if (doop) {
			if (boolquery.size() < 2) {
			    cout << "Syntax error: boolean operands need 2 arguments\n(NB: query should be in reverse polish notation).\n";
			    exit(1);
			}
			OmQuery boolq_right(boolquery.top());
			boolquery.pop();
			OmQuery newtop(boolop, boolquery.top(), boolq_right);
			boolquery.pop();
			boolquery.push(newtop);
		    } else {
			if (applystem)
			    term = stemmer.stem_word(term);
			boolquery.push(OmQuery(term));
		    }
		} else {
		    if (applystem)
			term = stemmer.stem_word(term);
		    DebugMsg("oldquery: " << query.get_description() << endl);
		    query = OmQuery(default_op, query, term);
		    DebugMsg("newquery: " << query.get_description() << endl);
		}
	    }
        }
	if (boolean) {
	    if (boolquery.size() == 0) {
		cout << "Syntax error: Empty boolean query.\n";
		exit(1);
	    }
	    while (boolquery.size() > 1) {
		// implicit AND of remains of query
		OmQuery boolq_right(boolquery.top());
		boolquery.pop();
		OmQuery newtop(OmQuery::OP_AND, boolquery.top(), boolq_right);
		boolquery.pop();
		boolquery.push(newtop);
	    }
	    query = OmQuery(OmQuery::OP_FILTER, query, boolquery.top());
	}

	enquire.set_query(query);
	DebugMsg("Query is: " << query.get_description() << endl);

	OmSettings opts;
	if (collapse_key != -1)
	    opts.set("match_collapse_key", collapse_key);

	OmMSet mset = enquire.get_mset(mfirst, msize, &rset, &opts);
	
	if (showmset) {
	    vector<OmMSetItem>::const_iterator i;
	    for(i = mset.items.begin();
		i != mset.items.end();
		i++) {
		OmDocument doc(enquire.get_doc(*i));
		string p = doc.get_data().value;
		cout << i->did << ":[" << p << "] " << i->wt << endl << endl;
	    }
	    cout << endl;
	}
    }
    catch (OmError &e) {
	cout << e.get_msg() << endl;
    }
}
