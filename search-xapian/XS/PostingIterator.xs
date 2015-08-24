MODULE = Search::Xapian		PACKAGE = Search::Xapian::PostingIterator

PROTOTYPES: ENABLE

PostingIterator *
new1()
    CODE:
	RETVAL = new PostingIterator();
    OUTPUT:
	RETVAL

PostingIterator *
new2(other)
    PostingIterator *	other
    CODE:
	RETVAL = new PostingIterator(*other);
    OUTPUT:
	RETVAL

void
PostingIterator::DESTROY()

void
PostingIterator::inc()
    CODE:
	++(*THIS);

bool
PostingIterator::equal1(that)
    PostingIterator *	that
    CODE:
	RETVAL = ((*THIS) == (*that));
    OUTPUT:
	RETVAL

bool
PostingIterator::nequal1(that)
    PostingIterator *	that
    CODE:
	RETVAL = ((*THIS) != (*that));
    OUTPUT:
	RETVAL

docid
PostingIterator::get_docid()
    CODE:
	RETVAL = THIS->operator*();
    OUTPUT:
	RETVAL

doclength
PostingIterator::get_doclength()
    CODE:
	RETVAL = THIS->get_doclength();
    OUTPUT:
	RETVAL

termcount
PostingIterator::get_wdf()
    CODE:
	RETVAL = THIS->get_wdf();
    OUTPUT:
	RETVAL

string
PostingIterator::get_description()

PositionIterator *
PostingIterator::positionlist_begin()
    CODE:
	RETVAL = new PositionIterator(THIS->positionlist_begin());
    OUTPUT:
	RETVAL

PositionIterator *
PostingIterator::positionlist_end()
    CODE:
	RETVAL = new PositionIterator(THIS->positionlist_end());
    OUTPUT:
	RETVAL

void
PostingIterator::skip_to(docid pos)
