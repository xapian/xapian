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

OmQuery::Internal qfs_readcompound();

OmQuery::Internal query_from_string(std::string qs)
{
    Assert(qs.length() > 1);

    qfs_start(qs);

    OmQuery::Internal retval = qfs_readquery();
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

    for (i=stats.termfreq.begin(); i != stats.termfreq.end(); ++i) {
	os << "T" << encode_tname(i->first) << " " << i->second << " ";
    }

    for (i = stats.reltermfreq.begin(); i != stats.reltermfreq.end(); ++i) {
	os << "R" << encode_tname(i->first) << " " << i->second << " ";
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
		" " + om_tostring(i->second) + " ";
    }

    for (i=stats.reltermfreq.begin();
	 i != stats.reltermfreq.end();
	 ++i) {
	result = result + "R" + encode_tname(i->first) +
		" " + om_tostring(i->second) + " ";
    }

    return result;
}

Stats
string_to_stats(const std::string &s)
{
    Stats stat;

    istrstream is(s.data(), s.length());

    is >> stat.collection_size;
    is >> stat.rset_size;
    is >> stat.average_length;

    std::string word;
    while (is >> word) {
	if (word.length() == 0) continue;

	if (word[0] == 'T') {
	    is >> stat.termfreq[word.substr(1)];
	} else if (word[0] == 'R') {
	    is >> stat.reltermfreq[word.substr(1)];
	} else {
	    throw OmNetworkError(std::string("Invalid stats string word: ") + word);
	}
    }

    return stat;
}

// A vector converter: vector<OmQuery::Internal> to vector<OmQuery::Internal *>
// The original vector is still responsible for destroying the objects.
std::vector<OmQuery::Internal *>
convert_subqs(std::vector<OmQuery::Internal> &v) {
    std::vector<OmQuery::Internal *> result;
    std::vector<OmQuery::Internal>::iterator i;
    for (i = v.begin(); i != v.end(); ++i) {
	result.push_back(&(*i));
    }
    return result;
}

OmQuery::Internal qfs_readquery()
{
    querytok qt = qfs_gettok();
    switch (qt.type) {
	case querytok::NULL_QUERY:  // null query
	    return OmQuery::Internal();
	    break;
	case querytok::TERM:
	    return OmQuery::Internal(qt.tname, qt.wqf, qt.term_pos);
	    break;
	case querytok::OP_BRA:
	    return qfs_readcompound();
	    break;
	case querytok::BOOL_FLAG:
	    {
		OmQuery::Internal temp(qfs_readquery());
		temp.set_bool(true);
		return temp;
	    }
	    break;
	case querytok::QUERY_LEN:
	    {
		OmQuery::Internal temp(qfs_readquery());
		temp.set_length(qt.qlen);
		return temp;
	    }
	    break;
	default:
	    Assert(false);
    }
    throw OmInvalidArgumentError("Invalid query string: type was `" +
				 om_tostring(qt.type) + "'");
}

