MODULE = Search::Xapian		PACKAGE = Search::Xapian::Stem

PROTOTYPES: ENABLE

Stem *
Stem::new(language)
    string	language
    CODE:
	try {
	    RETVAL = new Stem(language);
	} catch (const Error &error) {
	    croak( "Exception: %s", error.get_msg().c_str() );
	}
    OUTPUT:
        RETVAL

string
Stem::stem_word(string word)
    CODE:
        RETVAL = (*THIS)(word);
    OUTPUT:
        RETVAL

string
Stem::get_description()

void
Stem::DESTROY()
