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
new2(op, left, right);
    int		op
    string	left
    string	right
    CODE:
        RETVAL = new Query( (Query::op) op, left, right );
    OUTPUT:
        RETVAL

Query *
new3(op, left, right);
    int		op
    Query *	left
    Query *	right
    CODE:
        RETVAL = new Query( (Query::op) op, *left, *right );
    OUTPUT:
        RETVAL


void
Query::set_window(termpos window)

void
Query::set_cutoff(weight cutoff)

void
Query::set_elite_set_size(termcount size)

termcount
Query::get_length()

termcount
Query::set_length(termcount qlen)

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

void
Query::is_empty()

string
Query::get_description()

void
Query::DESTROY()
