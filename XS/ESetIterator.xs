MODULE = Search::Xapian 	PACKAGE = Search::Xapian::ESetIterator

PROTOTYPES: ENABLE

OmESetIterator *
new1();
    CODE:
        RETVAL = new OmESetIterator();
    OUTPUT:
        RETVAL

OmESetIterator *
new2(other);
    OmESetIterator * other
    CODE:
        RETVAL = new OmESetIterator(* other);
    OUTPUT:
        RETVAL

OmESetIterator *
OmESetIterator::inc()
    CODE:
        RETVAL = new OmESetIterator();
        *RETVAL = ++(*THIS);
    OUTPUT:
        RETVAL

bool
OmMSetIterator::equal(OmMSetIterator * that)
    CODE:
        RETVAL = ((*THIS) == (*that));
    OUTPUT:
        RETVAL

bool
OmMSetIterator::nequal(OmMSetIterator * that)
    CODE:
        RETVAL = ((*THIS) != (*that));
    OUTPUT:
        RETVAL

string
OmESetIterator::get_termname()
    CODE:
        RETVAL = THIS->operator*();
    OUTPUT:
        RETVAL

om_weight
OmESetIterator::get_weight()

string
OmESetIterator::get_description()

void
OmESetIterator::DESTROY()
