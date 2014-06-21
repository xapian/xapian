MODULE = Search::Xapian		PACKAGE = Search::Xapian::WritableDatabase

PROTOTYPES: ENABLE

WritableDatabase *
new1(file, opts)
    string	file
    int		opts
    CODE:
	try {
	    RETVAL = new WritableDatabase(file, opts);
	} catch (...) {
	    handle_exception();
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

WritableDatabase *
new3()
    CODE:
	try {
	    RETVAL = new WritableDatabase(InMemory::open());
	} catch (...) {
	    handle_exception();
	}
    OUTPUT:
	RETVAL

void
WritableDatabase::flush()
   CODE:
	try {
	    THIS->flush();
	} catch (...) {
	    handle_exception();
	}


void
WritableDatabase::begin_transaction(flushed = NO_INIT)
    bool flushed
    CODE:
	try {
	    if (items == 2) { /* items includes the hidden this pointer */
		THIS->begin_transaction(flushed);
	    } else {
		THIS->begin_transaction();
	    }
	} catch (...) {
	    handle_exception();
	}

void
WritableDatabase::commit_transaction()
    CODE:
	try {
	    THIS->commit_transaction();
	} catch (...) {
	    handle_exception();
	}

void
WritableDatabase::cancel_transaction()
    CODE:
	try {
	    THIS->cancel_transaction();
	} catch (...) {
	    handle_exception();
	}

docid
WritableDatabase::add_document(document)
    Document *	document
    CODE:
	try {
	    RETVAL = THIS->add_document(*document);
	} catch (...) {
	    handle_exception();
	}
    OUTPUT:
	RETVAL

void
WritableDatabase::delete_document(did)
    docid	did
    CODE:
	try {
	    THIS->delete_document(did);
	} catch (...) {
	    handle_exception();
	}

void
WritableDatabase::delete_document_by_term(unique_term)
    string	unique_term
    CODE:
	try {
	    THIS->delete_document(unique_term);
	} catch (...) {
	    handle_exception();
	}

void
WritableDatabase::replace_document(did, document)
    docid	did
    Document *	document
    CODE:
	try {
	    THIS->replace_document(did, *document);
	} catch (...) {
	    handle_exception();
	}

void
WritableDatabase::replace_document_by_term(unique_term, document)
    string	unique_term
    Document *	document
    CODE:
	try {
	    THIS->replace_document(unique_term, *document);
	} catch (...) {
	    handle_exception();
	}

void
WritableDatabase::set_metadata(string key, string value)
    CODE:
	try {
	    THIS->set_metadata(key, value);
	} catch (...) {
	    handle_exception();
	}

void
WritableDatabase::DESTROY()

void
WritableDatabase::add_synonym(string term, string synonym)
    CODE:
	try {
	    THIS->add_synonym(term, synonym);
	} catch (...) {
	    handle_exception();
	}

void
WritableDatabase::remove_synonym(string term, string synonym)
    CODE:
	try {
	    THIS->remove_synonym(term, synonym);
	} catch (...) {
	    handle_exception();
	}

void
WritableDatabase::clear_synonyms(string term)
    CODE:
	try {
	    THIS->clear_synonyms(term);
	} catch (...) {
	    handle_exception();
	}

void
WritableDatabase::add_spelling(word, freqinc = 1)
    string word
    termcount freqinc
    CODE:
	try {
	    THIS->add_spelling(word, freqinc);
	} catch (...) {
	    handle_exception();
	}

void
WritableDatabase::remove_spelling(word, freqdec  = 1)
    string word
    termcount freqdec
    CODE:
	try {
	    THIS->remove_spelling(word, freqdec);
	} catch (...) {
	    handle_exception();
	}
