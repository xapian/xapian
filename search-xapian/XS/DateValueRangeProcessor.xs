MODULE = Search::Xapian		PACKAGE = Search::Xapian::DateValueRangeProcessor

PROTOTYPES: ENABLE

DateValueRangeProcessor *
new(const char * CLASS, valueno valno)
    CODE:
	RETVAL = new DateValueRangeProcessor(valno);
    OUTPUT:
	RETVAL

valueno
DateValueRangeProcessor::process_value_range(begin, end)
	string & begin;
	string & end;
	CODE:
		RETVAL = (*THIS)(begin, end);
	OUTPUT:
		RETVAL

void
DateValueRangeProcessor::DESTROY()
