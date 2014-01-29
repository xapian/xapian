MODULE = Search::Xapian  		PACKAGE = Search::Xapian::SimpleStopper

PROTOTYPES: ENABLE

SimpleStopper *
new0()
    CODE:
	RETVAL = new SimpleStopper();
    OUTPUT:
	RETVAL

bool
SimpleStopper::stop_word(term)
    string term;
    CODE:
	RETVAL = (*THIS)(term);
    OUTPUT:
	RETVAL

void
SimpleStopper::add(string term)

void
SimpleStopper::DESTROY()
