/* socketcommon.cc: Various useful Prog{Server,Client}-related utilities
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include "socketcommon.h"
#include "omdebug.h"
#include "omqueryinternal.h"
#include "readquery.h"
#include "stats.h"
#include "utils.h"
#include "om/omenquire.h"
#include "om/omdocument.h"
#include "omlinebuf.h"

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
#if 0
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
#endif
    string result;

    result += om_inttostring(stats.collection_size);
    result += " ";
    result += om_inttostring(stats.rset_size);
    result += " ";
    result += doubletostring(stats.average_length);
    result += " ";

    map<om_termname, om_doccount>::const_iterator i;

    for (i=stats.termfreq.begin();
	 i != stats.termfreq.end();
	 ++i) {
	result = result + "T" + encode_tname(i->first) +
		"=" + doubletostring(i->second) + " ";
    }

    for (i=stats.reltermfreq.begin();
	 i != stats.reltermfreq.end();
	 ++i) {
	result = result + "R" + encode_tname(i->first) +
		"=" + doubletostring(i->second) + " ";
    }

    return result;
}

Stats
string_to_stats(const string &s)
{
    Stats stat;

    istrstream is(s.c_str(), s.length());

    is >> stat.collection_size;
    is >> stat.rset_size;
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
	case querytok::BOOL_FLAG:
	    {
		OmQueryInternal temp(qfs_readquery());
		temp.set_bool(true);
		return temp;
	    }
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
	    case querytok::BOOL_FLAG:
		{
		    OmQueryInternal temp(qfs_readquery());
		    temp.set_bool(true);
		    subqs.push_back(temp);
		}
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

OmSocketLineBuf::OmSocketLineBuf(int readfd_, int writefd_)
	: readfd(readfd_), writefd(writefd_)
{
    // set non-blocking flag on reading fd
    if (fcntl(readfd, F_SETFL, O_NONBLOCK) < 0) {
	throw OmNetworkError("Can't set non-blocking flag on fd");
    };
}

OmSocketLineBuf::OmSocketLineBuf(int fd_)
	: readfd(fd_), writefd(fd_)
{
    // set non-blocking flag on reading fd
    if (fcntl(readfd, F_SETFL, O_NONBLOCK) < 0) {
	throw OmNetworkError("Can't set non-blocking flag on fd");
    };
}

string
OmSocketLineBuf::readline()
{
    string::size_type pos;

    // FIXME: put the timeout somewhere sensible.
    const int timeout = 10;

    time_t start_time = time(NULL);
    time_t curr_time;

    while ((curr_time = time(NULL)) <= (start_time + timeout) &&
	   (pos = buffer.find_first_of('\n')) == buffer.npos) {
	char buf[256];
	
	// wait for input to be available.
	fd_set fdset;
	FD_ZERO(&fdset);
	FD_SET(readfd, &fdset);

	struct timeval tv;
	tv.tv_sec = start_time + timeout - curr_time;
	tv.tv_usec = 0;

	int retval = select(readfd+1, &fdset, 0, 0, &tv);

	if (retval < 0) {
	    if (errno == EAGAIN) {
		continue;
	    } else {
		throw OmNetworkError(string("select:") = strerror(errno));
	    }
	} else if (retval == 0) {
	    continue;
	}

	ssize_t received = read(readfd, buf, sizeof(buf) - 1);

	buffer += string(buf, buf + received);
    }
    if (curr_time > (start_time + timeout)) {
	throw OmNetworkTimeoutError("No response from remote end");
    }

    string retval(buffer.begin(), buffer.begin() + pos);

    buffer.erase(0, pos+1);

    return retval;
}

void
OmSocketLineBuf::wait_for_data(int msecs)
{
    // FIXME: share with readline()
    while (buffer.find_first_of('\n') == buffer.npos) {
	char buf[256];
	
	// wait for input to be available.
	fd_set fdset;
	FD_ZERO(&fdset);
	FD_SET(readfd, &fdset);

	struct timeval tv;
	tv.tv_sec = msecs / 1000;
	tv.tv_usec = (msecs % 1000) * 1000;

	int retval = select(readfd+1, &fdset, 0, 0,
			    (msecs == 0)? NULL : &tv);
	if (retval == 0) {
	    // select's timeout arrived before any data
	    throw OmNetworkTimeoutError("Timeout exceeded waiting for remote.");
	} else if (retval < 0) {
	    // an error happened
	    if (errno == EINTR) {
		// select interrupted due to signal
		// FIXME: adjust timeout for next time around to compensate
		// for time used.  Need to use gettimeofday() or similar, since
		// the contents of tv are now effectively undefined.  (On Linux,
		// it's the time not slept, but this isn't portable)
		continue;
	    } else {
		throw OmNetworkError("Unknown network error waiting for remote.");
	    }
	}
	// if we got this far, then there is data to be received.

	ssize_t received;
	do {
	    received = read(readfd, buf, sizeof(buf) - 1);

	    if (received > 0) {
		buffer += string(buf, buf + received);
	    } else if (received < 0) {
		if (errno != EAGAIN) {
		    throw OmNetworkError(string("Network error: ") +
					 string(strerror(errno)));
		}
	    }
	} while (received > 0);
    }
}

bool
OmSocketLineBuf::data_waiting()
{
    if (buffer.find_first_of('\n') != buffer.npos) {
	return true;
    } else {
	// crude check to see if there's data in the socket
	fd_set fdset;
	FD_ZERO(&fdset);
	FD_SET(readfd, &fdset);

	struct timeval tv;
	tv.tv_sec = 0;
	// a tenth of a second, as an arbitrary non-zero number to avoid
	// hogging the CPU in a loop.
	tv.tv_usec = 100000;
	if (select(readfd+1, &fdset, 0, 0, &tv) > 0) {
	    return true;
	} else {
	    return false;
	}
    }
}

void
OmSocketLineBuf::writeline(string s)
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

string
moptions_to_string(const OmMatchOptions &moptions)
{
    string result;

    result += om_inttostring(moptions.do_collapse) + " ";
    result += om_inttostring(moptions.collapse_key) + " ";
    result += om_inttostring(moptions.sort_forward) + " ";
    result += om_inttostring(moptions.percent_cutoff) + " ";
    result += om_inttostring(moptions.max_or_terms);

    return result;
}

OmMatchOptions
string_to_moptions(const string &s)
{
    istrstream is(s.c_str());

    OmMatchOptions mopt;

    is >> mopt.do_collapse
       >> mopt.collapse_key
       >> mopt.sort_forward
       >> mopt.percent_cutoff
       >> mopt.max_or_terms;

    DebugMsg("string_to_moptions: mopt " << s << "->"
	     << moptions_to_string(mopt) << endl);
    return mopt;
}

string
omrset_to_string(const OmRSet &omrset)
{
    string result = om_inttostring(omrset.items.size());

    for (set<om_docid>::const_iterator i = omrset.items.begin();
	 i != omrset.items.end();
	 ++i) {
	result += " ";
	result += om_inttostring(*i);
    }
    return result;
}

string
omkey_to_string(const OmKey &omkey)
{
    return encode_tname(omkey.value);
}

OmKey
string_to_omkey(const string &s)
{
    return decode_tname(s);
}

OmRSet
string_to_omrset(const string &s)
{
    OmRSet omrset;

    om_docid did;
    int numitems;

    istrstream is(s.c_str());

    is >> numitems;

    while (numitems-- && (is >> did)) {
	omrset.items.insert(did);
    }

    return omrset;
}

bool startswith(const string &s, const string &prefix)
{
    for (string::size_type i=0; i<prefix.length(); ++i) {
	if ((i > s.length()) || (s[i] != prefix[i])) {
	    return false;
	}
    }
    return true;
}
