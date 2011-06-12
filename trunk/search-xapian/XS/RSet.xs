MODULE = Search::Xapian		PACKAGE = Search::Xapian::RSet

PROTOTYPES: ENABLE

RSet *
new1()
    CODE:
        RETVAL = new RSet();
    OUTPUT:
        RETVAL

RSet *
new2(other)
    RSet *	other
    CODE:
        RETVAL = new RSet(*other);
    OUTPUT:
        RETVAL


doccount
RSet::size()

bool
RSet::empty()

void
RSet::add_document1(it)
    MSetIterator *	it
    CODE:
	THIS->add_document(*it);

void
RSet::add_document2(did)
    docid did
    CODE:
	THIS->add_document(did);

void
RSet::remove_document1(it)
    MSetIterator *	it
    CODE:
	THIS->remove_document(*it);

void
RSet::remove_document2(did)
    docid	did
    CODE:
	THIS->remove_document(did);

bool
RSet::contains1(it)
    MSetIterator *	it
    CODE:
	RETVAL = THIS->contains(*it);
    OUTPUT:
        RETVAL

bool
RSet::contains2(did)
    docid	did
    CODE:
	RETVAL = THIS->contains(did);
    OUTPUT:
        RETVAL

string
RSet::get_description()

void
RSet::DESTROY()
