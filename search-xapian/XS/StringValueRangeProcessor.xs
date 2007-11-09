MODULE = Search::Xapian		PACKAGE = Search::Xapian::StringValueRangeProcessor

PROTOTYPES: ENABLE

StringValueRangeProcessor *
StringValueRangeProcessor::new(valueno valno)
    CODE:
	RETVAL = new StringValueRangeProcessor(valno);
    OUTPUT:
	RETVAL

valueno
StringValueRangeProcessor::process_value_range(begin, end)
	string & begin;
	string & end;
	CODE:
		RETVAL = (*THIS)(begin, end);
	OUTPUT:
		RETVAL

void
StringValueRangeProcessor::DESTROY()
