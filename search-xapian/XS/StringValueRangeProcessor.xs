MODULE = Search::Xapian		PACKAGE = Search::Xapian::StringValueRangeProcessor

PROTOTYPES: ENABLE

StringValueRangeProcessor *
StringValueRangeProcessor::new(valno, str="", prefix = true)
    valueno valno
    string str
    bool prefix
    CODE:
        switch (items) { /* items includes the hidden this pointer */
        case 2:
            RETVAL = new StringValueRangeProcessor(valno);
            break;
        case 3:
        case 4: {
            RETVAL = new StringValueRangeProcessor(valno, str, prefix);
            break;
        }
        default:
            croak("Bad parameter count for new");
        }
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
