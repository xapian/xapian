/* msearch.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
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

#include <om/om.h>

#include <vector>
#include <stack>
#include <memory>

#include "getopt.h"

int
main(int argc, char *argv[])
{
    int msize = 10;
    int mfirst = 0;
    OmRSet rset;
    std::vector<OmSettings *> dbs;
    bool showmset = true;
    bool applystem = false;
    OmQuery::op default_op = OmQuery::OP_OR;
    int collapse_key = -1;

    bool syntax_error = false;

    struct option opts[] = {
	{"msize",		required_argument, 0, 'c'},
	{"mfirst",		required_argument, 0, 'f'},
	{"key",			required_argument, 0, 'k'},
	{"remote",		required_argument, 0, 'r'},
	{"dbdir",		required_argument, 0, 'd'},
	{"stem",		no_argument, 0, 's'},
	{"nostem",		no_argument, 0, 'S'},
	{"showmset",		no_argument, 0, 'm'},
	{"hidemset",		no_argument, 0, 'M'},
	{"matchall",		no_argument, 0, 'a'},
	{"rel",			required_argument, 0, 'R'},
	{NULL,			0, 0, 0}
    };

    int c;
    while ((c = getopt_long(argc, argv, "", opts, NULL)) != EOF) {
	switch (c) {
	    case 'c':
		msize = atoi(optarg);
		break;
	    case 'f':
		mfirst = atoi(optarg);
		break;
	    case 'k':
		collapse_key = atoi(optarg);
		break;
	    case 'r': {
		OmSettings *params = new OmSettings;
		params->set("backend", "remote");
		params->set("remote_type", "tcp");
		char *p = strchr(optarg, ':');
		if (p) {
		    *p = '\0';
		    params->set("remote_server", optarg);
		    params->set("remote_port", p + 1);
		    dbs.push_back(params);
		} else {
		    syntax_error = true;
		}
		break;
	    }
	    case 'd': {
		OmSettings *params = new OmSettings;
		params->set("backend", "auto");
		params->set("auto_dir", optarg);
		dbs.push_back(params);
		break;
	    }
	    case 's':
		applystem = true;
		break;
	    case 'S':
		applystem = false;
		break;
	    case 'm':
		showmset = true;
		break;
	    case 'M':
		showmset = false;
		break;
	    case 'a':
		default_op = OmQuery::OP_AND;
		break;
	    case 'R':
		rset.add_document(atoi(optarg));
		break;
	    default:
		syntax_error = true;
	}
    }
	
    if (syntax_error || dbs.empty() || argv[optind] == NULL) {
	std::cout << "Syntax: " << argv[0] << " [OPTIONS] TERM...\n" <<
		"\t--msize <msize>\n" <<
		"\t--mfirst <first mitem to return>\n" <<
		"\t--key <key to collapse mset on>\n" <<
		"\t--dbdir DIRECTORY\n" <<
		"\t--remote SERVER:PORT\n" <<
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

	std::vector<OmSettings *>::const_iterator p;
	for (p = dbs.begin(); p != dbs.end(); p++) {
	    mydbs.add_database(**p);
	    delete *p;
	}

	OmEnquire enquire(mydbs);

	OmStem stemmer("english");

	OmQuery query;
	bool query_defined = false;

	std::stack<OmQuery> boolquery;
	// Parse query into OmQuery object
	bool boolean = false;
        for (char **p = argv + optind; *p; p++) {
	    std::string term = *p;
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
			    std::cout << "Syntax error: boolean operands need 2 arguments\n(NB: query should be in reverse polish notation).\n";
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
		    if (query_defined) {
		        query = OmQuery(default_op, query, term);
		    } else {
		        query = OmQuery(term);
			query_defined = true;
		    }
		}
	    }
        }
	if (boolean) {
	    if (boolquery.size() == 0) {
		std::cout << "Syntax error: Empty boolean query.\n";
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

	OmSettings opts;
	if (collapse_key != -1)
	    opts.set("match_collapse_key", collapse_key);

	OmMSet mset = enquire.get_mset(mfirst, msize, &rset, &opts);
	
	if (showmset) {
	    for (OmMSetIterator i = mset.begin(); i != mset.end(); i++) {
		OmDocument doc = i.get_document();
		std::string p = doc.get_data();
		std::cout << *i << ":[" << p << "] " << i.get_weight()
			<< std::endl << std::endl;
	    }
	    std::cout << std::endl;
	}
    }
    catch (const OmError &e) {
	std::cout << e.get_msg() << std::endl;
    }
}
