#include <stdio.h>

//#define MULTIDB

#ifdef MULTIDB
#include "multi_database.h"
#endif

#include "da_database.h"
#include "match.h"
#include "stem.h"

int main(int argc, char *argv[]) {
    try {
#ifdef MULTIDB
	MultiDatabase database;
	database.open_subdatabase(new DADatabase, "testdir", 0);
#else
        DADatabase database;
	database.open("testdir", 0);
#endif
       
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
	    else {
		if (boolean) {
		    if (term == "OR") {
			if (match.add_bor()) {
			    printf("Added boolean OR\n");
			} else {
			    printf("Failed to add boolean OR\n");
			}
			continue;
		    } else if (term == "NOT") {
			if (match.add_bandnot()) {
			    printf("Added boolean ANDNOT\n");
			} else {
			    printf("Failed to add boolean ANDNOT\n");
			}
			continue;
		    } else if (term == "AND") {
			if (match.add_band()) {
			    printf("Added boolean AND\n");
			} else {
			    printf("Failed to add boolean AND\n");
			}
			continue;
		    } else if (term == "XOR") {
			if (match.add_bxor()) {
			    printf("Added boolean XOR\n");
			} else {
			    printf("Failed to add boolean XOR\n");
			}
			continue;
		    }
		}

		term = stemmer.stem_word(term);
		
		bool res;
		if (boolean) {
		    res = match.add_bterm(term);
		} else {
		    res = match.add_pterm(term);
		}
		
		if (res) {
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
