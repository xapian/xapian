/* idxtest.cc:
 *
 * ----START-LICENCE----
 * -----END-LICENCE-----
 */

#include "om.h"
#include <algorithm>
#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc < 4 || argc > 5) {
	printf("Syntax: %s dbdir term docid [termpos]\n", argv[0]);
	exit(1);
    }

    try {
#if 0
	SleepyDatabase db;
	db.open(argv[1], 0);

	termid tid = db.add_term(argv[2]);
	docid did = atoi(argv[3]);
	termpos tpos = 0;
	if(argc == 5) {
	    tpos = atoi(argv[4]);
	}

	db.add(tid, did, tpos);
#endif
    }
    catch (OmError e) {
	cout << e.get_msg() << endl;
    }
    catch (exception e) {
	cout << "Exception" << endl;
    }

    return 0;
}
