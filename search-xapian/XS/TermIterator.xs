MODULE = Search::Xapian		PACKAGE = Search::Xapian::TermIterator

PROTOTYPES: ENABLE

OmTermIterator *
OmTermIterator::new()

void
OmTermIterator::DESTROY()


MODULE = Search::Xapian		PACKAGE = Search::Xapian::Stem		

PROTOTYPES: ENABLE

OmStem *
OmStem::new(language)
    string *     language
    CODE:
        RETVAL = new OmStem(*language);
    OUTPUT:
        RETVAL

string
OmStem::stem_word(word)
    string *     word
    CODE:
        RETVAL = THIS->stem_word(*word);
    OUTPUT:
        RETVAL

void
OmStem::DESTROY()