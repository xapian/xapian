MODULE = Search::Xapian		PACKAGE = Search::Xapian::PositionIterator

PROTOTYPES: ENABLE

PositionIterator *
new1();
    CODE:
        RETVAL = new PositionIterator();
    OUTPUT:
        RETVAL

PositionIterator *
new2(other);
    PositionIterator *	other
    CODE:
        RETVAL = new PositionIterator(*other);
    OUTPUT:
        RETVAL

void
PositionIterator::DESTROY()
  
PositionIterator *
PositionIterator::inc()
    CODE:
        RETVAL = new PositionIterator();
        *RETVAL = ++(*THIS);
    OUTPUT:
        RETVAL

PositionIterator *
PositionIterator::add_to(number)
    int		number
    CODE:
        THIS->operator++(number);
    OUTPUT:
        THIS

bool
PositionIterator::equal(that)
    PositionIterator *	that
    CODE:
        RETVAL = ((*THIS) == (*that));
    OUTPUT:
        RETVAL

bool
PositionIterator::nequal(that)
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
PositionIterator::skip_to(pos)
    termpos	pos
    CODE:
        THIS->skip_to(pos);
