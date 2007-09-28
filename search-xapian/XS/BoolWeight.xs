MODULE = Search::Xapian		PACKAGE = Search::Xapian::BoolWeight

PROTOTYPES: ENABLE

BoolWeight *
BoolWeight::new()
    CODE:
	// CLASS isn't used because the typemap for O_WEIGHT means that
	// the object is blessed as Search::Xapian::Weight, so cast to
	// void to avoid "unused variable" warning.
	(void)CLASS;
	RETVAL = new BoolWeight();
    OUTPUT:
	RETVAL

void
BoolWeight::DESTROY()
