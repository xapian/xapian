MODULE = Search::Xapian		PACKAGE = Search::Xapian::Enquire		

PROTOTYPES: ENABLE

OmEnquire *
OmEnquire::new(databases)
    OmDatabase * databases
    CODE:
        RETVAL = new OmEnquire(* databases);
    OUTPUT:
        RETVAL

void
OmEnquire::set_query_object(query)
    OmQuery *    query
    CODE:
        THIS->set_query(* query);

OmQuery *
OmEnquire::get_query();
    CODE:
        RETVAL = new OmQuery();
        *RETVAL = THIS->get_query();
    OUTPUT:
        RETVAL

OmMSet *
OmEnquire::get_mset(first, maxitems)
    om_doccount first
    om_doccount maxitems
    CODE:
        RETVAL = new OmMSet();
        *RETVAL = THIS->get_mset(first, maxitems);
    OUTPUT:
        RETVAL

OmESet *
OmEnquire::get_eset(maxitems, omrset)
    om_doccount maxitems
    OmRSet *    omrset
    CODE:
        RETVAL = new OmESet();
        *RETVAL = THIS->get_eset(maxitems, * omrset);
    OUTPUT:
        RETVAL

OmTermIterator *
OmEnquire::get_matching_terms_begin1(om_docid did)
    CODE:
        RETVAL = new OmTermIterator();
        *RETVAL = THIS->get_matching_terms_begin(did);
    OUTPUT:
        RETVAL

OmTermIterator *
OmEnquire::get_matching_terms_begin2(it)
        OmMSetIterator *        it
    CODE:
        RETVAL = new OmTermIterator();
        *RETVAL = THIS->get_matching_terms_begin(* it);
    OUTPUT:
        RETVAL

OmTermIterator *
OmEnquire::get_matching_terms_end1(om_docid did)
    CODE:
        RETVAL = new OmTermIterator();
        *RETVAL = THIS->get_matching_terms_end(did);
    OUTPUT:
        RETVAL

OmTermIterator *
OmEnquire::get_matching_terms_end2(it)
        OmMSetIterator *        it
    CODE:
        RETVAL = new OmTermIterator();
        *RETVAL = THIS->get_matching_terms_end(* it);
    OUTPUT:
        RETVAL

void
OmEnquire::register_match_decoder(name, mdecider)
        string *         name
        OmMatchDecider * mdecider
    CODE:   
        THIS->register_match_decider(* name, mdecider);

string
OmEnquire::get_description()

void
OmEnquire::DESTROY()