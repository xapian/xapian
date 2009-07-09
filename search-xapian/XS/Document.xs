MODULE = Search::Xapian			PACKAGE = Search::Xapian::Document

PROTOTYPES: ENABLE

Document *
new1()
    CODE:
        RETVAL = new Document();
    OUTPUT:
        RETVAL

Document *
new2(other)
    Document * other
    CODE:
        RETVAL = new Document(*other);
    OUTPUT:
        RETVAL

string
Document::get_value(valueno valno)
    CODE:
	try {
	    RETVAL = THIS->get_value(valno);
	} catch (const Error &error) {
	    croak("Exception: %s", error.get_msg().c_str());
	}
    OUTPUT:
	RETVAL

void
Document::add_value(valno, value)
    valueno	valno
    string	value
    CODE:
	try {
	    THIS->add_value(valno, value);
	} catch (const Error &error) {
	    croak("Exception: %s", error.get_msg().c_str());
	}

void
Document::remove_value(valueno valno)
    CODE:
	try {
	    THIS->remove_value(valno);
	} catch (const Error &error) {
	    croak("Exception: %s", error.get_msg().c_str());
	}

void
Document::clear_values()

string
Document::get_data()
    CODE:
	try {
	    RETVAL = THIS->get_data();
	} catch (const Error &error) {
	    croak("Exception: %s", error.get_msg().c_str());
	}
    OUTPUT:
	RETVAL

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
	try {
	    if (items == 4) { /* items includes the hidden this pointer */
		THIS->add_posting(tname, tpos, wdfinc);
	    } else {
		THIS->add_posting(tname, tpos);
	    }
	} catch (const Error &error) {
	    croak("Exception: %s", error.get_msg().c_str());
	}

void
Document::add_term(tname, wdfinc = NO_INIT)
    string	tname
    termcount	wdfinc
    CODE:
	try {
	    if (items == 3) { /* items includes the hidden this pointer */
		THIS->add_term(tname, wdfinc);
	    } else {
		THIS->add_term(tname);
	    }
	} catch (const Error &error) {
	    croak("Exception: %s", error.get_msg().c_str());
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
        } catch (const Error &error) {
	    croak("Exception: %s", error.get_msg().c_str());
        }

void
Document::remove_term(tname)
    string	tname
    CODE:
	try {
            THIS->remove_term(tname);  
        } catch (const Error &error) {
            croak("Exception: %s", error.get_msg().c_str());
        }

void
Document::clear_terms()

termcount
Document::termlist_count()
    CODE:
	try {
	    RETVAL = THIS->termlist_count();
	} catch (const Error &error) {
	    croak("Exception: %s", error.get_msg().c_str());
	}
    OUTPUT:
        RETVAL

TermIterator *
Document::termlist_begin()
    CODE:
	try {
	    RETVAL = new TermIterator(THIS->termlist_begin());
	} catch (const Error &error) {
	    croak("Exception: %s", error.get_msg().c_str());
	}
    OUTPUT:
        RETVAL

TermIterator *
Document::termlist_end()
    CODE:
        RETVAL = new TermIterator(THIS->termlist_end());
    OUTPUT:
        RETVAL

termcount
Document::values_count()
    CODE:
	try {
	    RETVAL = THIS->values_count();
	} catch (const Error &error) {
	    croak("Exception: %s", error.get_msg().c_str());
	}
    OUTPUT:
        RETVAL

ValueIterator *
Document::values_begin()
    CODE:
	try {
	    RETVAL = new ValueIterator(THIS->values_begin());
	} catch (const Error &error) {
	    croak("Exception: %s", error.get_msg().c_str());
	}
    OUTPUT:
        RETVAL

ValueIterator *
Document::values_end()
    CODE:
        RETVAL = new ValueIterator(THIS->values_end());
    OUTPUT:
        RETVAL

string
Document::get_description()

void
Document::DESTROY()
