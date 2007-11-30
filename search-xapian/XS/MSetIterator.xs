MODULE = Search::Xapian		PACKAGE = Search::Xapian::MSetIterator

PROTOTYPES: ENABLE

MSetIterator *
new1()
    CODE:
        RETVAL = new MSetIterator();
    OUTPUT:
        RETVAL

MSetIterator *
new2(other)
    MSetIterator *	other
    CODE:
        RETVAL = new MSetIterator(* other);
    OUTPUT:
        RETVAL

void
MSetIterator::inc()
    CODE:
        ++(*THIS);

void
MSetIterator::dec()
    CODE:
        --(*THIS);

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
        RETVAL = new Document(THIS->get_document());
    OUTPUT:
        RETVAL

doccount
MSetIterator::get_rank()

weight
MSetIterator::get_weight()

doccount
MSetIterator::get_collapse_count()

percent
MSetIterator::get_percent()

string
MSetIterator::get_description()

void
MSetIterator::DESTROY()
