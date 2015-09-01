MODULE = Search::Xapian	 PACKAGE = Search::Xapian::Enquire

PROTOTYPES: ENABLE

Enquire *
Enquire::new(databases)
    Database *  databases
    CODE:
	try {
	    RETVAL = XAPIAN_PERL_NEW(Enquire, (*databases));
	} catch (...) {
	    handle_exception();
	}
    OUTPUT:
	RETVAL

void
Enquire::set_query1(query)
    Query *     query
    CODE:
	try {
	    THIS->set_query(*query);
	} catch (...) {
	    handle_exception();
	}

void
Enquire::set_query2(query, len)
    Query *     query
    termcount   len
    CODE:
	try {
	    THIS->set_query(*query, len);
	} catch (...) {
	    handle_exception();
	}

Query *
Enquire::get_query()
    CODE:
	try {
	    RETVAL = new Query(THIS->get_query());
	} catch (...) {
	    handle_exception();
	}
    OUTPUT:
	RETVAL

void
Enquire::set_collapse_key(collapse_key, collapse_max = 1)
    valueno     collapse_key
    doccount    collapse_max
    CODE:
	try {
	    THIS->set_collapse_key(collapse_key, collapse_max);
	} catch (...) {
	    handle_exception();
	}

void
Enquire::set_docid_order(order)
    int	 order
    CODE:
	try {
	    THIS->set_docid_order(static_cast<Enquire::docid_order>(order));
	} catch (...) {
	    handle_exception();
	}

void
Enquire::set_cutoff(percent_cutoff, weight_cutoff = NO_INIT)
    percent     percent_cutoff
    weight      weight_cutoff
    CODE:
	try {
	    if (items == 3) { /* items includes the hidden this pointer */
		THIS->set_cutoff(percent_cutoff, weight_cutoff);
	    } else {
		THIS->set_cutoff(percent_cutoff);
	    }
	} catch (...) {
	    handle_exception();
	}

void
Enquire::set_sort_by_relevance()
    CODE:
	// Clear reference to any currently set sorter object.
	XAPIAN_PERL_REF(Enquire, THIS, sorter, NULL);
	THIS->set_sort_by_relevance();

void
Enquire::set_sort_by_value(sort_key, ascending = NO_INIT)
    valueno	sort_key
    bool	ascending
    CODE:
	// Clear reference to any currently set sorter object.
	XAPIAN_PERL_REF(Enquire, THIS, sorter, NULL);
	try {
	    if (items == 3) { /* items includes the hidden this pointer */
		THIS->set_sort_by_value(sort_key, ascending);
	    } else {
		THIS->set_sort_by_value(sort_key);
	    }
	} catch (...) {
	    handle_exception();
	}

void
Enquire::set_sort_by_value_then_relevance(sort_key, ascending = NO_INIT)
    valueno	sort_key
    bool	ascending
    CODE:
	// Clear reference to any currently set sorter object.
	XAPIAN_PERL_REF(Enquire, THIS, sorter, NULL);
	try {
	    if (items == 3) { /* items includes the hidden this pointer */
		THIS->set_sort_by_value_then_relevance(sort_key, ascending);
	    } else {
		THIS->set_sort_by_value_then_relevance(sort_key);
	    }
	} catch (...) {
	    handle_exception();
	}

void
Enquire::set_sort_by_relevance_then_value(sort_key, ascending = NO_INIT)
    valueno	sort_key
    bool	ascending
    CODE:
	// Clear reference to any currently set sorter object.
	XAPIAN_PERL_REF(Enquire, THIS, sorter, NULL);
	try {
	    if (items == 3) { /* items includes the hidden this pointer */
		THIS->set_sort_by_relevance_then_value(sort_key, ascending);
	    } else {
		THIS->set_sort_by_relevance_then_value(sort_key);
	    }
	} catch (...) {
	    handle_exception();
	}

void
Enquire::set_sort_by_key(sorter, ascending = NO_INIT)
    MultiValueSorter * sorter
    bool	ascending
    CODE:
	// Keep a reference to the currently set object.
	XAPIAN_PERL_REF(Enquire, THIS, sorter, ST(1));
	try {
	    if (items == 3) { /* items includes the hidden this pointer */
		THIS->set_sort_by_key(sorter, ascending);
	    } else {
		THIS->set_sort_by_key(sorter);
	    }
	} catch (...) {
	    handle_exception();
	}

void
Enquire::set_sort_by_key_then_relevance(sorter, ascending = NO_INIT)
    MultiValueSorter * sorter
    bool	ascending
    CODE:
	// Keep a reference to the currently set object.
	XAPIAN_PERL_REF(Enquire, THIS, sorter, ST(1));
	try {
	    if (items == 3) { /* items includes the hidden this pointer */
		THIS->set_sort_by_key_then_relevance(sorter, ascending);
	    } else {
		THIS->set_sort_by_key_then_relevance(sorter);
	    }
	} catch (...) {
	    handle_exception();
	}

