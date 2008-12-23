MODULE = Search::Xapian		PACKAGE = Search::Xapian::TermIterator

PROTOTYPES: ENABLE

TermIterator *
new1()
    CODE:
        RETVAL = new TermIterator();
    OUTPUT:
        RETVAL

TermIterator *
new2(other)
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
        RETVAL = new TermIterator(++(*THIS));
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

void
TermIterator::skip_to(string tname)

termcount
TermIterator::get_wdf()

doccount
TermIterator::get_termfreq()

PositionIterator *
TermIterator::positionlist_begin()
    CODE:
        RETVAL = new PositionIterator(THIS->positionlist_begin());
    OUTPUT:
        RETVAL

PositionIterator *
TermIterator::positionlist_end()
    CODE:
        RETVAL = new PositionIterator(THIS->positionlist_end());
    OUTPUT:
        RETVAL

string
TermIterator::get_description()
