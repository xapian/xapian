MODULE = Search::Xapian		PACKAGE = Search::Xapian::Database		

PROTOTYPES: ENABLE

OmDatabase *
new1(file)
    string * file
    CODE:
        OmDatabase * database = new OmDatabase(); 
        try {
            *database = OmAuto__open(* file);
            RETVAL = database;                                    
        }
        catch (const OmError &error) {
            croak( "Exception: %s", error.get_msg().c_str() );
        }
    OUTPUT:
        RETVAL

OmDatabase *
new2(database)
    OmDatabase * database
    CODE:
        RETVAL = new OmDatabase(* database);
    OUTPUT:
        RETVAL

void
OmDatabase::add_database(database)
    OmDatabase * database
    CODE:
        THIS->add_database(* database);

void
OmDatabase::reopen()

string
OmDatabase::get_description()

OmTermIterator *
OmDatabase::termlist_begin(om_docid did)
    CODE:
        RETVAL = new OmTermIterator();
        *RETVAL = THIS->termlist_begin(did);
    OUTPUT:
        RETVAL

OmTermIterator *
OmDatabase::termlist_end(om_docid did)
    CODE:
        RETVAL = new OmTermIterator();
        *RETVAL = THIS->termlist_end(did);
    OUTPUT:
        RETVAL

OmTermIterator *
OmDatabase::allterms_begin()
    CODE:
        RETVAL = new OmTermIterator();
        *RETVAL = THIS->allterms_begin();
    OUTPUT:
        RETVAL

OmTermIterator *
OmDatabase::allterms_end()
    CODE:
        RETVAL = new OmTermIterator();
        *RETVAL = THIS->allterms_begin();
    OUTPUT:
        RETVAL

om_doccount
OmDatabase::get_doccount()

om_doclength
OmDatabase::get_avlength()

om_doccount
OmDatabase::get_termfreq(om_termname tname)

bool
OmDatabase::term_exists(om_termname tname)

om_termcount
OmDatabase::get_collection_freq(om_termname tname)

om_doclength
OmDatabase::get_doclength(om_docid did)

void
OmDatabase::keep_alive()

OmDocument *
OmDatabase::get_document(om_docid did)
    CODE:
        RETVAL = new OmDocument();
        *RETVAL = THIS->get_document(did);
    OUTPUT:
        RETVAL

void
OmDatabase::DESTROY()