/* matchtest.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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

#include "om.h"
#include "stemmer.h"

#include <vector>
#include <stack>

int
main(int argc, char *argv[])
{
    int msize = 10;
    int mfirst = 0;
    const char *progname = argv[0];
    OmRSet rset;
    vector<vector<string> > dbargs;
    vector<string> dbtypes;
    bool showmset = false;
    om_queryop default_op = OM_MOP_OR;
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
	} else if (argc >= 2 && strcmp(argv[0], "--da") == 0) {
	    vector<string> args;
	    args.push_back(argv[1]);
	    dbargs.push_back(args);
	    dbtypes.push_back("da_flimsy");
	    argc -= 2;
	    argv += 2;
	} else if (argc >= 2 && strcmp(argv[0], "--im") == 0) {
	    vector<string> args;
	    args.push_back(argv[1]);
	    dbargs.push_back(args);
	    dbtypes.push_back("inmemory");
	    argc -= 2;
	    argv += 2;
	} else if (strcmp(argv[0], "--showmset") == 0) {
	    showmset = true;
	    argc--;
	    argv++;
	} else if (strcmp(argv[0], "--matchall") == 0) {
	    default_op = OM_MOP_AND;
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
	
    if (syntax_error || argc < 1 || !dbtypes.size()) {
	cout << "Syntax: " << progname << " [OPTIONS] TERM ..." << endl <<
		"\t--msize <msize>\n" <<
		"\t--mfirst <first mitem to return>\n" <<
		"\t--key <key to collapse mset on>\n" <<
		"\t--da DBDIRECTORY\n" <<
		"\t--im INMEMORY\n" <<
		"\t--rel DOCID\n" <<
		"\t--multidb\n" <<
		"\t--showmset\n" <<
		"\t--matchall\n";
	exit(1);
    }

    try {
        OmEnquire enquire;

	vector<string>::const_iterator p;
	vector<vector<string> >::const_iterator q;
	for(p = dbtypes.begin(), q = dbargs.begin();
	    p != dbtypes.end();
	    p++, q++) {

	    enquire.add_database(*p, *q);
	}

	Stemmer * stemmer = StemmerBuilder::create(STEMLANG_ENGLISH);

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
		    query = OmQuery(OM_MOP_FILTER, query, boolquery.top());
		}
		boolean = false;
		continue;
	    } else {
		if (boolean) {
		    bool doop = false;
		    om_queryop boolop = default_op;
		    if (term == "OR") {
			boolop = OM_MOP_OR;
			doop = true;
		    } else if (term == "NOT") {
			boolop = OM_MOP_AND_NOT;
			doop = true;
		    } else if (term == "AND") {
			boolop = OM_MOP_AND;
			doop = true;
		    } else if (term == "XOR") {
			boolop = OM_MOP_XOR;
			doop = true;
		    } else if (term == "ANDMAYBE") {
			boolop = OM_MOP_AND_MAYBE;
			doop = true;
		    } else if (term == "ANDNOT") {
			boolop = OM_MOP_AND_NOT;
			doop = true;
		    }
		    if(doop) {
			Assert(boolquery.size() >= 2);
			OmQuery boolq_right(boolquery.top());
			boolquery.pop();
			OmQuery newtop(boolop, boolquery.top(), boolq_right);
			boolquery.pop();
			boolquery.push(newtop);
		    } else {
			boolquery.push(OmQuery(stemmer->stem_word(term)));
		    }
		} else {
		    term = stemmer->stem_word(term);
		    DebugMsg("oldquery: " << query.get_description() << endl);
		    query = OmQuery(default_op, query, term);
		    DebugMsg("newquery: " << query.get_description() << endl);
		}
	    }
        }
	if(boolean) {
	    Assert(boolquery.size() == 1);
	    query = OmQuery(OM_MOP_FILTER, query, boolquery.top());
	}

	enquire.set_query(query);
	DebugMsg("Query is: " << query.get_description() << endl);

	OmMatchOptions opts;
	if(collapse_key != -1) opts.set_collapse_key(collapse_key);

	OmMSet mset = enquire.get_mset(mfirst, msize, &rset, &opts);
	
	if (showmset) {
	    vector<OmMSetItem>::const_iterator i;
	    for(i = mset.items.begin();
		i != mset.items.end();
		i++) {
		string p = enquire.get_doc_data(*i).value;
		cout << i->did << ":[" << p << "] " << i->wt << endl << endl;
	    }
	    cout << endl;
	}
    }
    catch (OmError &e) {
	cout << e.get_msg() << endl;
    }
}
