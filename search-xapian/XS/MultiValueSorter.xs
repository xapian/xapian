MODULE = Search::Xapian		PACKAGE = Search::Xapian::MultiValueSorter

PROTOTYPES: ENABLE

MultiValueKeyMaker *
new0()
    CODE:
	RETVAL = new MultiValueKeyMaker();
    OUTPUT:
	RETVAL

void
MultiValueKeyMaker::add(valueno valno, bool forward = NO_INIT)
    CODE:
	if (items == 3) { /* items includes the hidden this pointer */
	    THIS->add_value(valno, !forward);
	} else {
	    THIS->add_value(valno);
	}

void
MultiValueKeyMaker::DESTROY()
