/* progcommon.cc: Various useful Prog{Server,Client}-related utilities
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

#include <string>
#include <vector>
#include <strstream.h>
#include <unistd.h>
#include <cerrno>
#include "progcommon.h"
#include "omassert.h"
#include "omqueryinternal.h"
#include "readquery.h"
#include "stats.h"

OmQueryInternal qfs_readcompound();

OmQueryInternal query_from_string(string qs)
{
    Assert(qs.length() > 1);

    qfs_start(qs);
    
//    cerr << "Reading from query " << qs << endl;
    OmQueryInternal retval = qfs_readquery();
    DebugMsg("query_from_string(" << qs << ") = " << retval.serialise());
    Assert(retval.serialise() == qs);

    qfs_end();

    return retval;
}

string stats_to_string(const Stats &stats)
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

Stats
string_to_stats(const string &s)
{
    Stats stat;

    istrstream is(s.c_str(), s.length());

    is >> stat.collection_size;
    is >> stat.average_length;

    string word;
    while (is >> word) {
	if (word.length() == 0) continue;

	if (word[0] == 'T') {
            vector<string> parts;
	    split_words(word.substr(1), parts, '=');

	    if (parts.size() != 2) {
		throw OmNetworkError(string("Invalid stats string word part: ")
				     + word);
	    }

	    stat.termfreq[decode_tname(parts[0])] = atoi(parts[1].c_str());
	} else if (word[0] == 'R') {
            vector<string> parts;
	    split_words(word.substr(1), parts, '=');

	    if (parts.size() != 2) {
		throw OmNetworkError(string("Invalid stats string word part: ")
				     + word);
	    }
	    
	    stat.reltermfreq[decode_tname(parts[0])] = atoi(parts[1].c_str());
	} else {
	    throw OmNetworkError(string("Invalid stats string word: ") + word);
	}
    }

    return stat;
}

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

OmLineBuf::OmLineBuf(int readfd_, int writefd_)
	: readfd(readfd_), writefd(writefd_)
{
}

OmLineBuf::OmLineBuf(int fd_)
	: readfd(fd_), writefd(fd_)
{
}

string
OmLineBuf::readline()
{
    string::size_type pos;
    while ((pos = buffer.find_first_of('\n')) == buffer.npos) {
	char buf[256];
	ssize_t received = read(readfd, buf, sizeof(buf) - 1);

	buffer += string(buf, buf + received);
    }
    string retval(buffer.begin(), buffer.begin() + pos);

    buffer.erase(0, pos+1);

    return retval;
}

void
OmLineBuf::writeline(string s)
{
    if (s.length() == 0 || s[s.length()-1] != '\n') {
	s += '\n';
    }
    while (s.length() > 0) {
	ssize_t written = write(writefd, s.data(), s.length());

	if (written < 0) {
	    throw OmNetworkError(std::string("write:") + strerror(errno));
	}

	s.erase(0, written);
    }
}
