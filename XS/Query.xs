MODULE = Search::Xapian  		PACKAGE = Search::Xapian::Query		

PROTOTYPES: ENABLE

OmQuery *
new1(term);
    om_termname term
    CODE:
        RETVAL = new OmQuery(term);
    OUTPUT:
        RETVAL

OmQuery *
new2(op, left, right);
    int          op
    om_termname left
    om_termname right
    CODE:
        RETVAL = new OmQuery( (OmQuery::op)op, left, right );
    OUTPUT:
        RETVAL

OmQuery *
new3(op, left, right);
    int          op
    OmQuery *   left
    OmQuery *   right
    CODE:
        RETVAL = new OmQuery( (OmQuery::op)op, *left, *right );
    OUTPUT:
        RETVAL


void
OmQuery::set_window(om_termpos window)

void
OmQuery::set_cutoff(om_weight cutoff)

void
OmQuery::set_elite_set_size(om_termcount size)

om_termcount
OmQuery::get_length()

om_termcount
OmQuery::set_length(om_termcount qlen)

OmTermIterator *
OmQuery::get_terms_begin()
    CODE:
        RETVAL = new OmTermIterator();
        *RETVAL = THIS->get_terms_begin();
    OUTPUT:
        RETVAL

OmTermIterator *
OmQuery::get_terms_end()
    CODE:
        RETVAL = new OmTermIterator();
        *RETVAL = THIS->get_terms_begin();
    OUTPUT:
        RETVAL

void
OmQuery::is_empty()

string
OmQuery::get_description()

void
OmQuery::DESTROY()