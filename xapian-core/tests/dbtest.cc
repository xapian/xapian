#include <stdio.h>

#include "proto_database.h"
//#include "match.h"

int main(int argc, char *argv[]) {
    ProtoDatabase database;
    PostListIterator * postlist;
    termid tid;

    try {
	database.open("testdir", 0);
	tid = database.term_name_to_id("2");
	postlist = database.open_post_list(tid);
	while(!postlist->at_end()) {
	    docid did;

	    did = postlist->get_docid();
	    printf("TermId: %d  DocId: %d\n", tid, did);
	    if(did == 2) postlist->skip_to(5);
	    else postlist->next();
	}
	delete postlist;
	database.close();
    }
    catch (OmError e) {
	cout << e.get_msg() << endl;
    }
}
