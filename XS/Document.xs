MODULE = Search::Xapian 		PACKAGE = Search::Xapian::Document		

PROTOTYPES: ENABLE

OmDocument *
new1();
    CODE:
        RETVAL = new OmDocument();
    OUTPUT:
        RETVAL

OmDocument *
new2(other);
    OmDocument * other
    CODE:
        RETVAL = new OmDocument(* other);
    OUTPUT:
        RETVAL
 
string
OmDocument::get_value(om_valueno value)

void
OmDocument::add_value(valueno, value)
    om_valueno  valueno
    string *    value
    CODE:
        THIS->add_value(valueno, *value);

string
OmDocument::get_data()

void
OmDocument::set_data(data)
    string * data
    CODE:
        THIS->set_data(*data);

void
OmDocument::add_posting(tname, tpos)
    om_termname  tname
    om_termpos   tpos

void
OmDocument::add_term_nopos(tname)
    om_termname  tname

void
OmDocument::remove_posting(tname, tpos)
    om_termname  tname
    om_termpos   tpos

void
OmDocument::remove_term(tname)
    om_termname  tname

void
OmDocument::clear_terms()

om_termcount
OmDocument::termlist_count()

OmTermIterator *
OmDocument::termlist_begin()
    CODE:
        RETVAL = new OmTermIterator();
        *RETVAL = THIS->termlist_begin();
    OUTPUT:
        RETVAL

OmTermIterator *
OmDocument::termlist_end()
    CODE:
        RETVAL = new OmTermIterator();
        *RETVAL = THIS->termlist_end();
    OUTPUT:
        RETVAL

om_termcount
OmDocument::values_count()

OmValueIterator *
OmDocument::values_begin()
    CODE:
        RETVAL = new OmValueIterator();
        *RETVAL = THIS->values_begin();
    OUTPUT:
        RETVAL

OmValueIterator *
OmDocument::values_end()
    CODE:
        RETVAL = new OmValueIterator();
        *RETVAL = THIS->values_end();
    OUTPUT:
        RETVAL

string
OmDocument::get_description()

void
OmDocument::DESTROY()