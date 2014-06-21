MODULE = Search::Xapian		PACKAGE = Search::Xapian::ESetIterator

PROTOTYPES: ENABLE

ESetIterator *
new1()
    CODE:
	RETVAL = new ESetIterator();
    OUTPUT:
	RETVAL

ESetIterator *
new2(other)
    ESetIterator *	other
    CODE:
	RETVAL = new ESetIterator(* other);
    OUTPUT:
	RETVAL

void
ESetIterator::inc()
    CODE:
	++(*THIS);

void
ESetIterator::dec()
    CODE:
	--(*THIS);

bool
ESetIterator::equal(ESetIterator * that)
    CODE:
	RETVAL = ((*THIS) == (*that));
    OUTPUT:
	RETVAL

bool
ESetIterator::nequal(ESetIterator * that)
    CODE:
	RETVAL = ((*THIS) != (*that));
    OUTPUT:
	RETVAL

string
ESetIterator::get_termname()
    CODE:
	RETVAL = THIS->operator*();
    OUTPUT:
	RETVAL

weight
ESetIterator::get_weight()

string
ESetIterator::get_description()

void
ESetIterator::DESTROY()
