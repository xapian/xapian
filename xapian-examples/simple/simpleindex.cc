// simplesearch.cc

#include <om/om.h>

int main(int argc, char *argv[])
{
    // Simplest possible options parsing: we just require three or more
    // parameters.
    if(argc < 4) {
	cout << "usage: " << argv[0] <<
		" <path to database> <document data> <document terms>" << endl;
	exit(1);
    }
    
    // Catch any OmError exceptions thrown
    try {
	// Make the database
	vector<string> parameters;
	parameters.push_back(argv[1]);
	OmWritableDatabase database("sleepycat", parameters);

	// Make the document
	OmDocumentContents newdocument;

	// Put the data in the document
	newdocument.data = string(argv[2]);

	// Put the terms into the document
	for(int i = 3; i < argc; i++) {
	    newdocument.add_posting(argv[i], i - 2);
	}

	// Add the document to the database
	database.add_document(newdocument);
    }
    catch(OmError &error) {
	cout << "Exception: "  << error.get_msg() << endl;
    }
}
