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

OmQueryInternal query_from_string(std::string qs)
{
    Assert(qs.length() > 1);

    qfs_start(qs);

    OmQueryInternal retval = qfs_readquery();
    DebugMsg("query_from_string(" << qs << ") = " << retval.serialise());
    Assert(retval.serialise() == qs);

    qfs_end();

    return retval;
}

std::string stats_to_string(const Stats &stats)
{
#if 0
    ostrstream os;

    os << stats.collection_size << " ";
    os << stats.average_length << " ";

    std::map<om_termname, om_doccount>::const_iterator i;

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

    std::string result(os.str());

    os.freeze(0);
#endif
    std::string result;

    result += om_tostring(stats.collection_size);
    result += " ";
    result += om_tostring(stats.rset_size);
    result += " ";
    result += om_tostring(stats.average_length);
    result += " ";

    std::map<om_termname, om_doccount>::const_iterator i;

    for (i=stats.termfreq.begin();
	 i != stats.termfreq.end();
	 ++i) {
	result = result + "T" + encode_tname(i->first) +
		"=" + om_tostring(i->second) + " ";
    }

    for (i=stats.reltermfreq.begin();
	 i != stats.reltermfreq.end();
	 ++i) {
	result = result + "R" + encode_tname(i->first) +
		"=" + om_tostring(i->second) + " ";
    }

    return result;
}

Stats
string_to_stats(const std::string &s)
{
    Stats stat;

    istrstream is(s.c_str(), s.length());

    is >> stat.collection_size;
    is >> stat.rset_size;
    is >> stat.average_length;

    std::string word;
    while (is >> word) {
	if (word.length() == 0) continue;

	if (word[0] == 'T') {
            std::vector<std::string> parts;
	    split_words(word.substr(1), parts, '=');

	    if (parts.size() != 2) {
		throw OmNetworkError(std::string("Invalid stats string word part: ")
				     + word);
	    }

	    stat.termfreq[decode_tname(parts[0])] = atoi(parts[1].c_str());
	} else if (word[0] == 'R') {
            std::vector<std::string> parts;
	    split_words(word.substr(1), parts, '=');

	    if (parts.size() != 2) {
		throw OmNetworkError(std::string("Invalid stats string word part: ")
				     + word);
	    }

	    stat.reltermfreq[decode_tname(parts[0])] = atoi(parts[1].c_str());
	} else {
	    throw OmNetworkError(std::string("Invalid stats string word: ") + word);
	}
    }

    return stat;
}

// A vector converter: vector<OmQueryInternal> to vector<OmQueryInternal *>
// The original vector is still responsible for destroying the objects.
std::vector<OmQueryInternal *>
convert_subqs(std::vector<OmQueryInternal> &v) {
    std::vector<OmQueryInternal *> result;
    for (std::vector<OmQueryInternal>::iterator i=v.begin();
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
	    Assert(false);
    }
    throw OmInvalidArgumentError("Invalid query string: type was `" +
				 om_tostring(qt.type) + "'");
}

