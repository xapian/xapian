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

void
Document::remove_value(valueno valno)

void
Document::clear_values()

string
Document::get_data()

void
Document::set_data(data)
    string	data
    CODE:
        THIS->set_data(data);

void
Document::add_posting(tname, tpos, wdfinc = NO_INIT)
    string	tname
    termpos	tpos
    termcount	wdfinc
    CODE:
	/* FIXME: catch exceptions for case where tname is empty? */
        if (items == 4) { /* items includes the hidden this pointer */
            THIS->add_posting(tname, tpos, wdfinc);
        } else {
            THIS->add_posting(tname, tpos);
        }

void
Document::add_term(tname, wdfinc = NO_INIT)
    string	tname
    termcount	wdfinc
    CODE:
        if (items == 3) { /* items includes the hidden this pointer */
            THIS->add_term(tname, wdfinc);
        } else {
            THIS->add_term(tname);
        }

void
Document::remove_posting(tname, tpos, wdfdec = NO_INIT)
    string	tname
    termpos	tpos
    termcount	wdfdec
    CODE:
	try {
            if (items == 4) { /* items includes the hidden this pointer */
                THIS->remove_posting(tname, tpos, wdfdec);
            } else {
                THIS->remove_posting(tname, tpos);
            }
        }
        catch (const Error &error) {
            croak( "Exception: %s", error.get_msg().c_str() );
        }

void
Document::remove_term(tname)
    string	tname
    CODE:
	try {
            THIS->remove_term(tname);  
        }
        catch (const Error &error) {
            croak( "Exception: %s", error.get_msg().c_str() );
        }

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
