MODULE = Search::Xapian  		PACKAGE = Search::Xapian::QueryParser

PROTOTYPES: ENABLE

QueryParser *
new0()
	CODE:
		RETVAL = new QueryParser();
	OUTPUT:
		RETVAL

void
QueryParser::set_stemming_options(lang,stem_all)
	string lang
	bool stem_all
    CODE:
		Xapian::Stopper *stopper = new Xapian::Stopper();
        THIS->set_stemming_options(lang,stem_all,stopper);

void
QueryParser::set_default_op(op)
	int op
	CODE:
		THIS->set_default_op( (Query::op) op );

void
QueryParser::set_database(database)
	Database * database
	CODE:
		THIS->set_database( *database );

Query *
QueryParser::parse_query(q)
	string q
	CODE:
		RETVAL = new Query();
		*RETVAL = THIS->parse_query(q);
	OUTPUT:
		RETVAL

void
QueryParser::DESTROY()
