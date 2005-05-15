MODULE = Search::Xapian  		PACKAGE = Search::Xapian::Stopper

PROTOTYPES: ENABLE

bool
Stopper::stop_word(term)
	string term;
	CODE:
		RETVAL = (*THIS)(term);
	OUTPUT:
		RETVAL

void
Stopper::DESTROY()
