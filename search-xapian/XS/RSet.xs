MODULE = Search::Xapian		PACKAGE = Search::Xapian::RSet

PROTOTYPES: ENABLE

OmRSet *
new1();
    CODE:
        RETVAL = new OmRSet();
    OUTPUT:
        RETVAL

OmRSet *
new2(other);
    OmRSet * other
    CODE:
        RETVAL = new OmRSet(* other);
    OUTPUT:
        RETVAL
 

om_termcount
OmRSet::size()

bool
OmRSet::empty()

void
OmRSet::add_document(om_docid did)

void
OmRSet::remove_document(om_docid did)

bool
OmRSet::contains(om_docid did)

string
OmRSet::get_description()

void
OmRSet::DESTROY()
