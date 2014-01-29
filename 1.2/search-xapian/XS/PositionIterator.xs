MODULE = Search::Xapian		PACKAGE = Search::Xapian::PositionIterator

PROTOTYPES: ENABLE

PositionIterator *
new1()
    CODE:
        RETVAL = new PositionIterator();
    OUTPUT:
        RETVAL

PositionIterator *
new2(other)
    PositionIterator *	other
    CODE:
        RETVAL = new PositionIterator(*other);
    OUTPUT:
        RETVAL

void
PositionIterator::DESTROY()

void
PositionIterator::inc()
    CODE:
        ++(*THIS);

bool
PositionIterator::equal1(that)
    PositionIterator *	that
    CODE:
        RETVAL = ((*THIS) == (*that));
    OUTPUT:
        RETVAL

bool
PositionIterator::nequal1(that)
    PositionIterator *	that
    CODE:
        RETVAL = ((*THIS) != (*that));
    OUTPUT:
        RETVAL

termpos
PositionIterator::get_termpos()
    CODE:
        RETVAL = THIS->operator*();
    OUTPUT:
        RETVAL

void
PositionIterator::skip_to(termpos pos)

string
PositionIterator::get_description()
