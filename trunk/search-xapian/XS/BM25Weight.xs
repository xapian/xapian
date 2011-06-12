MODULE = Search::Xapian		PACKAGE = Search::Xapian::BM25Weight

PROTOTYPES: ENABLE

BM25Weight *
new1()
    CODE:
	try {
	    RETVAL = new BM25Weight();
	} catch (...) {
	    handle_exception();
	}
    OUTPUT:
        RETVAL

BM25Weight *
new2(k1, k2, k3, b, min_normlen)
    double	k1
    double	k2
    double	k3
    double	b
    double	min_normlen
    CODE:
	try {
	    RETVAL = new BM25Weight(k1, k2, k3, b, min_normlen);
	} catch (...) {
	    handle_exception();
	}
    OUTPUT:
	RETVAL

void
BM25Weight::DESTROY()
