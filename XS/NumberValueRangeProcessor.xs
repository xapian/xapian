MODULE = Search::Xapian		PACKAGE = Search::Xapian::NumberValueRangeProcessor

PROTOTYPES: ENABLE

NumberValueRangeProcessor *
new(const char * CLASS, valueno valno)
    CODE:
	RETVAL = new NumberValueRangeProcessor(valno);
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
