MODULE = Search::Xapian		PACKAGE = Search::Xapian::Stem

PROTOTYPES: ENABLE

Stem *
Stem::new(language)
    string	language
    CODE:
        RETVAL = new Stem(language);
    OUTPUT:
        RETVAL

string
Stem::stem_word(word)
    string	word
    CODE:
        RETVAL = THIS->stem_word(word);
    OUTPUT:
        RETVAL

string
Stem::get_description()
    CODE:
	try {
            RETVAL = THIS->get_description();
        }
        catch (const Error &error) {
            croak( "Exception: %s", error.get_msg().c_str() );
        }
    OUTPUT:
        RETVAL

void
Stem::DESTROY()
