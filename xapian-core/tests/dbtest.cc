#include "proto_database.h"
#include "match.h"

int main(int argc, char *argv[]) {
    ProtoDatabase database;
    ProtoPostListIterator * postlist;

    try {
	database.open("testdir", 0);
	postlist = database.open_post_list(0);
	while(!postlist->at_end()) {
	    docid id;
	    id = postlist->get_docid();
	    printf("DocId: %d\n", id);

	    postlist->next();
	}
	postlist->close();
	database.close();
    }
    catch (OmError e) {
	cout << e.get_msg() << endl;
    }
}
