#include "proto_database.h"
#include "match.h"

int main(int argc, char *argv[]) {
    ProtoDatabase database;
    ProtoPostListIterator * postlist;

    try {
	database.open("testdir", 0);
	postlist = database.open_post_list(1);
	while(!postlist->at_end()) {
	    docid id;
	    id = postlist->get_docid();
	    printf("DocId: %d\n", id);
	    if(id == 2) postlist->skip_to(5);
	    else postlist->next();
	}
	delete postlist;
	database.close();
    }
    catch (OmError e) {
	cout << e.get_msg() << endl;
    }
}
