#include <stdio.h>

#include "omerror.h"
#include "database.h"
#include "postlist.h"
#include "termlist.h"
#include "database_factory.h"

int main(int argc, char *argv[]) {
    DBPostList * postlist;
    TermList * termlist;
    docid did;

    try {
	DatabaseFactory dbfactory;
	IRDatabase * database;
#if 1
	IRSingleDatabase *tmp = dbfactory.make(OM_DBTYPE_TEXTFILE);
	tmp->open("textfile", 1);
	database = tmp;
#endif
#if 0
	MultiDatabase database;
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
	database->close();
    }
    catch (OmError e) {
	cout << e.get_msg() << endl;
    }
    catch (exception e) {
	cout << "Exception" << endl;
    }
}
