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
 *  the qfs_* functions are internal.  Error checking is currently
 *  minimal.
 */

char hextochar(char high, char low)
{
    int h;
    if (high >= '0' && high <= '9') {
	h = high - '0';
    } else {
	high = toupper(high);
	if (high <= 'F' && high >= 'A') {
	    h = high - 'A' + 10;
	} else {
	    Assert(false);
	}
    }
    if (low >= '0' && low <= '9') {
	h = low - '0';
    } else {
	low = toupper(low);
	if (low <= 'F' && low >= 'A') {
	    h = low - 'A' + 10;
	} else {
	    Assert(false);
	}
    }
    return low + (high << 4);
}

/// Read a term, from after the %T
OmQueryInternal *qfs_readterm(string::const_iterator &i,
			      string::const_iterator end)
{
    string tname;
    // read termname
    while (*i != ',') {
	char high = *i++;
	char low  = *i++;
	tname += hextochar(high, low);
    }
    ++i; // skip comma

    // read wqf
    om_termcount wqf = 0;
    while (*i != ',') {
	wqf *= 10;
	wqf += (*i++) - '0';
    }

    // read termpos
    om_termpos termpos = 0;
    while (i != end && isdigit(*i)) {
	termpos *= 10;
	termpos += (*i++) - '0';
    }

    return new OmQueryInternal(tname, wqf, termpos);
}

OmQueryInternal *qfs_readquery(string::const_iterator &i,
			       string::const_iterator end);

string qfs_readword(string::const_iterator &i,
		    string::const_iterator end)
{
    string result;
    while (i != end && (isalpha(*i))) {
	result += *i;
	++i;
    }
    return result;
}
// read a compound term, just after the %(
// FIXME: leaks like a sieve
OmQueryInternal *qfs_readcompound(string::const_iterator &i,
			       string::const_iterator end)
{
    vector<OmQueryInternal *> subqs;
    while ((*i == ' ') || (*i == '%')) {
	++i;
    };
    if (*i == ')') {
	++i;
	return new OmQueryInternal();
    }

    OmQueryInternal *temp;
    while ((temp = qfs_readquery(i, end))) {
	subqs.push_back(temp);
    }
    string op_string = qfs_readword(i, end);
    if (op_string == "and") {
	return new OmQueryInternal(OM_MOP_AND,
			       subqs.begin(),
			       subqs.end());
    } else if (op_string == "or") {
	return new OmQueryInternal(OM_MOP_OR,
			       subqs.begin(),
			       subqs.end());
    } else if (op_string == "andmaybe") {
	return new OmQueryInternal(OM_MOP_AND_MAYBE,
			       *subqs[0],
			       *subqs[1]);
    } else if (op_string == "andnot") {
	return new OmQueryInternal(OM_MOP_AND_NOT,
			       *subqs[0],
			       *subqs[1]);
    } else if (op_string == "xor") {
	return new OmQueryInternal(OM_MOP_XOR,
			       *subqs[0],
			       *subqs[1]);
    } else if (op_string == "filter") {
	return new OmQueryInternal(OM_MOP_FILTER,
			       *subqs[0],
			       *subqs[1]);
    } else {
	return 0;
    }
}

/// Read a whole query, from just after the %.
OmQueryInternal *qfs_readquery(string::const_iterator &i,
			       string::const_iterator end)
{
    switch (*i) {
	case 'N':  // null query
	    ++i;
	    return new OmQueryInternal();
	    break;
	case 'T':
	    ++i;
	    return qfs_readterm(i, end);
	    break;
	case '(':
	    ++i;
	    return qfs_readcompound(i, end);
	    break;
	case 'a': // and, andor, andmaybe
	case 'f': // filter
	case 'o': // or
	case 'x': // xor
	    // not for us to look at, return
	    return 0;
	default:
	    Assert(false);
    }
    return 0;
}

OmQueryInternal *query_from_string(string qs)
{
    Assert(qs.length() > 1);

    string::const_iterator i = qs.begin();
    Assert(*i == '%');
    ++i;
    
    return qfs_readquery(i, qs.end());
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
	    cerr << "responding to SETWEIGHT" << endl;
	    IRWeight::weight_type wt_type = 
		    static_cast<IRWeight::weight_type>(
		    atol(words[1].c_str()));
	    leafmatch.set_weighting(wt_type);
	    cout << "OK" << endl;
	    cout.flush();
	} else if (words[0] == "SETQUERY") {
	    OmQueryInternal *temp =
		    query_from_string(message.substr(9,
						     message.npos));
	    leafmatch.set_query(temp);
	    cerr << "CLIENT QUERY: " << temp->serialise() << endl;
	    cout << "OK" << endl;
	    cout.flush();
	} else {
	    cout << "ERROR" << endl;
	    cout.flush();
	}
    }
}