OmQuery::Internal qfs_readcompound()
{
    std::vector<OmQuery::Internal> subqs;
    while (1) {
	querytok qt = qfs_gettok();
	switch (qt.type) {
	    case querytok::OP_KET:
		if (subqs.empty()) {
		    return OmQuery::Internal();
		} else {
		    throw OmInvalidArgumentError("Invalid query string");
		}
		break;
	    case querytok::NULL_QUERY:
		subqs.push_back(OmQuery::Internal());
		break;
	    case querytok::BOOL_FLAG:
		{
		    OmQuery::Internal temp(qfs_readquery());
		    temp.set_bool(true);
		    subqs.push_back(temp);
		}
		break;
	    case querytok::QUERY_LEN:
		{
		    OmQuery::Internal temp(qfs_readquery());
		    temp.set_length(qt.qlen);
		    subqs.push_back(temp);
		}
		break;
	    case querytok::TERM:
		subqs.push_back(OmQuery::Internal(qt.tname, qt.wqf,
						  qt.term_pos));
		break;
	    case querytok::OP_BRA:
		subqs.push_back(qfs_readcompound());
		break;
	    case querytok::OP_AND:
		{
		    std::vector<OmQuery::Internal *> temp =
			    convert_subqs(subqs);
		    querytok myqt = qfs_gettok();
		    if (myqt.type != querytok::OP_KET) {
			throw OmInvalidArgumentError("Expected %) in query string");
		    }
		    return OmQuery::Internal(OmQuery::OP_AND, temp.begin(),
					     temp.end());
		}
		break;
	    case querytok::OP_OR:
		{
		    std::vector<OmQuery::Internal *> temp =
			    convert_subqs(subqs);
		    querytok myqt = qfs_gettok();
		    if (myqt.type != querytok::OP_KET) {
			throw OmInvalidArgumentError("Expected %) in query string");
		    }
		    return OmQuery::Internal(OmQuery::OP_OR, temp.begin(),
					     temp.end());
		}
		break;
	    case querytok::OP_FILTER:
		{
		    std::vector<OmQuery::Internal *> temp =
			    convert_subqs(subqs);
		    querytok myqt = qfs_gettok();
		    if (myqt.type != querytok::OP_KET) {
			throw OmInvalidArgumentError("Expected %) in query string");
		    }
		    return OmQuery::Internal(OmQuery::OP_FILTER, temp.begin(),
					     temp.end());
		}
		break;
	    case querytok::OP_XOR:
		{
		    std::vector<OmQuery::Internal *> temp =
			    convert_subqs(subqs);
		    querytok myqt = qfs_gettok();
		    if (myqt.type != querytok::OP_KET) {
			throw OmInvalidArgumentError("Expected %) in query string");
		    }
		    return OmQuery::Internal(OmQuery::OP_XOR, temp.begin(),
					     temp.end());
		}
		break;
	    case querytok::OP_ANDMAYBE:
		{
		    std::vector<OmQuery::Internal *> temp =
			    convert_subqs(subqs);
		    querytok myqt = qfs_gettok();
		    if (myqt.type != querytok::OP_KET) {
			throw OmInvalidArgumentError("Expected %) in query string");
		    }
		    return OmQuery::Internal(OmQuery::OP_AND_MAYBE,
					     temp.begin(), temp.end());
		}
		break;
	    case querytok::OP_ANDNOT:
		{
		    std::vector<OmQuery::Internal *> temp =
			    convert_subqs(subqs);
		    querytok myqt = qfs_gettok();
		    if (myqt.type != querytok::OP_KET) {
			throw OmInvalidArgumentError("Expected %) in query string");
		    }
		    return OmQuery::Internal(OmQuery::OP_AND_NOT, temp.begin(),
					     temp.end());
		}
		break;
	    case querytok::OP_NEAR:
		{
		    std::vector<OmQuery::Internal *> temp =
			    convert_subqs(subqs);
		    querytok myqt = qfs_gettok();
		    if (myqt.type != querytok::OP_KET) {
			throw OmInvalidArgumentError("Expected %) in query string");
		    }
		    return OmQuery::Internal(OmQuery::OP_NEAR, temp.begin(),
					     temp.end(), qt.window);
		}
		break;
	    case querytok::OP_PHRASE:
		{
		    std::vector<OmQuery::Internal *> temp =
			    convert_subqs(subqs);
		    querytok myqt = qfs_gettok();
		    if (myqt.type != querytok::OP_KET) {
			throw OmInvalidArgumentError("Expected %) in query string");
		    }
		    return OmQuery::Internal(OmQuery::OP_PHRASE, temp.begin(),
					     temp.end(), qt.window);
		}
		break;
	    default:
		throw OmInvalidArgumentError("Invalid query string");
	} // switch(qt.type)
    } // while(1)
}

OmSocketLineBuf::OmSocketLineBuf(int readfd_, int writefd_, const std::string & errcontext_)
	: readfd(readfd_), writefd(writefd_), errcontext(errcontext_)
{
    // set non-blocking flag on reading fd
    if (fcntl(readfd, F_SETFL, O_NONBLOCK) < 0) {
	throw OmNetworkError("Can't set non-blocking flag on fd", errcontext, errno);
    }
}

OmSocketLineBuf::OmSocketLineBuf(int fd_, const std::string & errcontext_)
	: readfd(fd_), writefd(fd_), errcontext(errcontext_)
{
    // set non-blocking flag on reading fd
    if (fcntl(readfd, F_SETFL, O_NONBLOCK) < 0) {
	throw OmNetworkError("Can't set non-blocking flag on fd", errcontext, errno);
    }
}

