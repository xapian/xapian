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

TermIterator *
TermIterator::add_to(number)
    int		number
    CODE:
        THIS->operator++(number);
    OUTPUT:
        THIS

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
TermIterator::skip_to(tname)
    string	tname
    CODE:
        THIS->skip_to(tname);

termcount
TermIterator::get_wdf()
    CODE:
        RETVAL = THIS->get_wdf();
    OUTPUT:
        RETVAL

doccount
TermIterator::get_termfreq()
    CODE:
        RETVAL = THIS->get_termfreq();
    OUTPUT:
        RETVAL

PositionIterator *
TermIterator::positionlist_begin()
    CODE:
        RETVAL = new PositionIterator;
        *RETVAL = THIS->positionlist_begin();
    OUTPUT:
        RETVAL

PositionIterator *
TermIterator::positionlist_end()
    CODE:
        RETVAL = new PositionIterator;
        *RETVAL = THIS->positionlist_end();
    OUTPUT:
        RETVAL

string
TermIterator::get_description()
    CODE:
        RETVAL = THIS->get_description();
    OUTPUT:
        RETVAL
