MODULE = Search::Xapian         PACKAGE = Search::Xapian::Enquire               

PROTOTYPES: ENABLE

Enquire *
Enquire::new(databases)
    Database *  databases
    CODE:
        RETVAL = new Enquire(*databases);
    OUTPUT:
        RETVAL

void
Enquire::set_query_object(query)
    Query *     query
    CODE:
        THIS->set_query(*query);

Query *
Enquire::get_query();
    CODE:
        RETVAL = new Query();
        *RETVAL = THIS->get_query();
    OUTPUT:
        RETVAL

void
Enquire::set_collapse_key(collapse_key)
    valueno     collapse_key
    CODE:
        THIS->set_collapse_key(collapse_key);

void
Enquire::set_sort_forward(sort_forward)
    bool        sort_forward
    CODE:
        THIS->set_sort_forward(sort_forward);

void
Enquire::set_cutoff(percent_cutoff, weight_cutoff = NO_INIT)
    percent     percent_cutoff
    weight      weight_cutoff
    CODE:
        if (items == 2) {
            THIS->set_cutoff(percent_cutoff, weight_cutoff);
        } else {
            THIS->set_cutoff(percent_cutoff);
        }

void
Enquire::set_sorting(sort_key, sort_bands)
    valueno     sort_key
    int         sort_bands
    CODE:
        THIS->set_sorting(sort_key, sort_bands);

void
Enquire::set_bias(bias_weight, bias_halflife)
    weight      bias_weight
    time_t      bias_halflife
    CODE:
        THIS->set_bias(bias_weight, bias_halflife);

MSet *
Enquire::get_mset(first, maxitems)
    doccount    first
    doccount    maxitems
    CODE:
        RETVAL = new MSet();
        *RETVAL = THIS->get_mset(first, maxitems);
    OUTPUT:
        RETVAL

ESet *
Enquire::get_eset(maxitems, rset)
    doccount    maxitems
    RSet *      rset
    CODE:
        RETVAL = new ESet();
        *RETVAL = THIS->get_eset(maxitems, *rset);
    OUTPUT:
        RETVAL

TermIterator *
Enquire::get_matching_terms_begin1(docid did)
    CODE:
        RETVAL = new TermIterator();
        *RETVAL = THIS->get_matching_terms_begin(did);
    OUTPUT:
        RETVAL

TermIterator *
Enquire::get_matching_terms_begin2(it)
        MSetIterator *        it
    CODE:
        RETVAL = new TermIterator();
        *RETVAL = THIS->get_matching_terms_begin(* it);
    OUTPUT:
        RETVAL

TermIterator *
Enquire::get_matching_terms_end1(docid did)
    CODE:
        RETVAL = new TermIterator();
        *RETVAL = THIS->get_matching_terms_end(did);
    OUTPUT:
        RETVAL

TermIterator *
Enquire::get_matching_terms_end2(it)
        MSetIterator *  it
    CODE:
        RETVAL = new TermIterator();
        *RETVAL = THIS->get_matching_terms_end(* it);
    OUTPUT:
        RETVAL

void
Enquire::register_match_decoder(name, mdecider)
        string          name
        MatchDecider *  mdecider
    CODE:   
        THIS->register_match_decider(name, mdecider);

string
Enquire::get_description()

void
Enquire::DESTROY()
