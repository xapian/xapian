#include <om/om.h>

int main(int argc, char *argv[]) {
     if(argc < 3) {
        cout << "usage: " << argv[0] <<
                " <path to database> <search terms>" << endl;
        exit(1);
    }

     try {
       // code which accesses Omsee
       OmDatabase databases;
       OmSettings db_parameters;
       db_parameters.set("backend", "quartz");
       db_parameters.set("quartz_dir", argv[1]);
       databases.add_database(db_parameters); // can search multiple databases at once

       // start an enquire session
       OmEnquire enquire(databases);

       vector<om_termname> queryterms;
       for (int optpos = 2; optpos < argc; optpos++) {
	 queryterms.push_back(argv[optpos]);
       }

       // MOP stands for "Match OPeration"
       OmQuery query(OmQuery::OP_AND, queryterms.begin(), queryterms.end());

       cout << "Performing query `" <<
         query.get_description() << "'" << endl;

       enquire.set_query(query); // copies query object

       OmMSet matches = enquire.get_mset(0, 10); // get top 10 matches

       cout << matches.items.size() << " results found" << endl;

       vector<OmMSetItem>::const_iterator i;
       for (i = matches.items.begin(); i != matches.items.end(); i++) {
	 cout << "Document ID " << i->did << "\t";
	 cout << matches.convert_to_percent(*i) << "% ";
	 OmDocument doc = enquire.get_doc(*i);
	 cout << "[" << doc.get_data().value << "]" << endl; // data associated with doc
       }

     }
     catch(OmError & error) {
       cout << "Exception: " << error.get_msg() << endl;
     }
     
}
