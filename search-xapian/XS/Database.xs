MODULE = Search::Xapian		PACKAGE = Search::Xapian::Database

PROTOTYPES: ENABLE

Database *
new1(file)
    string	file
    CODE:
	try {
	    RETVAL = new Database(file);
	} catch (...) {
	    handle_exception();
	}
    OUTPUT:
	RETVAL

Database *
new2(database)
    Database *	database
    CODE:
	try {
	    RETVAL = new Database(*database);
	} catch (...) {
	    handle_exception();
	}
    OUTPUT:
	RETVAL

void
Database::add_database(database)
    Database *	database
    CODE:
	try {
	    THIS->add_database(*database);
	} catch (...) {
	    handle_exception();
	}

void
Database::reopen()
    CODE:
	try {
	    THIS->reopen();
	} catch (...) {
	    handle_exception();
	}

string
Database::get_description()
    CODE:
	try {
	    RETVAL = THIS->get_description();
	} catch (...) {
	    handle_exception();
	}
    OUTPUT:
	RETVAL

TermIterator *
Database::termlist_begin(did)
    docid	did
    CODE:
	try {
	    RETVAL = new TermIterator(THIS->termlist_begin(did));
	} catch (...) {
	    handle_exception();
	}
    OUTPUT:
	RETVAL

TermIterator *
Database::termlist_end(did)
    docid	did
    CODE:
	try {
	    RETVAL = new TermIterator(THIS->termlist_end(did));
	} catch (...) {
	    handle_exception();
	}
    OUTPUT:
	RETVAL

PositionIterator *
Database::positionlist_begin(did, term)
    docid	did
    string	term
    CODE:
	try {
	    RETVAL = new PositionIterator(THIS->positionlist_begin(did, term));
	} catch (...) {
	    handle_exception();
	}
    OUTPUT:
	RETVAL

PositionIterator *
Database::positionlist_end(did, term)
    docid	did
    string	term
    CODE:
	try {
	    RETVAL = new PositionIterator(THIS->positionlist_end(did, term));
	} catch (...) {
	    handle_exception();
	}
    OUTPUT:
	RETVAL

TermIterator *
Database::allterms_begin(prefix = "")
    string prefix
    CODE:
	try {
	    RETVAL = new TermIterator(THIS->allterms_begin(prefix));
	} catch (...) {
	    handle_exception();
	}
    OUTPUT:
	RETVAL

TermIterator *
Database::allterms_end(prefix = "")
    string prefix
    CODE:
	try {
	    RETVAL = new TermIterator(THIS->allterms_end(prefix));
	} catch (...) {
	    handle_exception();
	}
    OUTPUT:
	RETVAL

PostingIterator *
Database::postlist_begin(term)
    string	term
    CODE:
	try {
	    RETVAL = new PostingIterator(THIS->postlist_begin(term));
	} catch (...) {
	    handle_exception();
	}
    OUTPUT:
	RETVAL

PostingIterator *
Database::postlist_end(term)
    string	term
    CODE:
	try {
	    RETVAL = new PostingIterator(THIS->postlist_end(term));
	} catch (...) {
	    handle_exception();
	}
    OUTPUT:
	RETVAL

doccount
Database::get_doccount()
    CODE:
	try {
	    RETVAL = THIS->get_doccount();
	} catch (...) {
	    handle_exception();
	}
    OUTPUT:
	RETVAL

docid
Database::get_lastdocid()
    CODE:
	try {
	    RETVAL = THIS->get_lastdocid();
	} catch (...) {
	    handle_exception();
	}
    OUTPUT:
	RETVAL

doclength
Database::get_avlength()
    CODE:
	try {
	    RETVAL = THIS->get_avlength();
	} catch (...) {
	    handle_exception();
	}
    OUTPUT:
	RETVAL

doccount
Database::get_termfreq(tname)
    string	tname
    CODE:
	try {
	    RETVAL = THIS->get_termfreq(tname);
	} catch (...) {
	    handle_exception();
	}
    OUTPUT:
	RETVAL

bool
Database::term_exists(tname)
    string	tname
    CODE:
	try {
	    RETVAL = THIS->term_exists(tname);
	} catch (...) {
	    handle_exception();
	}
    OUTPUT:
	RETVAL

termcount
Database::get_collection_freq(tname)
    string	tname
    CODE:
	try {
	    RETVAL = THIS->get_collection_freq(tname);
	} catch (...) {
	    handle_exception();
	}
    OUTPUT:
	RETVAL

doclength
Database::get_doclength(did)
    docid	did
    CODE:
	try {
	    RETVAL = THIS->get_doclength(did);
	} catch (...) {
	    handle_exception();
	}
    OUTPUT:
	RETVAL

void
Database::keep_alive()
    CODE:
	try {
	    THIS->keep_alive();
	} catch (...) {
	    handle_exception();
	}

Document *
Database::get_document(docid did)
    CODE:
	try {
	    RETVAL = new Document(THIS->get_document(did));
	} catch (...) {
	    handle_exception();
	}
    OUTPUT:
	RETVAL

string
Database::get_spelling_suggestion(word, max_edit_distance = 2)
    string  word
    int     max_edit_distance
    CODE:
	try {
	    RETVAL = THIS->get_spelling_suggestion(word, max_edit_distance);
	} catch (...) {
	    handle_exception();
	}
    OUTPUT:
	RETVAL

string
Database::get_metadata(string key)

void
Database::DESTROY()
