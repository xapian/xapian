/* socketcommon.h: Various useful Socket{Server,Client}-related utilities
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003,2005 Olly Betts
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

#ifndef OM_HGUARD_SOCKETCOMMON_H
#define OM_HGUARD_SOCKETCOMMON_H

#include <string>
#include <map>
#include <sys/time.h>

#include "omlinebuf.h"
#include "omenquireinternal.h"

using std::map;

#define XAPIAN_SOCKET_PROTOCOL_VERSION 17

class Stats;
class OmTime;

/** The OmSocketLineBuf class implements a two-way line discipline
 *  using Unix filedescriptors, allowing the client to read
 *  and write a line at a time conveniently.
 */
class OmSocketLineBuf : public OmLineBuf {
    private:
	/// The filedescriptor used for reading
	int readfd;
	/// The filedescriptor used for writing
	int writefd;

	/// The buffer used for input
	string buffer;

	/// The context to report with errors
	string errcontext;

	/// disallow copies
	OmSocketLineBuf(const OmSocketLineBuf &other);
	void operator=(const OmSocketLineBuf &other);

	/** Read one line from readfd
	 *  @param end_time	The time at which the read will
	 *  			fail with a timeout error.
	 */
	string do_readline(const OmTime & end_time);

	/** Write one line to writefd
	 */
	void do_writeline(string s, const OmTime & end_time);

	/** Attempt to read some data
	 */
	void attempt_to_read(const OmTime & end_time);
    public:
	/** The main constructor.  The arguments are the
	 *  input and output filedescriptors to use.
	 */
	OmSocketLineBuf(int readfd_, int writefd_,
	       		const string & errcontext_);

	/** A convenience constructor which takes only one
	 *  fd, which can be both read from and written to.
	 */
	OmSocketLineBuf(int fd_, const string & errcontext_);

	/** Return true if there is data available to be read.
	 */
	bool data_waiting();

	/** Block until at least a line of data has been read.
	 *
	 *  @param msecs  The timeout in milliseconds (or infinite
	 *                if zero).  An exception will be thrown if
	 *                the timeout is exceeded.
	 */
	void wait_for_data(int msecs);
};

/** Convert a Stats object into a string representation.
 *
 *  @param  stats	The stats object to serialise.
 */
string stats_to_string(const Stats &stats);

/** Convert a string representing a Stats object back into an
 *  object.
 *
 *  @param  s		The serialised Stats object.
 */
Stats string_to_stats(const string &s);

/** Convert an RSet object into a string representation.
 *
 *  @param omrset		The object to serialise.
 */
string omrset_to_string(const Xapian::RSet &omrset);

/** Convert a serialised RSet string back into an object.
 *
 *  @param s		The serialised object as a string.
 */
Xapian::RSet string_to_omrset(const string &s);

/** Convert a Xapian::MSet object into a string representation.
 *
 *  @param ommset		The object to serialise.
 */
string ommset_to_string(const Xapian::MSet &ommset);

/** Convert a serialised Xapian::MSet string back into an object.
 *
 *  @param s		The serialised object as a string.
 */
Xapian::MSet string_to_ommset(const string &s);

/** Convert a terminfo map into a string representation.
 *
 *  @param terminfo		The terminfo map to serialise.
 */
string ommset_termfreqwts_to_string(const map<string,
	Xapian::MSet::Internal::TermFreqAndWeight> &terminfo);

/** Convert a serialised terminfo string back into a map.
 *
 *  @param s		The seralised map as a string.
 */
map<string, Xapian::MSet::Internal::TermFreqAndWeight>
string_to_ommset_termfreqwts(const string &s);

/** returns true if the string s starts with prefix.
 */
bool startswith(const string &s, const string &prefix);

#endif /* OM_HGUARD_SOCKETCOMMON_H */
