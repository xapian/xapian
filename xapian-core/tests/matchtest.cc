#include <stdio.h>

#include "proto_database.h"
#include "match.h"

int main(int argc, char *argv[]) {
    try {
        ProtoDatabase database;
	database.open("testdir", 0);
       
        Match match(&database);
        if (match.add_pterm("2")) {
	    printf("Added term ok\n");
	} else {
	    printf("Failed to add term\n");
        }
        match.match();
	database.close();
    }
    catch (OmError e) {
	cout << e.get_msg() << endl;
    }
}
