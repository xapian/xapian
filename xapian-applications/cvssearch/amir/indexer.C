#include <om/om.h>

static void
lowercase_term(om_termname &term)
{
    om_termname::iterator i = term.begin();
    while(i != term.end()) {
        *i = tolower(*i);
        i++;
    }
}

int main(int argc, char *argv[]) {
  
  if(argc < 4) {
    cout << "usage: " << argv[0] <<
      " <path to database> <document data> <document terms>" << endl;
    exit(1);
  }
  
  try {
    // code which accesses Xapian

    OmSettings db_parameters;
    db_parameters.set("backend", "quartz");
    db_parameters.set("quartz_dir", argv[1]);
    OmWritableDatabase database(db_parameters); // open database (doesn't erase)

    OmDocumentContents newdocument;
    newdocument.data = string(argv[2]); // data associated with document (e.g., title, etc.)
    
    OmStem stemmer("english");

    // add document terms
    for(int i = 3; i < argc; i++) {
      om_termname term = argv[i];
      cout << term << " -> ";
      lowercase_term(term);
      term = stemmer.stem_word(term);
      cout << term << endl;
      newdocument.add_term_nopos(term);
    }

    // adding one document at a time is not so efficient though
    database.add_document(newdocument);

  }
  catch(OmError & error) {
    cout << "Exception: " << error.get_msg() << endl;
  } 
  
}
