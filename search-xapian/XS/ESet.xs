MODULE = Search::Xapian		PACKAGE = Search::Xapian::ESet

PROTOTYPES: ENABLE

ESet *
new1()
    CODE:
        RETVAL = new ESet();
    OUTPUT:
        RETVAL

ESet *
new2(other)
    ESet *	other
    CODE:
        RETVAL = new ESet(*other);
    OUTPUT:
        RETVAL


termcount
ESet::get_ebound()

termcount
ESet::size()
    ALIAS:
	Search::Xapian::ESet::FETCHSIZE = 1

bool
ESet::empty()

ESetIterator *
ESet::begin()
    CODE:
        RETVAL = new ESetIterator(THIS->begin());
    OUTPUT:
        RETVAL

ESetIterator *
ESet::end()
    CODE:
        RETVAL = new ESetIterator(THIS->end());
    OUTPUT:
        RETVAL

ESetIterator *
ESet::back()
    CODE:
        RETVAL = new ESetIterator(THIS->back());
    OUTPUT:
        RETVAL

ESetIterator *
ESet::FETCH(doccount i)
    CODE:
        RETVAL = new ESetIterator((*THIS)[i]);
    OUTPUT:
        RETVAL

string
ESet::get_description()

void
ESet::DESTROY()
