MODULE = Search::Xapian		PACKAGE = Search::Xapian::ValueCountMatchSpy

PROTOTYPES: ENABLE

ValueCountMatchSpy *
new1()
    CODE:
        RETVAL = new ValueCountMatchSpy();
    OUTPUT:
        RETVAL

ValueCountMatchSpy *
new2(slot)
    valueno slot
    CODE:
	RETVAL = new ValueCountMatchSpy(slot);
    OUTPUT:
	RETVAL

void
ValueCountMatchSpy::DESTROY()

size_t
ValueCountMatchSpy::get_total()
    CODE:
        try {
            RETVAL = THIS->get_total();
        } catch (...) {
            handle_exception();
        }
    OUTPUT:
        RETVAL

TermIterator *
ValueCountMatchSpy::values_begin()
    CODE:
      try {
          RETVAL = new TermIterator(THIS->values_begin());
      } catch (...) {
          handle_exception();
      }
    OUTPUT:
        RETVAL

TermIterator *
ValueCountMatchSpy::values_end()
    CODE:
      try {
          RETVAL = new TermIterator(THIS->values_end());
      } catch (...) {
          handle_exception();
      }
    OUTPUT:
        RETVAL

TermIterator *
ValueCountMatchSpy::top_values_begin(maxvalues)
    size_t maxvalues
    CODE:
      try {
          RETVAL = new TermIterator(THIS->top_values_begin(maxvalues));
      } catch (...) {
          handle_exception();
      }
    OUTPUT:
        RETVAL

TermIterator *
ValueCountMatchSpy::top_values_end(maxvalues)
    size_t maxvalues
    CODE:
      try {
          RETVAL = new TermIterator(THIS->top_values_end(maxvalues));
      } catch (...) {
          handle_exception();
      }
    OUTPUT:
        RETVAL