std::string
OmSocketLineBuf::do_readline(int msecs_timeout)
{
    DEBUGCALL(UNKNOWN, std::string, "OmSocketLineBuf::do_readline", msecs_timeout);
    std::string::size_type pos;

    time_t end_time = time(NULL) + msecs_timeout / 1000;    
    int end_time_usecs = (msecs_timeout % 1000) * 1000;

    while (1) {
	time_t curr_time = time(NULL);
	if (curr_time > end_time) {
	    throw OmNetworkTimeoutError("No response from remote end", errcontext);
	}
	pos = buffer.find_first_of('\n');
	if (pos != buffer.npos) break;

	// wait for input to be available.
	fd_set fdset;
	FD_ZERO(&fdset);
	FD_SET(readfd, &fdset);

	struct timeval tv;
	tv.tv_sec = end_time - curr_time;
	tv.tv_usec = end_time_usecs;

	int retval = select(readfd + 1, &fdset, 0, &fdset, &tv);

	if (retval < 0) {
	    if (errno == EAGAIN) {
		continue;
	    } else {
		throw OmNetworkError("select failed", errcontext, errno);
	    }
	} else if (retval == 0) {
	    continue;
	}

	char buf[256];
	ssize_t received = read(readfd, buf, sizeof(buf));

	if (received < 0) {
	    if (errno == EAGAIN) {
		continue;
	    } else {
		throw OmNetworkError("read failed", errcontext, errno);
	    }
	} else if (received == 0) {
	    continue;
	}

	buffer += std::string(buf, buf + received);
    }

    std::string retval(buffer.begin(), buffer.begin() + pos);

    buffer.erase(0, pos + 1);

    RETURN(retval);
}

