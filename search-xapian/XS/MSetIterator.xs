MODULE = Search::Xapian 	PACKAGE = Search::Xapian::MSetIterator

PROTOTYPES: ENABLE

OmMSetIterator *
new1();
    CODE:
        RETVAL = new OmMSetIterator();
    OUTPUT:
        RETVAL

OmMSetIterator *
new2(other);
    OmMSetIterator * other
    CODE:
        RETVAL = new OmMSetIterator(* other);
    OUTPUT:
        RETVAL

OmMSetIterator *
OmMSetIterator::inc()
    CODE:
        RETVAL = new OmMSetIterator();
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

om_docid
OmMSetIterator::get_docid()
    CODE:
        RETVAL = THIS->operator*();
    OUTPUT:
        RETVAL

OmDocument *
OmMSetIterator::get_document()
    CODE:
        RETVAL = new OmDocument();
        *RETVAL = THIS->get_document();
    OUTPUT:
        RETVAL

om_doccount
OmMSetIterator::get_rank()

om_weight
OmMSetIterator::get_weight()

om_percent
OmMSetIterator::get_percent()

string
OmMSetIterator::get_description()

void
OmMSetIterator::DESTROY()
