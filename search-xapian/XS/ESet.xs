MODULE = Search::Xapian		PACKAGE = Search::Xapian::ESet

PROTOTYPES: ENABLE

ESet *
new1();
    CODE:
        RETVAL = new ESet();
    OUTPUT:
        RETVAL

ESet *
new2(other);
    ESet *	other
    CODE:
        RETVAL = new ESet(*other);
    OUTPUT:
        RETVAL
 

termcount
ESet::get_ebound()

termcount
ESet::size()

bool
ESet::empty()

ESetIterator *
ESet::begin()
    CODE:
        RETVAL = new ESetIterator();
        *RETVAL = THIS->begin();
    OUTPUT:
        RETVAL

ESetIterator *
ESet::end()
    CODE:
        RETVAL = new ESetIterator();
        *RETVAL = THIS->end();
    OUTPUT:
        RETVAL

string
ESet::get_description()

void
ESet::DESTROY()