void
OmSocketLineBuf::wait_for_data(int msecs)
{
    // FIXME: share with readline()
    while (buffer.find_first_of('\n') == buffer.npos) {
	// wait for input to be available.
	fd_set fdset;
	FD_ZERO(&fdset);
	FD_SET(readfd, &fdset);

	struct timeval tv;
	tv.tv_sec = msecs / 1000;
	tv.tv_usec = (msecs % 1000) * 1000;

	int retval = select(readfd + 1, &fdset, 0, &fdset,
			    (msecs == 0) ? NULL : &tv);
	if (retval == 0) {
	    // select's timeout arrived before any data
	    throw OmNetworkTimeoutError("Timeout exceeded waiting for remote.", errcontext);
	} else if (retval < 0) {
	    // an error happened
	    if (errno == EINTR) {
		// select interrupted due to signal
		// FIXME: adjust timeout for next time around to compensate
		// for time used.  Need to use gettimeofday() or similar,
		// since the contents of tv are now effectively undefined.
		// (On Linux, it's the time not slept, but this isn't
		// portable)
		continue;
	    }
	    throw OmNetworkError("Network error waiting for remote", errcontext, errno);
	}
	// if we got this far, then there is data to be received.

	ssize_t received;
	do {
	    char buf[256];

	    received = read(readfd, buf, sizeof(buf) - 1);

	    if (received > 0) {
		buffer += std::string(buf, buf + received);
	    } else if (received < 0) {
		if (errno != EAGAIN) {
		    throw OmNetworkError("Network error", errcontext, errno);
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
	if (select(readfd + 1, &fdset, 0, &fdset, &tv) > 0) {
	    return true;
	} else {
	    return false;
	}
    }
}

void
OmSocketLineBuf::do_writeline(std::string s)
{
    DEBUGCALL(UNKNOWN, void, "OmSocketLineBuf::do_writeline", s);
    if (s.length() == 0 || s[s.length()-1] != '\n') {
	s += '\n';
    }
    while (s.length() > 0) {
	ssize_t written = write(writefd, s.data(), s.length());

	if (written < 0) {
	    throw OmNetworkError("write error", errcontext, errno);
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
    istrstream is(s.data(), s.length());

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
ommsetitems_to_string(const std::vector<OmMSetItem> &ommsetitems)
{
    // format: length wt did key wt did key ...
    std::string result = om_tostring(ommsetitems.size());

    std::vector<OmMSetItem>::const_iterator i;
    for (i = ommsetitems.begin(); i != ommsetitems.end(); ++i) {
	result += ' ';

	result += om_tostring(i->wt);
	result += " ";
	result += om_tostring(i->did);
	result += " ";
	result += omkey_to_string(i->collapse_key);

	DEBUGLINE(UNKNOWN, "MSETITEM: " << i->wt << " " << i->did);
    }
    //DEBUGLINE(UNKNOWN, "sent items...");

    return result;
}

std::string
ommset_termfreqwts_to_string(const std::map<om_termname,
			                OmMSet::TermFreqAndWeight> &terminfo)
{
    // encode as term freq,weight;term2 freq2,weight2;...
    std::string result;

    std::map<om_termname, OmMSet::TermFreqAndWeight>::const_iterator i;
    for (i = terminfo.begin(); i != terminfo.end(); ++i) {
	result += encode_tname(i->first);
	result += " ";
	result += om_tostring(i->second.termfreq);
	result += " ";
	result += om_tostring(i->second.termweight);
	if (i != terminfo.end()) result += ' ';
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
    result += om_tostring(ommset.matches_lower_bound);
    result += " ";
    result += om_tostring(ommset.matches_estimated);
    result += " ";
    result += om_tostring(ommset.matches_upper_bound);
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

std::vector<OmMSetItem>
string_to_ommsetitems(const std::string &s_)
{
    std::vector<OmMSetItem> result;

    std::string::size_type colon = s_.find_first_of(' ');
    std::string s = s_.substr(colon + 1);

    while (s.length() > 0) {
	std::string wt_s, did_s, key_s;
	std::string::size_type pos = s.find_first_of(' ');
	if (pos == s.npos) {
	    throw OmNetworkError("Invalid msetitem string `" + s + "'");
	}
	wt_s = s.substr(0, pos);
	s = s.substr(pos + 1);

	pos = s.find_first_of(' ');
	if (pos == s.npos) {
	    throw OmNetworkError("Invalid msetitem string `" + s + "'");
	}
	did_s = s.substr(0, pos);
	s = s.substr(pos + 1);

	pos = s.find_first_of(' ');
	if (pos == s.npos) {
	    key_s = s;
	    s = "";
	} else {
	    key_s = s.substr(0, pos);
	    s = s.substr(pos + 1);
	}
	result.push_back(OmMSetItem(atof(wt_s.c_str()),
				    atol(did_s.c_str()),
				    string_to_omkey(key_s)));
    }
    AssertEq((unsigned)atoi(s_.substr(0, colon).c_str()), result.size());
    return result;
}

OmMSet
string_to_ommset(const std::string &s)
{
    istrstream is(s.data(), s.length());

    om_doccount firstitem;
    om_doccount matches_lower_bound;
    om_doccount matches_estimated;
    om_doccount matches_upper_bound;
    om_weight max_possible;
    om_weight max_attained;
    std::vector<OmMSetItem> items;

    // first the easy ones...
    is >> firstitem >> 
	    matches_lower_bound >>
	    matches_estimated >>
	    matches_upper_bound >>
	    max_possible >>
	    max_attained;
    int msize;
    is >> msize;
    while (msize > 0) {
	std::string s;
	om_weight wt;
	om_docid did;
	is >> wt >> did >> s;
	items.push_back(OmMSetItem(wt, did, string_to_omkey(s)));
	msize--;
    }

    std::map<om_termname, OmMSet::TermFreqAndWeight> terminfo;
    om_termname term;
    while (is >> term) {
	OmMSet::TermFreqAndWeight tfaw;
	if (!(is >> tfaw.termfreq >> tfaw.termweight)) {
	    Assert(false); // FIXME
	}
	term = decode_tname(term);
	terminfo[term] = tfaw;
    }

    return OmMSet(firstitem,
		  matches_upper_bound,
		  matches_lower_bound,
		  matches_estimated,
		  max_possible, max_attained,
		  items, terminfo);
}

std::map<om_termname, OmMSet::TermFreqAndWeight>
string_to_ommset_termfreqwts(const std::string &s)
{
    std::map<om_termname, OmMSet::TermFreqAndWeight> result;
    istrstream is(s.data(), s.length());

    om_termname term;
    while (is >> term) {
	OmMSet::TermFreqAndWeight tfaw;
	if (!(is >> tfaw.termfreq >> tfaw.termweight)) {
	    Assert(false); // FIXME
	}
	term = decode_tname(term);
	result[term] = tfaw;
    }

    return result;
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

    istrstream is(s.data(), s.length());

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
