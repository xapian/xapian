MODULE = Search::Xapian 		PACKAGE = Search::Xapian::Document		

PROTOTYPES: ENABLE

Document *
new1();
    CODE:
        RETVAL = new Document();
    OUTPUT:
        RETVAL

Document *
new2(other);
    Document * other
    CODE:
        RETVAL = new Document(*other);
    OUTPUT:
        RETVAL
 
string
Document::get_value(valueno valno)

void
Document::add_value(valno, value)
    valueno	valno
    string	value
    CODE:
        THIS->add_value(valno, value);

string
Document::get_data()

void
Document::set_data(data)
    string	data
    CODE:
        THIS->set_data(data);

void
Document::add_posting(tname, tpos)
    string	tname
    termpos	tpos
    CODE:
        THIS->add_posting(tname, tpos);

void
Document::add_term(tname)
    string	tname
    CODE:
        THIS->add_term(tname);

void
Document::add_term_nopos(tname)
    string	tname
    CODE:
        THIS->add_term_nopos(tname);

void
Document::remove_posting(tname, tpos)
    string	tname
    termpos	tpos
    CODE:
        THIS->remove_posting(tname, tpos);

void
Document::remove_term(tname)
    string	tname
    CODE:
        THIS->remove_term(tname);  

void
Document::clear_terms()

termcount
Document::termlist_count()

TermIterator *
Document::termlist_begin()
    CODE:
        RETVAL = new TermIterator();
        *RETVAL = THIS->termlist_begin();
    OUTPUT:
        RETVAL

TermIterator *
Document::termlist_end()
    CODE:
        RETVAL = new TermIterator();
        *RETVAL = THIS->termlist_end();
    OUTPUT:
        RETVAL

termcount
Document::values_count()

ValueIterator *
Document::values_begin()
    CODE:
        RETVAL = new ValueIterator();
        *RETVAL = THIS->values_begin();
    OUTPUT:
        RETVAL

ValueIterator *
Document::values_end()
    CODE:
        RETVAL = new ValueIterator();
        *RETVAL = THIS->values_end();
    OUTPUT:
        RETVAL

string
Document::get_description()

void
Document::DESTROY()
