MODULE = Search::Xapian 	PACKAGE = Search::Xapian::MSetIterator

PROTOTYPES: ENABLE

MSetIterator *
new1();
    CODE:
        RETVAL = new MSetIterator();
    OUTPUT:
        RETVAL

MSetIterator *
new2(other);
    MSetIterator *	other
    CODE:
        RETVAL = new MSetIterator(* other);
    OUTPUT:
        RETVAL

MSetIterator *
MSetIterator::inc()
    CODE:
        RETVAL = new MSetIterator();
        *RETVAL = ++(*THIS);
    OUTPUT:
        RETVAL

MSetIterator *
MSetIterator::add_to(number)
    int		number
    CODE:
        THIS->operator++(number);
    OUTPUT:
        THIS

bool
MSetIterator::equal(that)
    MSetIterator *	that
    CODE:
        RETVAL = ((*THIS) == (*that));
    OUTPUT:
        RETVAL

bool
MSetIterator::nequal(that)
    MSetIterator *	that
    CODE:
        RETVAL = ((*THIS) != (*that));
    OUTPUT:
        RETVAL

docid
MSetIterator::get_docid()
    CODE:
        RETVAL = THIS->operator*();
    OUTPUT:
        RETVAL

Document *
MSetIterator::get_document()
    CODE:
        RETVAL = new Document();
        *RETVAL = THIS->get_document();
    OUTPUT:
        RETVAL

doccount
MSetIterator::get_rank()

weight
MSetIterator::get_weight()

percent
MSetIterator::get_percent()

string
MSetIterator::get_description()

void
MSetIterator::DESTROY()
