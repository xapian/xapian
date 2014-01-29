#include <xapian.h>

extern "C" {
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
}

MODULE = SymbolTest	PACKAGE = SymbolTest

PROTOTYPES: ENABLE

void
throw_from_libxapian()
    CODE:
	try {
	    Xapian::WritableDatabase db("/dev/null", Xapian::DB_CREATE_OR_OPEN);
	} catch (const Xapian::Error & error) {
	    croak("%s caught in SymbolTest", error.get_type());
	} catch (...) {
	    croak("Unknown C++ exception caught in SymbolTest");
	}
