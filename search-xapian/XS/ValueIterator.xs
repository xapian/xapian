MODULE = Search::Xapian		PACKAGE = Search::Xapian::ValueIterator

PROTOTYPES: ENABLE

ValueIterator *
new1()
    CODE:
        RETVAL = new ValueIterator();
    OUTPUT:
        RETVAL

ValueIterator *
new2(other)
    ValueIterator *	other
    CODE:
        RETVAL = new ValueIterator(*other);
    OUTPUT:
        RETVAL

void
ValueIterator::DESTROY()

void
ValueIterator::inc()
    CODE:
        ++(*THIS);

bool
ValueIterator::equal(that)
    ValueIterator *	that
    CODE:
        RETVAL = ((*THIS) == (*that));
    OUTPUT:
        RETVAL

bool
ValueIterator::nequal(that)
    ValueIterator *	that
    CODE:
        RETVAL = ((*THIS) != (*that));
    OUTPUT:
        RETVAL

string
ValueIterator::get_value()
    CODE:
        RETVAL = THIS->operator*();
    OUTPUT:
        RETVAL

valueno
ValueIterator::get_valueno()

string
ValueIterator::get_description()
