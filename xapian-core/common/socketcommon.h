/* progcommon.h: Various useful Prog{Server,Client}-related utilities
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

#ifndef OM_HGUARD_SOCKETCOMMON_H
#define OM_HGUARD_SOCKETCOMMON_H

#include <string>

#define OM_SOCKET_PROTOCOL_VERSION 1

class OmQueryInternal;
class OmMatchOptions;
class Stats;
class OmRSet;

/** The OmLineBuf class implements a two-way line discipline
 *  using Unix filedescriptors, allowing the client to read
 *  and write a line at a time conveniently.
 */
class OmLineBuf {
    private:
	/// The filedescriptor used for reading
	int readfd;
	/// The filedescriptor used for writing
	int writefd;

	/// The buffer used for input
	string buffer;

	/// disallow copies
	OmLineBuf(const OmLineBuf &other);
	void operator=(const OmLineBuf &other);
    public:
	/** The main constructor.  The arguments are the
	 *  input and output filedescriptors to use.
	 */
	OmLineBuf(int readfd_, int writefd_);

	/** A convenience constructor which takes only one
	 *  fd, which can be both read from and written to.
	 */
	OmLineBuf(int fd_);

	/** Read one line from readfd
	 */
	string readline();

	/** Return true if there is data available to be read.
	 */
	bool data_waiting();

	/** Block until at least a line of data has been read.
	 *
	 *  @param msecs  The timeout in milliseconds (or infinite
	 *                if zero).  An exception will be thrown if
	 *                the timeout is exceeded.
	 */
	void wait_for_data(int msecs = 0);

	/** Write one line to writefd
	 */
	void writeline(string s);
};

/** Build a query from a serialised string.
 *  This uses a flex parser for the tokenizing.
 *
 *  @param qs  The string from which to build the query.
 */
OmQueryInternal query_from_string(string qs);

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

/** Convert an OmMatchOptions object into a string representation.
 *
 *  @param moptions	The object to serialise.
 */
string moptions_to_string(const OmMatchOptions &moptions);

/** Convert a serialised OmMatchOptions string back into an object.
 *
 *  @param s		The serialised object as a string.
 */
OmMatchOptions string_to_moptions(const string &s);

/** Convert an OmRSet object into a string representation.
 *
 *  @param omrset		The object to serialise.
 */
string omrset_to_string(const OmRSet &omrset);

/** Convert a serialised OmRSet string back into an object.
 *
 *  @param s		The serialised object as a string.
 */
OmRSet string_to_omrset(const string &s);

OmQueryInternal qfs_readquery();

/** returns true if the string s starts with prefix.
 */
bool startswith(const string &s, const string &prefix);

#endif /* OM_HGUARD_SOCKETCOMMON_H */
