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
     if(argc < 3) {
        cout << "usage: " << argv[0] <<
                " <path to database> <search terms>" << endl;
        exit(1);
    }

     try {
       // code which accesses Xapian
       Xapian::Database databases;
       // can search multiple databases at once
       databases.add_database(Xapian::Quartz::open(argv[1]));

       // start an enquire session
       Xapian::Enquire enquire(databases);

       vector<string> queryterms;
       
       Xapian::Stem stemmer("english");

       for (int optpos = 2; optpos < argc; optpos++) {

	 string term = argv[optpos];
	 cout << term << " -> ";
	 lowercase_term(term);
	 term = stemmer.stem_word(term);
	 cout << term << endl;
	 
	 queryterms.push_back(term);
       }

       Xapian::Query query(Xapian::Query::OP_AND, queryterms.begin(), queryterms.end());

       cout << "Performing query `" <<
         query.get_description() << "'" << endl;

       enquire.set_query(query); // copies query object

       Xapian::MSet matches = enquire.get_mset(0, 10); // get top 10 matches

       cout << matches.items.size() << " results found" << endl;

       Xapian::MSetIterator i; 
       for (i = matches.items.begin(); i != matches.items.end(); i++) {
	 cout << "Document ID " << i->did << "\t";
	 cout << matches.convert_to_percent(*i) << "% ";
	 Xapian::Document doc = enquire.get_doc(*i);
	 cout << "[" << doc.get_data().value << "]" << endl; // data associated with doc
       }

     }
     catch(const Xapian::Error & error) {
       cout << "Exception: " << error.get_msg() << endl;
     }
     
}
