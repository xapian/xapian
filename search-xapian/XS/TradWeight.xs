MODULE = Search::Xapian		PACKAGE = Search::Xapian::TradWeight

PROTOTYPES: ENABLE

TradWeight *
new1()
    CODE:
	RETVAL = new TradWeight();
    OUTPUT:
	RETVAL

TradWeight *
new2(k)
    double	k
    CODE:
	RETVAL = new TradWeight(k);
    OUTPUT:
	RETVAL

void
TradWeight::DESTROY()
