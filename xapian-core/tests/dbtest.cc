#include <stdio.h>

#include "database.h"
#include "sleepy_database.h"
#include "da_database.h"
#include "multi_database.h"
#include "textfile_database.h"

int main(int argc, char *argv[]) {
    DBPostList * postlist;
    TermList * termlist;
    termid tid;
    docid did;

    try {
#if 1
	TextfileDatabase database;
	database.open("textfile", 1);
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

	tid = database.term_name_to_id("thou");
	printf("tid is %d\n", tid);
	if(tid == 0) {
	    printf("Term not found\n");
	} else {
	    termname tname = database.term_id_to_name(tid);
	    printf("tname is `%s'\n", tname.c_str());
	    // posting list 122 141 142 174 ...
	    postlist = database.open_post_list(tid, NULL);
	    printf("Termfreq: %d\n", postlist->get_termfreq());
	    postlist->next(0.0);
	    while(!postlist->at_end()) {
		docid did;
		weight wt;
		weight maxwt;

		did = postlist->get_docid();
		wt = postlist->get_weight();
		maxwt = postlist->get_maxweight();
		printf("TermId: %d  DocId: %d  Weight: %f  Maxweight: %f\n",
		       tid, did, wt, maxwt);
		if(did == 120) postlist->skip_to(144, 0.0);
		else postlist->next(0.0);
	    }
	    delete postlist;
	}
	/*
	termlist = database.open_term_list(200);
	delete termlist;
	termlist = database.open_term_list(201);
	delete termlist;
	termlist = database.open_term_list(202);
	delete termlist;
	*/
	did = 1;
	printf("\nTermlist for document %d:\n", did);
	termlist = database.open_term_list(did);
	termlist->next();
	while(!termlist->at_end()) {
	    termid tid = termlist->get_termid();

	    printf("Term (Id %d) `%s' wdf=%d termfreq=%d\n", tid,
		   database.term_id_to_name(tid).c_str(),
		   termlist->get_wdf(),
		   termlist->get_termfreq());
	    termlist->next();
	}
	delete termlist;
//	DARecord * rec = database.get_document(did);
//
//	delete rec;
	database.close();
    }
    catch (OmError e) {
	cout << e.get_msg() << endl;
    }
    catch (exception e) {
	cout << "Exception" << endl;
    }
}
