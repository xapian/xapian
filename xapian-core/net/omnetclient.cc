/* omnetclient.cc: remote match program
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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

#include <iostream>
#include <iomanip>
#include <string>
#include <typeinfo>
#include <memory>
#include <algorithm>
#include "database.h"
#include "database_builder.h"
#include <om/omerror.h>
#include <om/omenquire.h>
#include "leafmatch.h"
#include "omqueryinternal.h"
#include "readquery.h"

void run_matcher();

int main() {
    string message;
    getline(cin, message);
    cerr << "omnetclient: read " << message << endl;
    cout << "BOO!" << endl;
    cout.flush();

    try {
	run_matcher();
    } catch (OmError &e) {
	cerr << "OmError exception (" << typeid(e).name()
	     << "): " << e.get_msg() << endl;
    } catch (...) {
	cerr << "Caught exception" << endl;
    }
}

void split_words(string text, vector<string> &words) {
    if (text.length() > 0 && text[0] == ' ') {
	text.erase(0, text.find_first_not_of(' '));
    }
    while (text.length() > 0) {
	words.push_back(text.substr(0, text.find_first_of(' ')));
	text.erase(0, text.find_first_of(' '));
	text.erase(0, text.find_first_not_of(' '));
    }
}

/** The query parsing functions...  (interface is query_from_string,
 *  the qfs_* functions are "internal".
 */

// A vector converter: vector<OmQueryInternal> to vector<OmQueryInternal *>
// The original vector is still responsible for destroying the objects.
vector<OmQueryInternal *>
convert_subqs(vector<OmQueryInternal> &v) {
    vector<OmQueryInternal *> result;
    for (vector<OmQueryInternal>::iterator i=v.begin();
	 i != v.end();
	 ++i) {
	result.push_back(&(*i));
    }
    return result;
}

// read a compound term
OmQueryInternal qfs_readcompound()
{
    querytok qt;
    vector<OmQueryInternal> subqs;
    while(1) {
	qt = qfs_gettok();
	switch (qt.type) {
	    case querytok::OP_KET:
		if (subqs.size() == 0) {
		    return OmQueryInternal();
		} else {
		    throw OmInvalidArgumentError("Invalid query string");
		}
		break;
	    case querytok::NULL_QUERY:
		subqs.push_back(OmQueryInternal());
		break;
	    case querytok::TERM:
		subqs.push_back(OmQueryInternal(qt.tname,
						qt.wqf,
						qt.term_pos));
		break;
	    case querytok::OP_BRA:
		subqs.push_back(qfs_readcompound());
		break;
	    case querytok::OP_AND:
		{
		    vector<OmQueryInternal *> temp =
			    convert_subqs(subqs);
		    querytok myqt = qfs_gettok();
		    if (myqt.type != querytok::OP_KET) {
			throw OmInvalidArgumentError("Expected %) in query string");
		    }
		    return OmQueryInternal(OM_MOP_AND,
					   temp.begin(),
					   temp.end());
		}
		break;
	    case querytok::OP_OR:
		{
		    vector<OmQueryInternal *> temp =
			    convert_subqs(subqs);
		    querytok myqt = qfs_gettok();
		    if (myqt.type != querytok::OP_KET) {
			throw OmInvalidArgumentError("Expected %) in query string");
		    }
		    return OmQueryInternal(OM_MOP_OR,
					   temp.begin(),
					   temp.end());
		}
		break;
	    case querytok::OP_FILTER:
		{
		    vector<OmQueryInternal *> temp =
			    convert_subqs(subqs);
		    querytok myqt = qfs_gettok();
		    if (myqt.type != querytok::OP_KET) {
			throw OmInvalidArgumentError("Expected %) in query string");
		    }
		    return OmQueryInternal(OM_MOP_FILTER,
					   temp.begin(),
					   temp.end());
		}
		break;
	    case querytok::OP_XOR:
		{
		    vector<OmQueryInternal *> temp =
			    convert_subqs(subqs);
		    querytok myqt = qfs_gettok();
		    if (myqt.type != querytok::OP_KET) {
			throw OmInvalidArgumentError("Expected %) in query string");
		    }
		    return OmQueryInternal(OM_MOP_XOR,
					   temp.begin(),
					   temp.end());
		}
		break;
	    case querytok::OP_ANDMAYBE:
		{
		    vector<OmQueryInternal *> temp =
			    convert_subqs(subqs);
		    querytok myqt = qfs_gettok();
		    if (myqt.type != querytok::OP_KET) {
			throw OmInvalidArgumentError("Expected %) in query string");
		    }
		    return OmQueryInternal(OM_MOP_AND_MAYBE,
					   temp.begin(),
					   temp.end());
		}
		break;
	    case querytok::OP_ANDNOT:
		{
		    vector<OmQueryInternal *> temp =
			    convert_subqs(subqs);
		    querytok myqt = qfs_gettok();
		    if (myqt.type != querytok::OP_KET) {
			throw OmInvalidArgumentError("Expected %) in query string");
		    }
		    return OmQueryInternal(OM_MOP_AND_NOT,
					   temp.begin(),
					   temp.end());
		}
		break;
	    default:
		throw OmInvalidArgumentError("Invalid query string");
	} // switch(qt.type)
    } // while(1)
}

/// Read a whole query
OmQueryInternal qfs_readquery()
{
    querytok qt = qfs_gettok();
    switch (qt.type) {
	case querytok::NULL_QUERY:  // null query
	    return OmQueryInternal();
	    break;
	case querytok::TERM:
	    return OmQueryInternal(qt.tname, qt.wqf, qt.term_pos);
	    break;
	case querytok::OP_BRA:
	    return qfs_readcompound();
	    break;
	default:
	    cout << "Got type " << int(qt.type) << endl;
	    Assert(false);
    }
    throw OmInvalidArgumentError("Invalid query string");
}

OmQueryInternal query_from_string(string qs)
{
    Assert(qs.length() > 1);

    qfs_start(qs);
    
//    cerr << "Reading from query " << qs << endl;
    OmQueryInternal retval = qfs_readquery();
    Assert(retval.serialise() == qs);

    qfs_end();

    return retval;
}

void run_matcher() {
    // open the database to return results
    DatabaseBuilderParams param(OM_DBTYPE_INMEMORY);
    param.paths.push_back("text.txt");
    auto_ptr<IRDatabase> db(DatabaseBuilder::create(param));

    LeafMatch leafmatch(db.get());

    while (1) {
	string message;
	getline(cin, message);
	vector<string> words;

	split_words(message, words);

	if (words.empty()) {
	    break;
	};
	if (words[0] == "GETDOCCOUNT") {
	    cout << db->get_doccount() << endl;
	    cout.flush();
	} else if (words.size() == 2 && words[0] == "SETWEIGHT") {
	    //cerr << "responding to SETWEIGHT" << endl;
	    IRWeight::weight_type wt_type = 
		    static_cast<IRWeight::weight_type>(
		    atol(words[1].c_str()));
	    leafmatch.set_weighting(wt_type);
	    cout << "OK" << endl;
	    cout.flush();
	} else if (words[0] == "SETQUERY") {
	    OmQueryInternal temp =
		    query_from_string(message.substr(9,
						     message.npos));
	    leafmatch.set_query(&temp);
	    //cerr << "CLIENT QUERY: " << temp.serialise() << endl;
	    cout << "OK" << endl;
	    cout.flush();
	} else {
	    cout << "ERROR" << endl;
	    cout.flush();
	}
    }
}
