MODULE = Search::Xapian		PACKAGE = Search::Xapian::TermIterator

PROTOTYPES: ENABLE

TermIterator *
new1();
    CODE:
        RETVAL = new TermIterator();
    OUTPUT:
        RETVAL

TermIterator *
new2(other);
    TermIterator *	other
    CODE:
        RETVAL = new TermIterator(*other);
    OUTPUT:
        RETVAL
  
void
TermIterator::DESTROY()

TermIterator *
TermIterator::inc()
    CODE:
        RETVAL = new TermIterator();
        *RETVAL = ++(*THIS);
    OUTPUT:
        RETVAL

bool
TermIterator::equal(that)
    TermIterator *	that
    CODE:
        RETVAL = ((*THIS) == (*that));
    OUTPUT:
        RETVAL

bool
TermIterator::nequal(that)
    TermIterator *	that
    CODE:
        RETVAL = ((*THIS) != (*that));
    OUTPUT:
        RETVAL

string
TermIterator::get_termname()
    CODE:
        RETVAL = THIS->operator*();
    OUTPUT:
        RETVAL

string
TermIterator::get_description()
    CODE:
        RETVAL = THIS->get_description();
    OUTPUT:
        RETVAL
