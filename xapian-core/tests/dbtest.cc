#include <stdio.h>

#include "sleepy_database.h"
#include "da_database.h"
#include "multi_database.h"

int main(int argc, char *argv[]) {
    MultiDatabase database;
    PostList * postlist;
    TermList * termlist;
    termid tid;
    docid did;

    try {
	database.open_subdatabase(new DADatabase(), "testdir", 0);
	database.open_subdatabase(new SleepyDatabase(), "test_sleepy", 0);

	tid = database.term_name_to_id("thou");
	printf("tid is %d\n", tid);
	if(tid == 0) {
	    printf("Term not found\n");
	} else {
	    termname tname = database.term_id_to_name(tid);
	    printf("tname is `%s'\n", tname.c_str());
	    // posting list 122 141 142 174 ...
	    postlist = database.open_post_list(tid);
	    postlist->next(0.0);
	    while(!postlist->at_end()) {
		docid did;
		weight wt;

		did = postlist->get_docid();
		wt = postlist->get_weight();
		printf("TermId: %d  DocId: %d  Weight: %f\n", tid, did, wt);
		if(did == 120) postlist->skip_to(144, 0.0);
		else postlist->next(0.0);
	    }
	    delete postlist;
	}
	termlist = database.open_term_list(200);
	delete termlist;
	termlist = database.open_term_list(201);
	delete termlist;
	termlist = database.open_term_list(202);
	delete termlist;
	did = 111;
	termlist = database.open_term_list(did);
	printf("\nTermlist for document %d:\n", did);
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
