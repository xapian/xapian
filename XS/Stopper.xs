MODULE = Search::Xapian  		PACKAGE = Search::Xapian::Stopper

PROTOTYPES: ENABLE

Stopper *
Stopper::new()
	CODE:
		RETVAL = new Stopper();
	OUTPUT:
		RETVAL

bool
Stopper::stopword(term)
	string term;
	CODE:
		RETVAL = (*THIS)(term);
	OUTPUT:
		RETVAL

void
Stopper::DESTROY()
