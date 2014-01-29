MODULE = Search::Xapian		PACKAGE = Search::Xapian::NumberValueRangeProcessor

PROTOTYPES: ENABLE

NumberValueRangeProcessor *
NumberValueRangeProcessor::new(valueno valno, string str = NO_INIT, bool prefix = true)
    CODE:
	if (items == 2) {
	    RETVAL = new NumberValueRangeProcessor(valno);
	} else {
	    RETVAL = new NumberValueRangeProcessor(valno, str, prefix);
	}
    OUTPUT:
	RETVAL

valueno
NumberValueRangeProcessor::process_value_range(begin, end)
	string & begin;
	string & end;
	CODE:
		RETVAL = (*THIS)(begin, end);
	OUTPUT:
		RETVAL

void
NumberValueRangeProcessor::DESTROY()