void
Enquire::set_sort_by_relevance_then_key(sorter, ascending = NO_INIT)
    MultiValueSorter * sorter
    bool	ascending
    CODE:
	// Keep a reference to the currently set object.
	XAPIAN_PERL_REF(Enquire, THIS, sorter, ST(1));
	try {
	    if (items == 3) { /* items includes the hidden this pointer */
		THIS->set_sort_by_relevance_then_key(sorter, ascending);
	    } else {
		THIS->set_sort_by_relevance_then_key(sorter);
	    }
	} catch (...) {
	    handle_exception();
	}

MSet *
Enquire::get_mset1(first, maxitems, checkatleast = NO_INIT, rset = NO_INIT, func = NO_INIT)
    doccount    first
    doccount    maxitems
    doccount    checkatleast
    RSet *	rset
    SV *	func
    CODE:
	try {
	    MSet mset;
	    switch (items) { /* items includes the hidden this pointer */
		case 3:
		    mset = THIS->get_mset(first, maxitems);
		    break;
		case 4:
		    mset = THIS->get_mset(first, maxitems, checkatleast);
		    break;
		case 5:
		    mset = THIS->get_mset(first, maxitems, checkatleast, rset);
		    break;
		case 6: {
		    perlMatchDecider d(func);
		    mset = THIS->get_mset(first, maxitems, checkatleast, rset, &d);
		    break;
		}
		default:
		    croak("Bad parameter count for get_mset1");
	    }
	    RETVAL = new MSet(mset);
	} catch (...) {
	    handle_exception();
	}
    OUTPUT:
	RETVAL

MSet *
Enquire::get_mset2(first, maxitems, func)
    doccount    first
    doccount    maxitems
    SV *	func
    CODE:
	try {
	    perlMatchDecider d(func);
	    RETVAL = new MSet(THIS->get_mset(first, maxitems, 0, NULL, &d));
	} catch (...) {
	    handle_exception();
	}
    OUTPUT:
	RETVAL

ESet *
Enquire::get_eset(maxitems, rset, func = NO_INIT)
    doccount    maxitems
    RSet *      rset
    SV *	func
    CODE:
	try {
	    ESet eset;
	    switch (items) { /* items includes the hidden this pointer */
		case 3:
		    eset = THIS->get_eset(maxitems, *rset);
		    break;
		case 4: {
		    perlExpandDecider d(func);
		    eset = THIS->get_eset(maxitems, *rset, &d);
		    break;
		}
		default:
		    croak("Bad parameter count for get_eset");
	    }
	    RETVAL = new ESet(eset);
	} catch (...) {
	    handle_exception();
	}
    OUTPUT:
	RETVAL

TermIterator *
Enquire::get_matching_terms_begin1(docid did)
    CODE:
	try {
	    RETVAL = new TermIterator(THIS->get_matching_terms_begin(did));
	} catch (...) {
	    handle_exception();
	}
    OUTPUT:
	RETVAL

TermIterator *
Enquire::get_matching_terms_begin2(it)
    MSetIterator *	it
    CODE:
	try {
	    RETVAL = new TermIterator(THIS->get_matching_terms_begin(* it));
	} catch (...) {
	    handle_exception();
	}
    OUTPUT:
	RETVAL

TermIterator *
Enquire::get_matching_terms_end1(docid did)
    CODE:
	try {
	    RETVAL = new TermIterator(THIS->get_matching_terms_end(did));
	} catch (...) {
	    handle_exception();
	}
    OUTPUT:
	RETVAL

TermIterator *
Enquire::get_matching_terms_end2(it)
    MSetIterator *  it
    CODE:
	try {
	    RETVAL = new TermIterator(THIS->get_matching_terms_end(* it));
	} catch (...) {
	    handle_exception();
	}
    OUTPUT:
	RETVAL

void
Enquire::set_weighting_scheme(weight_)
	Weight *  weight_
    CODE:
	try {
	    THIS->set_weighting_scheme(*weight_);
	} catch (...) {
	    handle_exception();
	}

string
Enquire::get_description()

void
Enquire::add_matchspy(MatchSpy * spy)
    CODE:
    try {
        XAPIAN_PERL_REF(Enquire, THIS, matchspy, ST(1));
        THIS->add_matchspy(spy);
    } catch (...) {
        handle_exception();
    }

void
Enquire::clear_matchspies()
    CODE:
    try {
        XAPIAN_PERL_REF(Enquire, THIS, clear_matchspies,  NULL);
        THIS->clear_matchspies();
    } catch (...) {
        handle_exception();
    }

void
Enquire::DESTROY()
    CODE:
	XAPIAN_PERL_DESTROY(Enquire, THIS);
