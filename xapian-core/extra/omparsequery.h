/* parsequery.h: parser for omega query strings
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003 Olly Betts
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#ifndef XAPIAN_INCLUDED_PARSEQUERY_H
#define XAPIAN_INCLUDED_PARSEQUERY_H

#include <xapian.h>

#include <list>
#include <map>
#include <string>

class OmStopper {
    public:
	virtual bool operator()(const std::string &/*term*/) {
	    return false;
	}
};

class OmQueryParser {
    private:
	// Prevent copying
	OmQueryParser(const OmQueryParser &);
	OmQueryParser & operator=(const OmQueryParser &);
    
    public:
	OmQueryParser() : default_op(Xapian::Query::OP_OR), stop(NULL), stemmer(NULL),
		stem(true), stem_all(false)
	{
	    set_stemming_options("english");
	}
	
	void set_stemming_options(const std::string &lang,
				  bool stem_all_ = false,
				  OmStopper *stop_ = NULL);
	
	void set_default_op(Xapian::Query::op default_op_) {
	    default_op = default_op_;
	}

	void set_database(const Xapian::Database &db_) {
	    db = db_;
	}

	Xapian::Query parse_query(const std::string &q);
	
	std::list<std::string> termlist;
	std::list<std::string> stoplist;

	std::multimap<std::string, std::string> unstem;

	// Map "from" -> "A" ; "subject" -> "C" ; "newsgroups" -> "G" ;
	// "foobar" -> "XFOO"
	std::map<std::string, std::string> prefixes;

	// don't touch these - FIXME: make private and use friend...
	Xapian::Query::op default_op;

	OmStopper *stop;

	Xapian::Stem *stemmer;

	bool stem, stem_all;

	Xapian::Database db;
};

#endif /* XAPIAN_INCLUDED_PARSEQUERY_H */
