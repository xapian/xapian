MODULE = Search::Xapian		PACKAGE = Search::Xapian::WritableDatabase

PROTOTYPES: ENABLE

WritableDatabase *
new1(file, opts)
    string	file
    int		opts
    CODE:
        RETVAL = new WritableDatabase();
        try {
            *RETVAL = Auto::open(file, opts);
        }
        catch (const Error &error) {
            croak( "Exception: %s", error.get_msg().c_str() );
        }
    OUTPUT:
        RETVAL

WritableDatabase *
new2(database)
    WritableDatabase *	database
    CODE:
        RETVAL = new WritableDatabase(*database);
    OUTPUT:
        RETVAL

void
WritableDatabase::flush()
   CODE:
	try {
            THIS->flush();
        }
        catch (const Error &error) {
            croak( "Exception: %s", error.get_msg().c_str() );
        }

void
WritableDatabase::begin_transaction()
   CODE:
	try {
            THIS->begin_transaction();
        }
        catch (const Error &error) {
            croak( "Exception: %s", error.get_msg().c_str() );
        }

void
WritableDatabase::commit_transaction()
   CODE:
	try {
            THIS->commit_transaction();
        }
        catch (const Error &error) {
            croak( "Exception: %s", error.get_msg().c_str() );
        }

void
WritableDatabase::cancel_transaction()
   CODE:
	try {
            THIS->cancel_transaction();
        }
        catch (const Error &error) {
            croak( "Exception: %s", error.get_msg().c_str() );
        }

docid
WritableDatabase::add_document(document)
    Document *	document
    CODE:
        try {
	    RETVAL = THIS->add_document(*document);
        }
        catch (const Error &error) {
            croak( "Exception: %s", error.get_msg().c_str() );
        }
    OUTPUT:
        RETVAL

void
WritableDatabase::delete_document(docid did)

void
WritableDatabase::replace_document(did, document)
    docid	did
    Document *	document
    CODE:
        THIS->replace_document(did, *document);

string
WritableDatabase::get_description()

void
WritableDatabase::DESTROY()
