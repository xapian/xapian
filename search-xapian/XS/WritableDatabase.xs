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
WritableDatabase::delete_document(did)
    docid	did
    CODE:
        try {
	    THIS->delete_document(did);
        }
        catch (const Error &error) {
            croak( "Exception: %s", error.get_msg().c_str() );
        }

void
WritableDatabase::replace_document(did, document)
    docid	did
    Document *	document
    CODE:
        try {
	    THIS->replace_document(did, *document);
        }
        catch (const Error &error) {
            croak( "Exception: %s", error.get_msg().c_str() );
        }

void
WritableDatabase::reopen()
    CODE:
	try {
            THIS->reopen();
        }
        catch (const Error &error) {
            croak( "Exception: %s", error.get_msg().c_str() );
        }

string
WritableDatabase::get_description()
    CODE:
	try {
            RETVAL = THIS->get_description();
        }
        catch (const Error &error) {
            croak( "Exception: %s", error.get_msg().c_str() );
        }
    OUTPUT:
        RETVAL

TermIterator *
WritableDatabase::termlist_begin(did)
    docid	did
    CODE:
        RETVAL = new TermIterator();
	try {
	    *RETVAL = THIS->termlist_begin(did);
        }
        catch (const Error &error) {
            croak( "Exception: %s", error.get_msg().c_str() );
        }
    OUTPUT:
        RETVAL

TermIterator *
WritableDatabase::termlist_end(did)
    docid	did
    CODE:
        RETVAL = new TermIterator();
	try {
	    *RETVAL = THIS->termlist_end(did);
        }
        catch (const Error &error) {
            croak( "Exception: %s", error.get_msg().c_str() );
        }
    OUTPUT:
        RETVAL

TermIterator *
WritableDatabase::allterms_begin()
    CODE:
        RETVAL = new TermIterator();
	try {
	    *RETVAL = THIS->allterms_begin();
        }
        catch (const Error &error) {
            croak( "Exception: %s", error.get_msg().c_str() );
        }
    OUTPUT:
        RETVAL

TermIterator *
WritableDatabase::allterms_end()
    CODE:
        RETVAL = new TermIterator();
	try {
	    *RETVAL = THIS->allterms_end();
        }
        catch (const Error &error) {
            croak( "Exception: %s", error.get_msg().c_str() );
        }
    OUTPUT:
        RETVAL

PostingIterator *
WritableDatabase::postlist_begin(term)
    string	term
    CODE:
        RETVAL = new PostingIterator();
	try {
	    *RETVAL = THIS->postlist_begin(term);
        }
        catch (const Error &error) {
            croak( "Exception: %s", error.get_msg().c_str() );
        }
    OUTPUT:
        RETVAL

PostingIterator *
WritableDatabase::postlist_end(term)
    string	term
    CODE:
        RETVAL = new PostingIterator();
	try {
	    *RETVAL = THIS->postlist_end(term);
        }
        catch (const Error &error) {
            croak( "Exception: %s", error.get_msg().c_str() );
        }
    OUTPUT:
        RETVAL

doccount
WritableDatabase::get_doccount()
    CODE:
	try {
            RETVAL = THIS->get_doccount();
        }
        catch (const Error &error) {
            croak( "Exception: %s", error.get_msg().c_str() );
        }
    OUTPUT:
        RETVAL

doclength
WritableDatabase::get_avlength()
    CODE:
	try {
            RETVAL = THIS->get_avlength();
        }
        catch (const Error &error) {
            croak( "Exception: %s", error.get_msg().c_str() );
        }
    OUTPUT:
        RETVAL

doccount
WritableDatabase::get_termfreq(tname)
    string	tname
    CODE:
	try {
            RETVAL = THIS->get_termfreq(tname);
        }
        catch (const Error &error) {
            croak( "Exception: %s", error.get_msg().c_str() );
        }
    OUTPUT:
        RETVAL

bool
WritableDatabase::term_exists(tname)
    string	tname
    CODE:
	try {
	    RETVAL = THIS->term_exists(tname);
        }
        catch (const Error &error) {
            croak( "Exception: %s", error.get_msg().c_str() );
        }
    OUTPUT:
        RETVAL

termcount
WritableDatabase::get_collection_freq(tname)
    string	tname
    CODE:
	try {
	    RETVAL = THIS->get_collection_freq(tname);
        }
        catch (const Error &error) {
            croak( "Exception: %s", error.get_msg().c_str() );
        }
    OUTPUT:
        RETVAL

doclength
WritableDatabase::get_doclength(did)
    docid	did
    CODE:
	try {
	    RETVAL = THIS->get_doclength(did);
        }
        catch (const Error &error) {
            croak( "Exception: %s", error.get_msg().c_str() );
        }
    OUTPUT:
        RETVAL

void
WritableDatabase::keep_alive()
    CODE:
	try {
	    THIS->keep_alive();
        }
        catch (const Error &error) {
            croak( "Exception: %s", error.get_msg().c_str() );
        }

Document *
WritableDatabase::get_document(docid did)
    CODE:
        RETVAL = new Document();
	try {
	    *RETVAL = THIS->get_document(did);
        }
        catch (const Error &error) {
            croak( "Exception: %s", error.get_msg().c_str() );
        }
    OUTPUT:
        RETVAL

void
WritableDatabase::DESTROY()
