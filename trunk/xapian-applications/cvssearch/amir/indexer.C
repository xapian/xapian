#include <xapian.h>

static void
lowercase_term(string &term)
{
    string::iterator i = term.begin();
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

    // open database (doesn't erase)
    Xapian::WritableDatabase database(Xapian::Quartz::open(argv[1]));

    Xapian::Document newdocument;
    newdocument.data = string(argv[2]); // data associated with document (e.g., title, etc.)
    
    Xapian::Stem stemmer("english");

    // add document terms
    for(int i = 3; i < argc; i++) {
      string term = argv[i];
      cout << term << " -> ";
      lowercase_term(term);
      term = stemmer.stem_word(term);
      cout << term << endl;
      newdocument.add_term_nopos(term);
    }

    // adding one document at a time is not so efficient though
    database.add_document(newdocument);

  }
  catch(const Xapian::Error & error) {
    cout << "Exception: " << error.get_msg() << endl;
  } 
  
}
