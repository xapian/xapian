#include <stdio.h>

#include "proto_database.h"
#include "match.h"

int main(int argc, char *argv[]) {
    try {
        ProtoDatabase database;
	database.open("testdir", 0);
       
        Match match(&database);

        if (argc < 2) {
	    cout << "Syntax: " << argv[0] << " TERM ..." << endl;
	    exit(1);
	}
       
        for (char **p = argv + 1; *p; p++) {
	    if (match.add_pterm(*p)) {
	        printf("Added term \"%s\" ok\n", *p);
	    } else {
	        printf("Failed to add term \"%s\"\n", *p);
	    }
        }

        match.match();
	database.close();
    }
    catch (OmError e) {
	cout << e.get_msg() << endl;
    }
}
