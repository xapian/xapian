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

        if (argc < 2) {
	    cout << "Syntax: " << argv[0] << " TERM ..." << endl;
	    exit(1);
	}
       
        for (char **p = argv + 1; *p; p++) {
	    string term = *p;
	    term = stemmer.stem_word(term);
	    if (match.add_pterm(term)) {
	        printf("Added term \"%s\" ok\n", term.c_str());
	    } else {
	        printf("Failed to add term \"%s\"\n", term.c_str());
	    }
        }

        match.match();
	database.close();
    }
    catch (OmError e) {
	cout << e.get_msg() << endl;
    }
}
