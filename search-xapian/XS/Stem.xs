MODULE = Search::Xapian		PACKAGE = Search::Xapian::Stem

PROTOTYPES: ENABLE

Stem *
Stem::new(language)
    string	language
    CODE:
	try {
	    RETVAL = new Stem(language);
	} catch (...) {
	    handle_exception();
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
