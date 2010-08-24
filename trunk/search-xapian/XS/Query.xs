MODULE = Search::Xapian			PACKAGE = Search::Xapian::Query

PROTOTYPES: ENABLE

Query *
new1(term)
    string	term
    CODE:
        RETVAL = new Query(term);
    OUTPUT:
        RETVAL

Query *
new1weight(term, wqf, pos)
    string	term
    termcount	wqf
    termpos	pos
    CODE:
	RETVAL = new Query(term, wqf, pos);
    OUTPUT:
	RETVAL

Query *
new3scale(int op, Query * query, double factor)
    CODE:
        RETVAL = new Query( (Query::op) op, *query, factor );
    OUTPUT:
        RETVAL

Query *
new3range(op, valno, limit)
    int		op
    valueno	valno
    string	limit
    CODE:
        RETVAL = new Query( (Query::op) op, valno, limit );
    OUTPUT:
        RETVAL

Query *
new4range(op, valno, start, end)
    int		op
    valueno	valno
    string	start
    string	end
    CODE:
        RETVAL = new Query( (Query::op) op, valno, start, end );
    OUTPUT:
        RETVAL

Query *
newN(int op_, ...)
    CODE:
	Query::op op = (Query::op)op_;
	try {
	    vector<Query> queries;
	    queries.reserve(items - 1);
	    for( int i = 1; i < items; i++ ) {
		SV *sv = ST (i);
		if (sv_isa(sv, "Search::Xapian::Query")) {
		    Query *query = (Query*) SvIV((SV*) SvRV(sv));
		    queries.push_back(*query);
		} else if ( SvOK(sv) ) {
		    STRLEN len;
		    const char * ptr = SvPV(sv, len);
		    queries.push_back(Query(string(ptr, len)));
		} else {
		    croak( "USAGE: Search::Xapian::Query->new(OP, @TERMS_OR_QUERY_OBJECTS)" );
		}
	    }
            RETVAL = new Query(op, queries.begin(), queries.end());
        } catch (const Error &error) {
            croak( "Exception: %s", error.get_msg().c_str() );
        }
    OUTPUT:
        RETVAL

termcount
Query::get_length()

TermIterator *
Query::get_terms_begin()
    CODE:
        RETVAL = new TermIterator(THIS->get_terms_begin());
    OUTPUT:
        RETVAL

TermIterator *
Query::get_terms_end()
    CODE:
        RETVAL = new TermIterator(THIS->get_terms_end());
    OUTPUT:
        RETVAL

bool
Query::empty()

string
Query::get_description()

void
Query::DESTROY()
