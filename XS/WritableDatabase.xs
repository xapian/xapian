MODULE = Search::Xapian		PACKAGE = Search::Xapian::WritableDatabase

PROTOTYPES: ENABLE

OmWritableDatabase *
new1(params)
    OmSettings * params
    CODE:
        try {
            RETVAL = new OmWritableDatabase(* params);
        }
        catch (const OmError &error) {
            croak( "Exception: %s", error.get_msg().c_str() );
        }
    OUTPUT:
        RETVAL

OmWritableDatabase *
new2(database)
    OmWritableDatabase * database
    CODE:
        RETVAL = new OmWritableDatabase(* database);
    OUTPUT:
        RETVAL

void
OmWritableDatabase::flush()

void
OmWritableDatabase::begin_transaction()

void
OmWritableDatabase::commit_transaction()

void
OmWritableDatabase::cancel_transaction()

om_docid
OmWritableDatabase::add_document(document)
    OmDocument *        document
    CODE:
        RETVAL = THIS->add_document(*document);
    OUTPUT:
        RETVAL

void
OmWritableDatabase::delete_document(om_docid did)

void
OmWritableDatabase::replace_document(did, document)
    om_docid            did
    OmDocument *        document
    CODE:
        THIS->replace_document(did,* document);

string
OmWritableDatabase::get_description()

void
OmWritableDatabase::DESTROY()