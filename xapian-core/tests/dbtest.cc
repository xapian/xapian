#include <stdio.h>

#include "proto_database.h"
#include "da_database.h"
#include "da_record.h"

int main(int argc, char *argv[]) {
    ProtoDatabase database;
    PostList * postlist;
    TermList * termlist;
    termid tid;
    docid did;

    try {
	database.open("testdir", 0);
	tid = database.term_name_to_id("true");
	if(tid == 0) {
	    printf("Term not found\n");
	} else {
	    // posting list 122 141 142 174 ...
	    postlist = database.open_post_list(tid);
	    while(!postlist->at_end()) {
		docid did;
		weight wt;

		did = postlist->get_docid();
		wt = postlist->get_weight();
		printf("TermId: %d  DocId: %d  Weight: %f\n", tid, did, wt);
		if(did == 120) postlist->skip_to(144);
		else postlist->next();
	    }
	    delete postlist;
	}
	termlist = database.open_term_list(200);
	termlist = database.open_term_list(201);
	termlist = database.open_term_list(202);
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
}
