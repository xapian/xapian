/* socketcommon.cc: Various useful Prog{Server,Client}-related utilities
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002 Olly Betts
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

#include <config.h>

#ifdef HAVE_SSTREAM
#include <sstream>
using std::istringstream;
#else
#include <strstream.h>
#endif
#include <map>
using std::map;
#include <set>
using std::set;
#include <string>
#include <vector>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
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
#include "omenquireinternal.h"

OmQuery::Internal qfs_readcompound();

OmQuery::Internal query_from_string(string qs)
{
    Assert(qs.length() > 1);

    qfs_start(qs);

    OmQuery::Internal retval = qfs_readquery();
    DebugMsg("query_from_string(" << qs << ") = " << retval.serialise());
    Assert(retval.serialise() == qs);

    qfs_end();

    return retval;
}

string stats_to_string(const Stats &stats)
{
#if 0
    ostrstream os;

    os << stats.collection_size << ' ';
    os << stats.average_length << ' ';

    map<string, om_doccount>::const_iterator i;

    for (i=stats.termfreq.begin(); i != stats.termfreq.end(); ++i) {
	os << 'T' << encode_tname(i->first) << ' ' << i->second << ' ';
    }

    for (i = stats.reltermfreq.begin(); i != stats.reltermfreq.end(); ++i) {
	os << 'R' << encode_tname(i->first) << ' ' << i->second << ' ';
    }

    // FIXME: should be eos.
    os << '\0';

    string result(os.str());

    os.freeze(0);
#endif
    string result;

    result += om_tostring(stats.collection_size);
    result += ' ';
    result += om_tostring(stats.rset_size);
    result += ' ';
    result += om_tostring(stats.average_length);
    result += ' ';

    map<string, om_doccount>::const_iterator i;

    for (i = stats.termfreq.begin(); i != stats.termfreq.end(); ++i) {
	result += 'T';
	result += encode_tname(i->first);
	result += ' ';
	result += om_tostring(i->second);
	result += ' ';
    }

    for (i = stats.reltermfreq.begin(); i != stats.reltermfreq.end(); ++i) {
	result += 'R';
	result += encode_tname(i->first);
	result += ' ';
	result += om_tostring(i->second);
	result += ' ';
    }

    return result;
}

Stats
string_to_stats(const string &s)
{
    Stats stat;
#ifdef HAVE_SSTREAM
    istringstream is(s);
#else
    istrstream is(s.data(), s.length());
#endif

    is >> stat.collection_size;
    is >> stat.rset_size;
    is >> stat.average_length;

    string word;
    while (is >> word) {
	if (word.empty()) continue;

	if (word[0] == 'T') {
	    is >> stat.termfreq[decode_tname(word.substr(1))];
	} else if (word[0] == 'R') {
	    is >> stat.reltermfreq[decode_tname(word.substr(1))];
	} else {
	    throw OmNetworkError(string("Invalid stats string word: ") + word);
	}
    }

    return stat;
}

// A vector converter: vector<OmQuery::Internal> to vector<OmQuery::Internal *>
// The original vector is still responsible for destroying the objects.
vector<OmQuery::Internal *>
convert_subqs(vector<OmQuery::Internal> &v) {
    vector<OmQuery::Internal *> result;
    vector<OmQuery::Internal>::iterator i;
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
				 om_tostring(qt.type) + '\'');
}

static OmQuery::Internal
qint_from_vector(OmQuery::op op, vector<OmQuery::Internal *> & vec) {
    OmQuery::Internal qint(op);
    vector<OmQuery::Internal *>::const_iterator i;
    for (i = vec.begin(); i != vec.end(); i++)
	qint.add_subquery(**i);
    qint.end_construction();
    return qint;
}

OmQuery::Internal qfs_readcompound()
{
    vector<OmQuery::Internal> subqs;
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
		    vector<OmQuery::Internal *> temp =
			    convert_subqs(subqs);
		    querytok myqt = qfs_gettok();
		    if (myqt.type != querytok::OP_KET) {
			throw OmInvalidArgumentError("Expected %) in query string");
		    }
		    return qint_from_vector(OmQuery::OP_AND, temp);
		}
		break;
	    case querytok::OP_OR:
		{
		    vector<OmQuery::Internal *> temp =
			    convert_subqs(subqs);
		    querytok myqt = qfs_gettok();
		    if (myqt.type != querytok::OP_KET) {
			throw OmInvalidArgumentError("Expected %) in query string");
		    }
		    return qint_from_vector(OmQuery::OP_OR, temp);
		}
		break;
	    case querytok::OP_FILTER:
		{
		    vector<OmQuery::Internal *> temp =
			    convert_subqs(subqs);
		    querytok myqt = qfs_gettok();
		    if (myqt.type != querytok::OP_KET) {
			throw OmInvalidArgumentError("Expected %) in query string");
		    }
		    return qint_from_vector(OmQuery::OP_FILTER, temp);
		}
		break;
	    case querytok::OP_XOR:
		{
		    vector<OmQuery::Internal *> temp =
			    convert_subqs(subqs);
		    querytok myqt = qfs_gettok();
		    if (myqt.type != querytok::OP_KET) {
			throw OmInvalidArgumentError("Expected %) in query string");
		    }
		    return qint_from_vector(OmQuery::OP_XOR, temp);
		}
		break;
	    case querytok::OP_ANDMAYBE:
		{
		    vector<OmQuery::Internal *> temp =
			    convert_subqs(subqs);
		    querytok myqt = qfs_gettok();
		    if (myqt.type != querytok::OP_KET) {
			throw OmInvalidArgumentError("Expected %) in query string");
		    }
		    return qint_from_vector(OmQuery::OP_AND_MAYBE, temp);
		}
		break;
	    case querytok::OP_ANDNOT:
		{
		    vector<OmQuery::Internal *> temp =
			    convert_subqs(subqs);
		    querytok myqt = qfs_gettok();
		    if (myqt.type != querytok::OP_KET) {
			throw OmInvalidArgumentError("Expected %) in query string");
		    }
		    return qint_from_vector(OmQuery::OP_AND_NOT, temp);
		}
		break;
	    case querytok::OP_NEAR:
		{
		    vector<OmQuery::Internal *> temp =
			    convert_subqs(subqs);
		    querytok myqt = qfs_gettok();
		    if (myqt.type != querytok::OP_KET) {
			throw OmInvalidArgumentError("Expected %) in query string");
		    }
		    OmQuery::Internal qint(qint_from_vector(OmQuery::OP_NEAR, temp));
		    qint.set_window(qt.window);
		    return qint;

		}
		break;
	    case querytok::OP_PHRASE:
		{
		    vector<OmQuery::Internal *> temp =
			    convert_subqs(subqs);
		    querytok myqt = qfs_gettok();
		    if (myqt.type != querytok::OP_KET) {
			throw OmInvalidArgumentError("Expected %) in query string");
		    }
		    OmQuery::Internal qint(qint_from_vector(OmQuery::OP_PHRASE, temp));
		    qint.set_window(qt.window);
		    return qint;
		}
		break;
	    case querytok::OP_WEIGHT_CUTOFF:
		{
		    querytok myqt = qfs_gettok();
		    if (myqt.type != querytok::OP_KET) {
			throw OmInvalidArgumentError("Expected %) in query string");
		    }
		    OmQuery::Internal qint(OmQuery::OP_WEIGHT_CUTOFF);
		    Assert(subqs.size() == 1);
		    qint.add_subquery(subqs[0]);
		    qint.end_construction();
		    qint.set_cutoff(qt.cutoff);
		    return qint;
		}
		break;
	    case querytok::OP_ELITE_SET:
		{
		    vector<OmQuery::Internal *> temp =
			    convert_subqs(subqs);
		    querytok myqt = qfs_gettok();
		    if (myqt.type != querytok::OP_KET) {
			throw OmInvalidArgumentError("Expected %) in query string");
		    }
		    OmQuery::Internal qint(qint_from_vector(OmQuery::OP_ELITE_SET, temp));
		    qint.set_elite_set_size(qt.elite_set_size);
		    return qint;
		}
		break;
	    default:
		throw OmInvalidArgumentError("Invalid query string");
	} // switch(qt.type)
    } // while(1)
}

OmSocketLineBuf::OmSocketLineBuf(int readfd_, int writefd_, const string & errcontext_)
	: readfd(readfd_), writefd(writefd_), errcontext(errcontext_)
{
    // set non-blocking flag on reading fd
    if (fcntl(readfd, F_SETFL, O_NONBLOCK) < 0) {
	throw OmNetworkError("Can't set non-blocking flag on fd", errcontext, errno);
    }
}

OmSocketLineBuf::OmSocketLineBuf(int fd_, const string & errcontext_)
	: readfd(fd_), writefd(fd_), errcontext(errcontext_)
{
    // set non-blocking flag on reading fd
    if (fcntl(readfd, F_SETFL, O_NONBLOCK) < 0) {
	throw OmNetworkError("Can't set non-blocking flag on fd", errcontext, errno);
    }
}

string
OmSocketLineBuf::do_readline(const OmTime & end_time)
{
    DEBUGCALL(UNKNOWN, string, "OmSocketLineBuf::do_readline",
	      end_time.sec << ", " << end_time.usec);
    string::size_type pos;
    while ((pos = buffer.find_first_of('\n')) == buffer.npos) {
	attempt_to_read(end_time);
    }
    string retval(buffer.begin(), buffer.begin() + pos);
    buffer.erase(0, pos + 1);
    RETURN(retval);
}

void
OmSocketLineBuf::attempt_to_read(const OmTime & end_time)
{
    DEBUGCALL(UNKNOWN, string, "OmSocketLineBuf::attempt_to_read",
	      end_time.sec << ':' << end_time.usec);
    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(readfd, &fdset);

    OmTime time_diff(end_time - OmTime::now());
    if (time_diff.sec < 0) time_diff = OmTime(0);

    struct timeval tv;
    tv.tv_sec = time_diff.sec;
    tv.tv_usec = time_diff.usec;

    DEBUGLINE(UNKNOWN, "readfd=" << readfd << ", " <<
	      "tv.tv_sec=" << tv.tv_sec << ", " <<
	      "tv.tv_usec=" << tv.tv_usec);
    int retval = select(readfd + 1, &fdset, 0, &fdset, &tv);

    if (retval < 0) {
	if (errno == EINTR) {
	    // select interrupted due to signal
	    DEBUGLINE(UNKNOWN, "Got EINTR in select");
	    return;
	} else {
	    throw OmNetworkError(string("select failed (") +
				 strerror(errno) + ')',
				 errcontext, errno);
	}
    } else if (retval == 0) {
	// Check timeout
	if (OmTime::now() > end_time) {
	    // Timeout has expired, and no data is waiting
	    // (especially if we've been waiting on a different node's
	    // timeout)
	    DEBUGLINE(UNKNOWN, "timeout reached");
	    throw OmNetworkTimeoutError("No response from remote end", errcontext);
	}
	return;
    }

    char buf[4096];
    ssize_t received = read(readfd, buf, sizeof(buf));

    if (received < 0) {
	if (errno == EINTR) {
	    DEBUGLINE(UNKNOWN, "Got EINTR in read");
	    return;
	} else if (errno == EAGAIN) {
	    // Check timeout
	    if (OmTime::now() > end_time) {
		// Timeout has expired, and no data is waiting
		// (especially if we've been waiting on a different node's
		// timeout)
		DEBUGLINE(UNKNOWN, "read: got EAGAIN, but timeout reached");
		throw OmNetworkTimeoutError("No response from remote end", errcontext);
	    }
	    return;
	} else {
	    throw OmNetworkError("read failed", errcontext, errno);
	}
    } else if (received == 0) {
	DEBUGLINE(UNKNOWN, "read: got 0 bytes");
	throw OmNetworkError("No response from remote end", errcontext);
    }

    buffer += string(buf, buf + received);
}

void
OmSocketLineBuf::wait_for_data(int msecs)
{
    DEBUGCALL(UNKNOWN, string, "OmSocketLineBuf::wait_for_data", msecs);
    OmTime end_time = OmTime::now() + OmTime(msecs);
    // wait for input to be available.
    while (buffer.find_first_of('\n') == buffer.npos) {
	attempt_to_read(end_time);
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
OmSocketLineBuf::do_writeline(string s, const OmTime & end_time)
{
    DEBUGCALL(UNKNOWN, void, "OmSocketLineBuf::do_writeline", s);
    if (s.empty() || s[s.length() - 1] != '\n') {
	s += '\n';
    }
    while (s.length() > 0) {
	// the socket can become full - we need to use select() to wait
	// until there is space.
	fd_set fdset;
	FD_ZERO(&fdset);
	FD_SET(writefd, &fdset);

	OmTime time_diff(end_time - OmTime::now());
	if (time_diff.sec < 0) time_diff = OmTime(0);

	// this should probably go in an outer loop rather than the inner.
	struct timeval tv;
	tv.tv_sec = time_diff.sec;
	tv.tv_usec = time_diff.usec;

	int retval = select(writefd + 1, 0, &fdset, &fdset, &tv);

	if (retval == 0) {
	    // select() timed out.
	    throw OmNetworkTimeoutError("Timeout reached waiting to write data to socket", errcontext);
	} else if (retval < 0) {
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
	// if we got this far, we can fit data down the pipe/socket.

	ssize_t written = write(writefd, s.data(), s.length());

	if (written < 0) {
	    throw OmNetworkError("write error", errcontext, errno);
	}

	s.erase(0, written);
    }
}

string
omrset_to_string(const OmRSet &omrset)
{
    string result = om_tostring(omrset.size());

    for (set<om_docid>::const_iterator i = omrset.internal->items.begin();
	 i != omrset.internal->items.end();
	 ++i) {
	result += " ";
	result += om_tostring(*i);
    }
    return result;
}

string
ommsetitems_to_string(const vector<OmMSetItem> &ommsetitems)
{
    // format: length wt did key wt did key ...
    string result = om_tostring(ommsetitems.size());

    vector<OmMSetItem>::const_iterator i;
    for (i = ommsetitems.begin(); i != ommsetitems.end(); ++i) {
	result += ' ';

	result += om_tostring(i->wt);
	result += " ";
	result += om_tostring(i->did);
	result += " ";
	result += encode_tname(i->collapse_key);

	DEBUGLINE(UNKNOWN, "MSETITEM: " << i->wt << " " << i->did);
    }
    //DEBUGLINE(UNKNOWN, "sent items...");

    return result;
}

string
ommset_termfreqwts_to_string(const map<string,
			     OmMSet::Internal::Data::TermFreqAndWeight> &terminfo)
{
    // encode as term freq weight term2 freq2 weight2 ...
    string result;

    map<string, OmMSet::Internal::Data::TermFreqAndWeight>::const_iterator i;
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

string
ommset_to_string(const OmMSet &ommset)
{
    string result;

    // termandfreqthingies
    // items
    result += om_tostring(ommset.get_firstitem());
    result += " ";
    result += om_tostring(ommset.get_matches_lower_bound());
    result += " ";
    result += om_tostring(ommset.get_matches_estimated());
    result += " ";
    result += om_tostring(ommset.get_matches_upper_bound());
    result += " ";
    result += om_tostring(ommset.get_max_possible());
    result += " ";
    result += om_tostring(ommset.get_max_attained());
    result += " ";
    result += ommsetitems_to_string(ommset.internal->data->items);
    result += " ";
    result += ommset_termfreqwts_to_string(ommset.internal->data->termfreqandwts);

    return result;
}

vector<OmMSetItem>
string_to_ommsetitems(const string &s_)
{
    vector<OmMSetItem> result;

    string::size_type colon = s_.find_first_of(' ');
    string s = s_.substr(colon + 1);

    while (s.length() > 0) {
	string wt_s, did_s, value_s;
	string::size_type pos = s.find_first_of(' ');
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
	    value_s = s;
	    s = "";
	} else {
	    value_s = s.substr(0, pos);
	    s = s.substr(pos + 1);
	}
	result.push_back(OmMSetItem(atof(wt_s.c_str()),
				    atol(did_s.c_str()),
				    decode_tname(value_s)));
    }
    AssertEq((unsigned)atoi(s_.substr(0, colon)), result.size());
    return result;
}

OmMSet
string_to_ommset(const string &s)
{
#ifdef HAVE_SSTREAM
    istringstream is(s);
#else
    istrstream is(s.data(), s.length());
#endif

    om_doccount firstitem;
    om_doccount matches_lower_bound;
    om_doccount matches_estimated;
    om_doccount matches_upper_bound;
    om_weight max_possible;
    om_weight max_attained;
    vector<OmMSetItem> items;
    int msize;

    // first the easy ones...
    is >> firstitem >> 
	    matches_lower_bound >>
	    matches_estimated >>
	    matches_upper_bound >>
	    max_possible >>
	    max_attained >>
	    msize;
    if (!is) {
	throw OmNetworkError("Problem reading OmMSet from string");
    }
    while (msize > 0) {
	string s;
	om_weight wt;
	om_docid did;
	is >> wt >> did >> s;
	if (!is) {
	    throw OmNetworkError("Problem reading OmMSet from string");
	}
	items.push_back(OmMSetItem(wt, did, decode_tname(s)));
	msize--;
    }

    map<string, OmMSet::Internal::Data::TermFreqAndWeight> terminfo;
    string term;
    while (is >> term) {
	OmMSet::Internal::Data::TermFreqAndWeight tfaw;
	if (!(is >> tfaw.termfreq >> tfaw.termweight)) {
	    Assert(false); // FIXME
	}
	term = decode_tname(term);
	terminfo[term] = tfaw;
    }

    return OmMSet(new OmMSet::Internal(new OmMSet::Internal::Data(
				       firstitem,
				       matches_upper_bound,
				       matches_lower_bound,
				       matches_estimated,
				       max_possible, max_attained,
				       items, terminfo, 0)));
}

map<string, OmMSet::Internal::Data::TermFreqAndWeight>
string_to_ommset_termfreqwts(const string &s)
{
    map<string, OmMSet::Internal::Data::TermFreqAndWeight> result;
#ifdef HAVE_SSTREAM
    istringstream is(s);
#else
    istrstream is(s.data(), s.length());
#endif

    string term;
    while (is >> term) {
	OmMSet::Internal::Data::TermFreqAndWeight tfaw;
	if (!(is >> tfaw.termfreq >> tfaw.termweight)) {
	    Assert(false); // FIXME
	}
	term = decode_tname(term);
	result[term] = tfaw;
    }

    return result;
}

OmRSet
string_to_omrset(const string &s)
{
    OmRSet omrset;

    om_docid did;
    int numitems;

#ifdef HAVE_SSTREAM
    istringstream is(s);
#else
    istrstream is(s.data(), s.length());
#endif

    is >> numitems;

    while (numitems-- && (is >> did)) {
	omrset.add_document(did);
    }

    return omrset;
}
