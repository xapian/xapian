/* dbtest.cc:
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

#include "omerror.h"
#include "database.h"
#include "dbpostlist.h"
#include "termlist.h"
#include "database_builder.h"

int main(int argc, char *argv[]) {
    DBPostList * postlist;
    TermList * termlist;
    docid did;

    try {
	IRDatabase * database;
#if 1
	DatabaseBuilderParams dbparams(OM_DBTYPE_INMEMORY, true);
	dbparams.paths.push_back("textfile");
	database = DatabaseBuilder::create(OM_DBTYPE_INMEMORY);
#endif
#if 0
	database.open_subdatabase(new SleepyDatabase(), "test_sleepy", 0);
	//database.open_subdatabase(new SleepyDatabase(), "test_sleepy2", 0);
	//database.open_subdatabase(new DADatabase(), "testdir", 0);
#endif
#if 0
	SleepyDatabase database;
	database.open("test_sleepy", 0);
#endif
#if 0
	DADatabase database;
	database.open("testdir", 0);
#endif

	termname tname = "thou";
	if(database->term_exists(tname) == 0) {
	    printf("Term not found\n");
	} else {
	    printf("tname is `%s'\n", tname.c_str());
	    // posting list 122 141 142 174 ...
	    postlist = database->open_post_list(tname, NULL);
	    printf("Termfreq: %d\n", postlist->get_termfreq());
	    postlist->next(0.0);
	    while(!postlist->at_end()) {
		docid did;
		weight wt;
		weight maxwt;

		did = postlist->get_docid();
		wt = postlist->get_weight();
		maxwt = postlist->get_maxweight();
		printf("Termname: %s  DocId: %d  Weight: %f  Maxweight: %f\n",
		       tname.c_str(), did, wt, maxwt);
		if(did == 120) postlist->skip_to(144, 0.0);
		else postlist->next(0.0);
	    }
	    delete postlist;
	}
	/*
	termlist = database->open_term_list(200);
	delete termlist;
	termlist = database->open_term_list(201);
	delete termlist;
	termlist = database->open_term_list(202);
	delete termlist;
	*/
	did = 1;
	printf("\nTermlist for document %d:\n", did);
	termlist = database->open_term_list(did);
	termlist->next();
	while(!termlist->at_end()) {
	    termname tname = termlist->get_termname();

	    cout << "Term `" << tname <<
		    "' wdf=" << termlist->get_wdf() <<
		    " termfreq=" << termlist->get_termfreq() << endl;
	    termlist->next();
	}
	delete termlist;
//	DARecord * rec = database.get_document(did);
//
//	delete rec;
	delete database;
    }
    catch (OmError e) {
	cout << e.get_msg() << endl;
    }
    catch (exception e) {
	cout << "Exception" << endl;
    }
}
