MODULE = Search::Xapian		PACKAGE = Search::Xapian::BM25Weight

PROTOTYPES: ENABLE

BM25Weight *
BM25Weight::new()
    CODE:
        RETVAL = new BM25Weight();
    OUTPUT:
        RETVAL

 

void
BM25Weight::DESTROY()