OmQueryInternal qfs_readcompound()
{
    querytok qt;
    std::vector<OmQueryInternal> subqs;
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
		    std::vector<OmQueryInternal *> temp =
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
		    std::vector<OmQueryInternal *> temp =
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
		    std::vector<OmQueryInternal *> temp =
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
		    std::vector<OmQueryInternal *> temp =
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
		    std::vector<OmQueryInternal *> temp =
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
		    std::vector<OmQueryInternal *> temp =
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
	    case querytok::OP_NEAR:
		{
		    std::vector<OmQueryInternal *> temp =
			    convert_subqs(subqs);
		    querytok myqt = qfs_gettok();
		    if (myqt.type != querytok::OP_KET) {
			throw OmInvalidArgumentError("Expected %) in query string");
		    }
		    return OmQueryInternal(OM_MOP_NEAR,
					   temp.begin(),
					   temp.end(), qt.window);
		}
		break;
	    case querytok::OP_PHRASE:
		{
		    std::vector<OmQueryInternal *> temp =
			    convert_subqs(subqs);
		    querytok myqt = qfs_gettok();
		    if (myqt.type != querytok::OP_KET) {
			throw OmInvalidArgumentError("Expected %) in query string");
		    }
		    return OmQueryInternal(OM_MOP_PHRASE,
					   temp.begin(),
					   temp.end(), qt.window);
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

std::string
OmSocketLineBuf::do_readline()
{
    std::string::size_type pos;

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
		throw OmNetworkError(std::string("select:") = strerror(errno));
	    }
	} else if (retval == 0) {
	    continue;
	}

	ssize_t received = read(readfd, buf, sizeof(buf) - 1);

	buffer += std::string(buf, buf + received);
    }
    if (curr_time > (start_time + timeout)) {
	throw OmNetworkTimeoutError("No response from remote end");
    }

    std::string retval(buffer.begin(), buffer.begin() + pos);

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
		buffer += std::string(buf, buf + received);
	    } else if (received < 0) {
		if (errno != EAGAIN) {
		    throw OmNetworkError(std::string("Network error: ") +
					 std::string(strerror(errno)));
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
OmSocketLineBuf::do_writeline(std::string s)
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

std::string
moptions_to_string(const OmSettings &moptions)
{
    std::string result;

    result += om_tostring(moptions.get_int("match_collapse_key", 0)) + " ";
    result += om_tostring((int)moptions.get_bool("match_sort_forward", true)) + " ";
    result += om_tostring(moptions.get_int("match_percent_cutoff", 0)) + " ";
    result += om_tostring(moptions.get_int("match_max_or_terms", 0));

    return result;
}

OmSettings
string_to_moptions(const std::string &s)
{
    istrstream is(s.c_str());

    OmSettings mopt;
    bool sort_forward;
    int collapse_key, percent_cutoff, max_or_terms;

    is >> collapse_key
       >> sort_forward
       >> percent_cutoff
       >> max_or_terms;

    mopt.set("match_collapse_key", collapse_key);
    mopt.set("match_sort_forward", sort_forward);
    mopt.set("match_percent_cutoff", percent_cutoff);
    mopt.set("match_max_or_terms", max_or_terms);
    
    Assert(s == moptions_to_string(mopt));
//    DEBUGLINE(UNKNOWN, "string_to_moptions: mopt " << s << "->"
//	      << moptions_to_string(mopt));
    return mopt;
}

std::string
omrset_to_string(const OmRSet &omrset)
{
    std::string result = om_tostring(omrset.items.size());

    for (std::set<om_docid>::const_iterator i = omrset.items.begin();
	 i != omrset.items.end();
	 ++i) {
	result += " ";
	result += om_tostring(*i);
    }
    return result;
}

std::string
ommsetitems_to_string(const vector<OmMSetItem> &ommsetitems)
{
    // format: length:wt,did,key;wt,did,key;...
    std::string result = om_tostring(ommsetitems.size()) + ":";

    for (std::vector<OmMSetItem>::const_iterator i=ommsetitems.begin();
	 i != ommsetitems.end();
	 ++i) {
	result += om_tostring(i->wt);
	result += ",";
	result += om_tostring(i->did);
	result += ",";
	result += omkey_to_string(i->collapse_key);

	if (i != ommsetitems.end()) {
	    result += ";";
	}

	DEBUGLINE(UNKNOWN, "MSETITEM: " << i->wt << " " << i->did);
    }
    //DEBUGLINE(UNKNOWN, "sent items...");

    return result;
}

std::string
ommset_termfreqwts_to_string(const std::map<om_termname,
			                OmMSet::TermFreqAndWeight> &terminfo)
{
    // encode as term:freq,weight;term2:freq2,weight2;...
    string result;

    std::map<om_termname, OmMSet::TermFreqAndWeight>::const_iterator i;
    for (i = terminfo.begin(); i!= terminfo.end(); ++i) {
	result += encode_tname(i->first);
	result += ":";
	result += om_tostring(i->second.termfreq);
	result += ",";
	result += om_tostring(i->second.termweight);
	result += ";";
    }
    return result;
}

std::string
ommset_to_string(const OmMSet &ommset)
{
    std::string result;

    // termandfreqthingies
    // items
    result += om_tostring(ommset.firstitem);
    result += " ";
    result += om_tostring(ommset.mbound);
    result += " ";
    result += om_tostring(ommset.max_possible);
    result += " ";
    result += om_tostring(ommset.max_attained);
    result += " ";
    result += ommsetitems_to_string(ommset.items);
    result += " ";
    result += ommset_termfreqwts_to_string(ommset.get_all_terminfo());

    return result;
}

vector<OmMSetItem>
string_to_ommsetitems(const std::string &s_)
{
    vector<OmMSetItem> result;
    std::string s = s_;

    string lens = s.substr(0, s.find_first_of(':'));
    s = s.substr(s.find_first_of(':')+1);

    unsigned int len = atoi(lens.c_str());
    while (s.length() > 0) {
	std::string wt_s, did_s, key_s;
	std::string::size_type pos = s.find_first_of(',');
	if (pos == s.npos) {
	    throw OmNetworkError("Invalid msetitem string");
	}
	wt_s = s.substr(0, pos);
	s = s.substr(pos+1);

	pos = s.find_first_of(',');
	if (pos == s.npos) {
	    throw OmNetworkError("Invalid msetitem string");
	}
	did_s = s.substr(0, pos);
	s = s.substr(pos+1);

	pos = s.find_first_of(';');
	if (pos == s.npos) {
	    key_s = s;
	    s = "";
	} else {
	    key_s = s.substr(0, pos);
	    s = s.substr(pos+1);
	}
	result.push_back(OmMSetItem(atof(wt_s.c_str()),
				    atol(did_s.c_str()),
				    string_to_omkey(key_s)));
    }
    Assert(len == result.size());
    return result;
}

std::map<om_termname, OmMSet::TermFreqAndWeight>
string_to_ommset_termfreqwts(const std::string &s_)
{
    std::map<om_termname, OmMSet::TermFreqAndWeight> result;
    std::string s = s_;

    while (s.length() > 0) {
	std::string::size_type pos = s.find_first_of(';');
	std::string terminfo = s.substr(0, pos);
	s = s.substr(pos+1);

	om_termname term;
	om_doccount freq;
	om_weight wt;

	pos = terminfo.find_first_of(':');
	if (pos == terminfo.npos) {
	    throw OmNetworkError("Invalid term frequency/weight info string");
	}
	term = decode_tname(terminfo.substr(0, pos));
	terminfo = terminfo.substr(pos+1);

	pos = terminfo.find_first_of(',');
	if (pos == terminfo.npos || (pos != terminfo.find_last_of(','))) {
	    throw OmNetworkError("Invalid term frequency/weight info string");
	}
	freq = atoi(terminfo.substr(0, pos).c_str());
	wt = atof(terminfo.substr(pos+1).c_str());

	OmMSet::TermFreqAndWeight tfaw;
	tfaw.termfreq = freq;
	tfaw.termweight = wt;
	result[term] = tfaw;
    }

    return result;
}

OmMSet
string_to_ommset(const std::string &s)
{
    istrstream is(s.c_str());

    om_doccount firstitem;
    om_doccount mbound;
    om_weight max_possible;
    om_weight max_attained;
    vector<OmMSetItem> items;
    std::map<om_termname, OmMSet::TermFreqAndWeight> terminfo;

    // first the easy ones...
    is >> firstitem >> mbound >> max_possible >> max_attained;
    std::string items_s, terminfo_s;
    is >> items_s >> terminfo_s;

    items = string_to_ommsetitems(items_s);
    terminfo = string_to_ommset_termfreqwts(terminfo_s);

    return OmMSet(firstitem, mbound,
		  max_possible, max_attained,
		  items, terminfo);
}

std::string
omkey_to_string(const OmKey &omkey)
{
    return encode_tname(omkey.value);
}

OmKey
string_to_omkey(const std::string &s)
{
    return decode_tname(s);
}

OmRSet
string_to_omrset(const std::string &s)
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

bool startswith(const std::string &s, const std::string &prefix)
{
    for (std::string::size_type i=0; i<prefix.length(); ++i) {
	if ((i > s.length()) || (s[i] != prefix[i])) {
	    return false;
	}
    }
    return true;
}
