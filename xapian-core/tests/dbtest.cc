#include <stdio.h>

#include "proto_database.h"
#include "da_database.h"
//#include "match.h"

int main(int argc, char *argv[]) {
    DADatabase database;
    PostList * postlist;
    TermList * termlist;
    termid tid;
    docid did;

    try {
	database.open("testdir", 0);
	tid = database.term_name_to_id("abhor");
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
	did = 2510;
	termlist = database.open_term_list(did);
	printf("\nTermlist for document %d:\n", did);
	while(!termlist->at_end()) {
	    termid tid = termlist->get_termid();

	    printf("TermId: %d\n", tid);
	    termlist->next();
	}
	delete termlist;
	database.close();
    }
    catch (OmError e) {
	cout << e.get_msg() << endl;
    }
}
