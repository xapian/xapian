#include <stdio.h>

#include "da_database.h"
#include "match.h"
#include "stem.h"

int main(int argc, char *argv[]) {
    try {
        DADatabase database;
	database.open("testdir", 0);
       
        Match match(&database);
       
        StemEn stemmer;

        if (argc >= 3 && strcmp(argv[1], "--msize") == 0) {
	    match.set_max_msize(atoi(argv[2]));
	    argc -= 2;
	    memmove(argv + 1, argv + 3, argc * sizeof(char*));
	}

        if (argc < 2) {
	    cout << "Syntax: " << argv[0] << " TERM ..." << endl;
	    exit(1);
	}

	bool boolean = false;
        for (char **p = argv + 1; *p; p++) {
	    string term = *p;
	    if(term == "B") { boolean = true;}
	    else if(term == "P") { boolean = false;}
	    else if(boolean) {
		if (term == "OR") {
		    if (match.add_bor()) {
			printf("Added boolean OR\n");
		    } else {
			printf("Failed to add boolean OR\n");
		    }
		} else if (term == "AND") {
		    if (match.add_band()) {
			printf("Added boolean AND\n");
		    } else {
			printf("Failed to add boolean AND\n");
		    }
		} else {
		    if(match.add_bterm(term)) { 
			printf("Added boolean term \"%s\" ok\n", term.c_str());
		    } else {
			printf("Failed to add boolean term \"%s\"\n",
			       term.c_str());
		    }
		}
	    } else {
		term = stemmer.stem_word(term);
		if (match.add_pterm(term)) {
		    printf("Added term \"%s\" ok\n", term.c_str());
		} else {
		    printf("Failed to add term \"%s\"\n", term.c_str());
		}
	    }
        }

        match.match();
	database.close();
    }
    catch (OmError e) {
	cout << e.get_msg() << endl;
    }
}
