/* matchtest.cc
 *
 * ----START-LICENCE----
 * Copyright 1999 Dialog Corporation
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

#include <list>

int
main(int argc, char *argv[])
{
    int msize = 10;
    int mfirst = 0;
    const char *progname = argv[0];
    list<docid> reldocs;
    list<string> dbnames;
    list<om_database_type> dbtypes;
    bool multidb = false;
    bool showmset = false;
    matchop default_op = MOP_OR;

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
	} else if (argc >= 2 && strcmp(argv[0], "--db") == 0) {
	    dbnames.push_back(argv[1]);
	    dbtypes.push_back(OM_DBTYPE_DA);
	    argc -= 2;
	    argv += 2;
	} else if (argc >= 2 && strcmp(argv[0], "--im") == 0) {
	    dbnames.push_back(argv[1]);
	    dbtypes.push_back(OM_DBTYPE_INMEMORY);
	    argc -= 2;
	    argv += 2;
	} else if (strcmp(argv[0], "--multidb") == 0) {
	    multidb = true;
	    argc--;
	    argv++;
	} else if (strcmp(argv[0], "--showmset") == 0) {
	    showmset = true;
	    argc--;
	    argv++;
	} else if (strcmp(argv[0], "--matchall") == 0) {
	    default_op = MOP_AND;
	    argc--;
	    argv++;
	} else if (strcmp(argv[0], "--rel") == 0) {
	    reldocs.push_back(atoi(argv[1]));
	    argc -= 2;
	    argv += 2;
	} else {
	    syntax_error = true;
	    break;
	}
    }
	
    if (syntax_error || argc < 1) {
	cout << "Syntax: " << progname << " TERM ..." << endl;
	cout << "\t--msize <msize>\n";
	cout << "\t--mfirst <first mitem to return>\n";
	cout << "\t--db DBDIRECTORY\n";
	cout << "\t--im INMEMORY\n";
	cout << "\t--rel DOCID\n";
	cout << "\t--multidb\n";
	cout << "\t--showmset\n";
	cout << "\t--matchall\n";
	exit(1);
    }

    if(!dbnames.size()) {
	dbnames.push_back("testdir");
	dbtypes.push_back(OM_DBTYPE_DA);
    }
    
    try {
	IRDatabase *database;

	DatabaseBuilderParams dbparams;
	if (multidb || dbnames.size() > 1) {
	    dbparams.type = OM_DBTYPE_MULTI;

	    list<string>::const_iterator p;
	    list<om_database_type>::const_iterator q;
	    for(p = dbnames.begin(), q = dbtypes.begin();
		p != dbnames.end();
		p++, q++) {
		DatabaseBuilderParams subparams(*q);
		subparams.paths.push_back(*p);
		dbparams.subdbs.push_back(subparams);
	    }
	} else {
	    dbparams.type = *(dbtypes.begin());
	    dbparams.paths.push_back(*(dbnames.begin()));
	}
	database = DatabaseBuilder::create(dbparams);
       
	RSet rset(database);
	list<docid>::const_iterator i;
	for(i = reldocs.begin(); i != reldocs.end(); i++) {
	    rset.add_document(*i);
	}

        Match match(database);
	match.set_rset(&rset);
       
        StemEn stemmer;

	bool boolean = false;
	vector<termname> prob_terms;
        for (char **p = argv; *p; p++) {
	    string term = *p;
	    if (term == "B") {
		if (!prob_terms.empty()) {
		    match.add_oplist(default_op, prob_terms);
		    prob_terms.clear();
		}
		boolean = true;
		continue;
	    } else if (term == "P") {
		boolean = false;		
		continue;
	    } else {
		if (boolean) {
		    if (term == "OR") {
			if (match.add_op(MOP_OR)) {
			    printf("Added boolean OR\n");
			} else {
			    printf("Failed to add boolean OR\n");
			}
			continue;
		    } else if (term == "NOT") {
			if (match.add_op(MOP_AND_NOT)) {
			    printf("Added boolean ANDNOT\n");
			} else {
			    printf("Failed to add boolean ANDNOT\n");
			}
			continue;
		    } else if (term == "AND") {
			if (match.add_op(MOP_AND)) {
			    printf("Added boolean AND\n");
			} else {
			    printf("Failed to add boolean AND\n");
			}
			continue;
		    } else if (term == "XOR") {
			if (match.add_op(MOP_XOR)) {
			    printf("Added boolean XOR\n");
			} else {
			    printf("Failed to add boolean XOR\n");
			}
			continue;
		    } else if (term == "ANDMAYBE") {
			if (match.add_op(MOP_AND_MAYBE)) {
			    printf("Added boolean ANDMAYBE\n");
			} else {
			    printf("Failed to add boolean ANDMAYBE\n");
			}
			continue;
		    } else if (term == "ANDNOT") {
			if (match.add_op(MOP_AND_NOT)) {
			    printf("Added boolean ANDNOT\n");
			} else {
			    printf("Failed to add boolean ANDNOT\n");
			}
			continue;
		    }
		}

		term = stemmer.stem_word(term);

		if (boolean) {
		    match.add_term(term);
		} else {
		    prob_terms.push_back(term);
		}
	    }
        }

	if (!prob_terms.empty()) {
	    match.add_oplist(default_op, prob_terms);
	    prob_terms.clear();
	}

	vector<MSetItem> mset;
        match.match(mfirst, msize, mset);
	
	if (showmset) {
	    vector<MSetItem>::const_iterator i;
	    for(i = mset.begin();
		i != mset.end();
		i++) {
		docid did = i->did;
		IRDocument *doc = database->open_document(did);
		IRData data = doc->get_data();
		string p = data.value;
		cout << did << ":[" << p << "] " << i->wt << endl << endl;
	    }
	    cout << endl;
	}
	delete database;
    }
    catch (OmError e) {
	cout << e.get_msg() << endl;
    }
}
