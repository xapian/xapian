MODULE = Search::Xapian		PACKAGE = Search::Xapian::BoolWeight

PROTOTYPES: ENABLE

BoolWeight *
new1()
    CODE:
	try {
	    RETVAL = new BoolWeight();
	} catch (...) {
	    handle_exception();
	}
    OUTPUT:
	RETVAL

void
BoolWeight::DESTROY()
