#include "proto_database.h"
#include "match.h"

int main(int argc, char *argv[]) {
    ProtoDatabase database;

    try {
	database.open("testdir", 0);
	database.open_post_list(0);
	database.close();
    }
    catch (OmError e) {
	cout << e.get_msg() << endl;
    }
}
