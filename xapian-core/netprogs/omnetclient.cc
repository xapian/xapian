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
#include <strstream.h>
#include <iomanip.h>
#include "database.h"
#include "database_builder.h"
#include <om/omerror.h>
#include <om/omenquire.h>
#include "localmatch.h"
#include "omqueryinternal.h"
#include "readquery.h"
#include "netutils.h"

void run_matcher(const char *filename);

int main(int argc, char *argv[]) {
    string message;
    getline(cin, message);
    cerr << "omnetclient: read " << message << endl;
    cout << "BOO!" << endl;
    cout.flush();

    if (argc != 2) {
	cerr << "Wrong number of arguments" << endl;
	cout << "ERROR" << endl;
	exit(-1);
    }

    try {
	run_matcher(argv[1]);
    } catch (OmError &e) {
	cerr << "OmError exception (" << typeid(e).name()
	     << "): " << e.get_msg() << endl;
    } catch (...) {
	cerr << "Caught exception" << endl;
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

string
stats_to_string(const Stats &stats)
{
    ostrstream os;

    os << stats.collection_size << " ";
    os << stats.average_length << " ";

    map<om_termname, om_doccount>::const_iterator i;

    for (i=stats.termfreq.begin();
	 i != stats.termfreq.end();
	 ++i) {
	os << "T" << encode_tname(i->first) << "=" << i->second << " ";
    }

    for (i = stats.reltermfreq.begin();
	 i != stats.reltermfreq.end();
	 ++i) {
	os << "R" << encode_tname(i->first) << "=" << i->second << " ";
    }

    // FIXME: should be eos.
    os << '\0';

    string result(os.str());

    os.freeze(0);

    return result;
}

void run_matcher(const char *filename) {
    // open the database to return results
    DatabaseBuilderParams param(OM_DBTYPE_INMEMORY);
    param.paths.push_back(filename);
    auto_ptr<IRDatabase> db(DatabaseBuilder::create(param));

    StatsGatherer statgath;

    LocalMatch leafmatch(db.get());
    leafmatch.link_to_multi(&statgath);

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
	} else if (words[0] == "GETSTATS") {
	    // FIXME: we're not using the RSet stats yet.
	    leafmatch.prepare_match();

	    cout << stats_to_string(*statgath.get_stats()) << endl;
	    cout.flush();
	} else if (words[0] == "GET_MSET") {
	    //cerr << "GET_MSET: " << words.size() << " words" << endl;
	    if (words.size() != 3) {
		cout << "ERROR" << endl;
		cout.flush();
	    } else {
		om_doccount first = atoi(words[1].c_str());
		om_doccount maxitems = atoi(words[2].c_str());

		vector<OmMSetItem> mset;
		om_doccount mbound;
		om_weight greatest_wt;

		cerr << "About to get_mset(" << first
			<< ", " << maxitems << "..." << endl;

		leafmatch.get_mset(first,
				   maxitems,
				   mset,
				   msetcmp_forward,
				   &mbound,
				   &greatest_wt,
				   0);

		//cerr << "done get_mset..." << endl;

		cout << mset.size() << endl;
		cout.flush();

		//cerr << "sent size..." << endl;

		for (vector<OmMSetItem>::iterator i=mset.begin();
		     i != mset.end();
		     ++i) {
		    cout << i->wt << " " << i->did << endl;
		    cout.flush();

		    cerr << "MSETITEM: " << i->wt << " " << i->did << endl;
		}
		//cerr << "sent items..." << endl;

		cout << "OK" << endl;
		cout.flush();

		//cerr << "sent OK..." << endl;
	    }
	} else if (words[0] == "GETMAXWEIGHT") {
	    cout << leafmatch.get_max_weight() << endl;
	    cout.flush();
	} else {
	    cout << "ERROR" << endl;
	    cout.flush();
	}
    }
}
