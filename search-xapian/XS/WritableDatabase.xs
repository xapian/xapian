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
WritableDatabase::reopen()
    CODE:
	try {
            THIS->reopen();
	} catch (...) {
	    handle_exception();
        }

string
WritableDatabase::get_description()
    CODE:
	try {
            RETVAL = THIS->get_description();
	} catch (...) {
	    handle_exception();
        }
    OUTPUT:
        RETVAL

TermIterator *
WritableDatabase::termlist_begin(did)
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
WritableDatabase::termlist_end(did)
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
WritableDatabase::positionlist_begin(did, term)
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
WritableDatabase::positionlist_end(did, term)
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
WritableDatabase::allterms_begin(prefix = NO_INIT)
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
WritableDatabase::allterms_end(prefix = NO_INIT)
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
WritableDatabase::postlist_begin(term)
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
WritableDatabase::postlist_end(term)
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
WritableDatabase::get_doccount()
    CODE:
	try {
            RETVAL = THIS->get_doccount();
	} catch (...) {
	    handle_exception();
        }
    OUTPUT:
        RETVAL

docid
WritableDatabase::get_lastdocid()
    CODE:
	try {
            RETVAL = THIS->get_lastdocid();
	} catch (...) {
	    handle_exception();
        }
    OUTPUT:
        RETVAL

doclength
WritableDatabase::get_avlength()
    CODE:
	try {
            RETVAL = THIS->get_avlength();
	} catch (...) {
	    handle_exception();
        }
    OUTPUT:
        RETVAL

doccount
WritableDatabase::get_termfreq(tname)
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
WritableDatabase::term_exists(tname)
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
WritableDatabase::get_collection_freq(tname)
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
WritableDatabase::get_doclength(did)
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
WritableDatabase::keep_alive()
    CODE:
	try {
	    THIS->keep_alive();
	} catch (...) {
	    handle_exception();
        }


Document *
WritableDatabase::get_document(docid did)
    CODE:
	try {
	    RETVAL = new Document(THIS->get_document(did));
	} catch (...) {
	    handle_exception();
        }
    OUTPUT:
        RETVAL

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
