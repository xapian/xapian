#include "sleepy_database.h"

int main(int argc, char *argv[]) {
    if (argc != 4) {
	printf("Syntax: %s dbdir term docid\n", argv[0]);
	exit(1);
    }

    try {
	SleepyDatabase db;
	db.open(argv[1], 0);

	termid tid = db.add_term(argv[2]);
	docid did = atoi(argv[3]);

	db.add(tid, did);
    }
    catch (OmError e) {
	cout << e.get_msg() << endl;
    }
    catch (exception e) {
	cout << "Exception" << endl;
    }

    return 0;
}
