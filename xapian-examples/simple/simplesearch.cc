// simplesearch.cc

#include <stdio.h>
#include <om/om.h>

int main(int argc, char *argv[])
{
    // Simplest possible options parsing: we just require two or more
    // parameters.
    if(argc < 3) {
	printf("usage: %s <path to database> <search terms>\n", argv[0]);
    }
    
    // Make the database group
    OmDatabase databases;
    vector<string> parameters;
    parameters.push_back(argv[1]);
    databases.add_database("sleepycat", parameters);

    // Start an enquire session
    OmEnquire enquire(databases);

    // Prepare the query terms
    vector<om_termname> queryterms;
    for(int optpos = 2; optpos < argc; optpos++) {
	queryterms.push_back(argv[optpos]);
    }

    // 
    OmQuery query(OM_MOP_OR, queryterms.begin(), queryterms.end());
}
