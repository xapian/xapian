MODULE = Search::Xapian		PACKAGE = Search::Xapian::RSet

PROTOTYPES: ENABLE

RSet *
new1();
    CODE:
        RETVAL = new RSet();
    OUTPUT:
        RETVAL

RSet *
new2(other);
    RSet *	other
    CODE:
        RETVAL = new RSet(*other);
    OUTPUT:
        RETVAL
 

termcount
RSet::size()

bool
RSet::empty()

void
RSet::add_document(docid did)

void
RSet::remove_document(docid did)

bool
RSet::contains(docid did)

string
RSet::get_description()

void
RSet::DESTROY()
