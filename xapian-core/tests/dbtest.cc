#include <stdio.h>

#include "proto_database.h"
#include "da_database.h"
//#include "match.h"

int main(int argc, char *argv[]) {
    DADatabase database;
    PostList * postlist;
    termid tid;

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
	    printf("TermId: %d  DocId: %d  Weight: %d\n", tid, did, wt);
	    if(did == 120) postlist->skip_to(144);
	    else postlist->next();
	}
	delete postlist;
	database.close();
    }
    catch (OmError e) {
	cout << e.get_msg() << endl;
    }
}
