MODULE = Search::Xapian		PACKAGE = Search::Xapian::ESet

PROTOTYPES: ENABLE

OmESet *
new1();
    CODE:
        RETVAL = new OmESet();
    OUTPUT:
        RETVAL

OmESet *
new2(other);
    OmESet * other
    CODE:
        RETVAL = new OmESet(* other);
    OUTPUT:
        RETVAL
 

om_termcount
OmESet::get_ebound()

om_termcount
OmESet::size()

bool
OmESet::empty()

OmESetIterator *
OmESet::begin()
    CODE:
        RETVAL = new OmESetIterator();
        *RETVAL = THIS->begin();
    OUTPUT:
        RETVAL

OmESetIterator *
OmESet::end()
    CODE:
        RETVAL = new OmESetIterator();
        *RETVAL = THIS->end();
    OUTPUT:
        RETVAL

string
OmESet::get_description()

void
OmESet::DESTROY()
