MODULE = Search::Xapian  		PACKAGE = Search::Xapian::Query		

PROTOTYPES: ENABLE

Query *
new1(term);
    string	term
    CODE:
        RETVAL = new Query(term);
    OUTPUT:
        RETVAL

Query *
new2sv(op, subq);
    int		op
    string	subq
    CODE:
        RETVAL = new Query( (Query::op) op, subq );
    OUTPUT:
        RETVAL

Query *
new2obj(op, subq);
    int		op
    Query *	subq
    CODE:
        RETVAL = new Query( (Query::op) op, *subq );
    OUTPUT:
        RETVAL

Query *
newXsv(op, ...);
    int		op
    PREINIT:
        vector<string> terms;
    CODE:
        for( int i = 1; i <= items; i++ ) {
            SV *sv = ST (i);
	    if( SvOK(sv) && SvPOK(sv) ) {
		string term = SvPV_nolen(sv);
	        terms.push_back(term);
	    }
        }
	try {
            RETVAL = new Query( (Query::op) op, terms.begin(), terms.end() );
        }
        catch (const Error &error) {
            croak( "Exception: %s", error.get_msg().c_str() );
        }
    OUTPUT:
        RETVAL

Query *
newXobj(op, ...);
    int		op
    PREINIT:
        vector<Query> queries;
    CODE:
        for( int i = 1; i <= items; i++ ) {
            SV *sv = ST (i);
	    if( sv_isobject(sv) ) {
		Query *query = (Query*) SvIV((SV*) SvRV(sv));
	        queries.push_back(*query);
	    }
        }
	try {
            RETVAL = new Query( (Query::op) op, queries.begin(), queries.end() );
        }
        catch (const Error &error) {
            croak( "Exception: %s", error.get_msg().c_str() );
        }
    OUTPUT:
        RETVAL

void
Query::set_window(window)
    termpos	window
    CODE:
	THIS->set_window(window);

void
Query::set_cutoff(cutoff)
    weight	cutoff
    CODE:
        THIS->set_cutoff(cutoff);

void
Query::set_elite_set_size(size)
    termcount	size
    CODE:
	THIS->set_elite_set_size(size);

termcount
Query::get_length()

termcount
Query::set_length(qlen)
    termcount	qlen
    CODE:
        THIS->set_length(qlen);

TermIterator *
Query::get_terms_begin()
    CODE:
        RETVAL = new TermIterator();
        *RETVAL = THIS->get_terms_begin();
    OUTPUT:
        RETVAL

TermIterator *
Query::get_terms_end()
    CODE:
        RETVAL = new TermIterator();
        *RETVAL = THIS->get_terms_begin();
    OUTPUT:
        RETVAL

bool
Query::is_empty()

string
Query::get_description()

void
Query::DESTROY()
