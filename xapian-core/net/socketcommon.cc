/* socketcommon.cc: Various useful Prog{Server,Client}-related utilities
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003,2005,2006 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>

#include "safeerrno.h"

#ifdef HAVE_SSTREAM
#include <sstream>
#else
#include <strstream.h>
#endif
#include <map>
#include <set>
#include <string>
#include <vector>
#include <sys/time.h>
#include <sys/types.h>

#include "socketcommon.h"
#include "omdebug.h"
#include "omqueryinternal.h"
#include "stats.h"
#include "utils.h"
#include <xapian/enquire.h>
#include <xapian/document.h>
#include "omenquireinternal.h"
#include "netutils.h"

using namespace std;

string stats_to_string(const Stats &stats)
{
    string result;

    result += om_tostring(stats.collection_size);
    result += ' ';
    result += om_tostring(stats.rset_size);
    result += ' ';
    result += om_tostring(stats.average_length);
    result += ' ';

    map<string, Xapian::doccount>::const_iterator i;

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
	    throw Xapian::NetworkError(string("Invalid stats string word: ") + word);
	}
    }

    return stat;
}

OmSocketLineBuf::OmSocketLineBuf(int readfd_, int writefd_, const string & errcontext_)
	: readfd(readfd_), writefd(writefd_), errcontext(errcontext_)
{
    // set non-blocking flag on reading fd
    if (fcntl(readfd, F_SETFL, O_NONBLOCK) < 0) {
	throw Xapian::NetworkError("Can't set non-blocking flag on fd", errcontext, errno);
    }
}

OmSocketLineBuf::OmSocketLineBuf(int fd_, const string & errcontext_)
	: readfd(fd_), writefd(fd_), errcontext(errcontext_)
{
    // set non-blocking flag on reading fd
    if (fcntl(readfd, F_SETFL, O_NONBLOCK) < 0) {
	throw Xapian::NetworkError("Can't set non-blocking flag on fd", errcontext, errno);
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

    int retval;
    bool timeout = end_time.is_set();
    if (timeout) {
	OmTime time_diff(end_time - OmTime::now());
	if (time_diff.sec < 0) time_diff = OmTime(0);

	struct timeval tv;
	tv.tv_sec = time_diff.sec;
	tv.tv_usec = time_diff.usec;

	DEBUGLINE(UNKNOWN, "readfd=" << readfd << ", " <<
		  "tv.tv_sec=" << tv.tv_sec << ", " <<
		  "tv.tv_usec=" << tv.tv_usec);
	retval = select(readfd + 1, &fdset, 0, &fdset, &tv);
    } else {
	retval = select(readfd + 1, &fdset, 0, &fdset, NULL);
    }

    if (retval < 0) {
	if (errno == EINTR) {
	    // select interrupted due to signal
	    DEBUGLINE(UNKNOWN, "Got EINTR in select");
	    return;
	} else {
	    throw Xapian::NetworkError(string("select failed (") +
				 strerror(errno) + ')',
				 errcontext, errno);
	}
    } else if (retval == 0) {
	// Check timeout
	if (timeout && OmTime::now() > end_time) {
	    // Timeout has expired, and no data is waiting
	    // (especially if we've been waiting on a different node's
	    // timeout)
	    DEBUGLINE(UNKNOWN, "timeout reached");
	    throw Xapian::NetworkTimeoutError("No response from remote end", errcontext);
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
	    if (timeout && OmTime::now() > end_time) {
		// Timeout has expired, and no data is waiting
		// (especially if we've been waiting on a different node's
		// timeout)
		DEBUGLINE(UNKNOWN, "read: got EAGAIN, but timeout reached");
		throw Xapian::NetworkTimeoutError("No response from remote end", errcontext);
	    }
	    return;
	} else {
	    throw Xapian::NetworkError("read failed", errcontext, errno);
	}
    } else if (received == 0) {
	DEBUGLINE(UNKNOWN, "read: got 0 bytes");
	throw Xapian::NetworkError("No response from remote end", errcontext);
    }

    buffer += string(buf, buf + received);
}

void
OmSocketLineBuf::wait_for_data(int msecs)
{
    DEBUGCALL(UNKNOWN, string, "OmSocketLineBuf::wait_for_data", msecs);
    OmTime end_time;
    if (msecs) end_time = OmTime::now() + OmTime(msecs);
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
	    throw Xapian::NetworkTimeoutError("Timeout reached waiting to write data to socket", errcontext);
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
	    throw Xapian::NetworkError("Network error waiting for remote", errcontext, errno);
	}
	// if we got this far, we can fit data down the pipe/socket.

	ssize_t written = write(writefd, s.data(), s.length());

	if (written < 0) {
	    throw Xapian::NetworkError("write error", errcontext, errno);
	}

	s.erase(0, written);
    }
}

std::string
OmSocketLineBuf::readline(const OmTime & end_time)
{
    std::string retval;
    if (line_buffer.length() > 0) {
	retval = line_buffer;
	line_buffer = "";
    } else {
	retval = do_readline(end_time);
    }
    return retval;
}

void
OmSocketLineBuf::writeline(std::string msg, const OmTime & end_time)
{
    do_writeline(msg, end_time);
}

string
omrset_to_string(const Xapian::RSet &omrset)
{
    return omrset.internal->to_string();
}

string
ommsetitems_to_string(const vector<Xapian::Internal::MSetItem> &ommsetitems)
{
    // format: length wt did key wt did key ...
    string result = om_tostring(ommsetitems.size());

    vector<Xapian::Internal::MSetItem>::const_iterator i;
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
			     Xapian::MSet::Internal::TermFreqAndWeight> &terminfo)
{
    // encode as term freq weight term2 freq2 weight2 ...
    string result;

    map<string, Xapian::MSet::Internal::TermFreqAndWeight>::const_iterator i;
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
ommset_to_string(const Xapian::MSet &ommset)
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
    result += ommsetitems_to_string(ommset.internal->items);
    result += " ";
    result += ommset_termfreqwts_to_string(ommset.internal->termfreqandwts);

    return result;
}

vector<Xapian::Internal::MSetItem>
string_to_ommsetitems(const string &s_)
{
    vector<Xapian::Internal::MSetItem> result;

    string::size_type colon = s_.find_first_of(' ');
    string s = s_.substr(colon + 1);

    while (s.length() > 0) {
	string wt_s, did_s, value_s;
	string::size_type pos = s.find_first_of(' ');
	if (pos == s.npos) {
	    throw Xapian::NetworkError("Invalid msetitem string `" + s + "'");
	}
	wt_s = s.substr(0, pos);
	s = s.substr(pos + 1);

	pos = s.find_first_of(' ');
	if (pos == s.npos) {
	    throw Xapian::NetworkError("Invalid msetitem string `" + s + "'");
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
	result.push_back(Xapian::Internal::MSetItem(atof(wt_s.c_str()),
				    atol(did_s.c_str()),
				    decode_tname(value_s)));
    }
    AssertEq((unsigned)atoi(s_.substr(0, colon)), result.size());
    return result;
}

Xapian::MSet
string_to_ommset(const string &s)
{
#ifdef HAVE_SSTREAM
    istringstream is(s);
#else
    istrstream is(s.data(), s.length());
#endif

    Xapian::doccount firstitem;
    Xapian::doccount matches_lower_bound;
    Xapian::doccount matches_estimated;
    Xapian::doccount matches_upper_bound;
    Xapian::weight max_possible;
    Xapian::weight max_attained;
    vector<Xapian::Internal::MSetItem> items;
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
	throw Xapian::NetworkError("Problem reading Xapian::MSet from string");
    }
    while (msize > 0) {
	string t;
	Xapian::weight wt;
	Xapian::docid did;
	is >> wt >> did >> t;
	if (!is) {
	    throw Xapian::NetworkError("Problem reading Xapian::MSet from string");
	}
	items.push_back(Xapian::Internal::MSetItem(wt, did, decode_tname(t)));
	--msize;
    }

    map<string, Xapian::MSet::Internal::TermFreqAndWeight> terminfo;
    string term;
    while (is >> term) {
	Xapian::MSet::Internal::TermFreqAndWeight tfaw;
	if (!(is >> tfaw.termfreq >> tfaw.termweight)) {
	    Assert(false); // FIXME
	}
	term = decode_tname(term);
	terminfo[term] = tfaw;
    }

    return Xapian::MSet(new Xapian::MSet::Internal(
				       firstitem,
				       matches_upper_bound,
				       matches_lower_bound,
				       matches_estimated,
				       max_possible, max_attained,
				       items, terminfo, 0));
}

map<string, Xapian::MSet::Internal::TermFreqAndWeight>
string_to_ommset_termfreqwts(const string &s)
{
    map<string, Xapian::MSet::Internal::TermFreqAndWeight> result;
#ifdef HAVE_SSTREAM
    istringstream is(s);
#else
    istrstream is(s.data(), s.length());
#endif

    string term;
    while (is >> term) {
	Xapian::MSet::Internal::TermFreqAndWeight tfaw;
	if (!(is >> tfaw.termfreq >> tfaw.termweight)) {
	    Assert(false); // FIXME
	}
	term = decode_tname(term);
	result[term] = tfaw;
    }

    return result;
}

Xapian::RSet
string_to_omrset(const string &s)
{
    Xapian::RSet omrset;

    Xapian::docid did;
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
