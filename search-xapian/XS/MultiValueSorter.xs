MODULE = Search::Xapian		PACKAGE = Search::Xapian::MultiValueSorter

PROTOTYPES: ENABLE

MultiValueSorter *
new0()
    CODE:
	RETVAL = new MultiValueSorter();
    OUTPUT:
	RETVAL

void
MultiValueSorter::add(valueno valno, bool forward = NO_INIT)
    CODE:
	if (items == 3) { /* items includes the hidden this pointer */
	    THIS->add(valno, forward);
	} else {
	    THIS->add(valno);
	}

void
MultiValueSorter::DESTROY()
