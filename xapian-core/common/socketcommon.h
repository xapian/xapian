/* socketcommon.h: Various useful Socket{Server,Client}-related utilities
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

#ifndef OM_HGUARD_SOCKETCOMMON_H
#define OM_HGUARD_SOCKETCOMMON_H

#include <string>
#include <map>
#include "omlinebuf.h"
#include "om/omsettings.h"
#include "om/omenquire.h"

#define OM_SOCKET_PROTOCOL_VERSION 3

class OmQueryInternal;
class Stats;
class OmRSet;
class OmKey;
class OmMSet;

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
	std::string buffer;

	/// disallow copies
	OmSocketLineBuf(const OmSocketLineBuf &other);
	void operator=(const OmSocketLineBuf &other);

	/** Read one line from readfd
	 *  @param msecs_timeout	The timeout in milliseconds before
	 *  				throwing an exception.
	 */
	std::string do_readline(int msecs_timeout);

	/** Write one line to writefd
	 */
	void do_writeline(std::string s);
    public:
	/** The main constructor.  The arguments are the
	 *  input and output filedescriptors to use.
	 */
	OmSocketLineBuf(int readfd_, int writefd_);

	/** A convenience constructor which takes only one
	 *  fd, which can be both read from and written to.
	 */
	OmSocketLineBuf(int fd_);

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
};

/** Build a query from a serialised string.
 *  This uses a flex parser for the tokenizing.
 *
 *  @param qs  The string from which to build the query.
 */
OmQueryInternal query_from_string(std::string qs);

/** Convert a Stats object into a string representation.
 *
 *  @param  stats	The stats object to serialise.
 */
std::string stats_to_string(const Stats &stats);

/** Convert a string representing a Stats object back into an
 *  object.
 *
 *  @param  s		The serialised Stats object.
 */
Stats string_to_stats(const std::string &s);

/** Convert the match_ options from an OmSettings object into a string representation.
 *
 *  @param moptions	The object to serialise.
 */
std::string moptions_to_string(const OmSettings &moptions);

/** Convert a serialised OmSettings string back into an object.
 *
 *  @param s		The serialised object as a string.
 */
OmSettings string_to_moptions(const std::string &s);

/** Convert an OmRSet object into a string representation.
 *
 *  @param omrset		The object to serialise.
 */
std::string omrset_to_string(const OmRSet &omrset);

/** Convert a serialised OmRSet string back into an object.
 *
 *  @param s		The serialised object as a string.
 */
OmRSet string_to_omrset(const std::string &s);

/** Convert an OmMSet object into a string representation.
 *
 *  @param ommset		The object to serialise.
 */
std::string ommset_to_string(const OmMSet &ommset);

/** Convert a serialised OmMSet string back into an object.
 *
 *  @param s		The serialised object as a string.
 */
OmMSet string_to_ommset(const std::string &s);

/** Convert a terminfo map into a string representation.
 *
 *  @param terminfo		The terminfo map to serialise.
 */
std::string ommset_termfreqwts_to_string(const std::map<om_termname,
					 OmMSet::TermFreqAndWeight> &terminfo);

/** Convert a serialised terminfo string back into a map.
 *
 *  @param s		The seralised map as a string.
 */
std::map<om_termname, OmMSet::TermFreqAndWeight>
string_to_ommset_termfreqwts(const std::string &s);

/** Convert an OmKey object into a string representation.
 *
 *  @param omrset		The object to serialise.
 */
std::string omkey_to_string(const OmKey &omkey);

/** Convert a serialised OmKey string back into an object.
 *
 *  @param s		The serialised object as a string.
 */
OmKey string_to_omkey(const std::string &s);

OmQueryInternal qfs_readquery();

/** returns true if the string s starts with prefix.
 */
bool startswith(const std::string &s, const std::string &prefix);

#endif /* OM_HGUARD_SOCKETCOMMON_H */
