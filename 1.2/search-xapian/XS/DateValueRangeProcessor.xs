MODULE = Search::Xapian		PACKAGE = Search::Xapian::DateValueRangeProcessor

PROTOTYPES: ENABLE

DateValueRangeProcessor *
DateValueRangeProcessor::new(valueno valno, bool prefer_mdy = false, int epoch_year = 1970)
    CODE:
	RETVAL = new DateValueRangeProcessor(valno, prefer_mdy, epoch_year);
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
