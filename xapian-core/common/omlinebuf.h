/* omlinebuf.h: An abstract line buffer class
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003 Olly Betts
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

#ifndef OM_HGUARD_OMLINEBUF_H
#define OM_HGUARD_OMLINEBUF_H

#include <string>
#include "omtime.h"

/** The OmLineBuf class is an interface to a line-buffered
 *  stream used by the network backends.
 *
 *  @exception Xapian::NetworkTimeoutError is thrown if a timeout is exceeded
 *                                   waiting for remote input.
 *
 *  @exception Xapian::NetworkError is thrown for any non-timeout network error,
 *                            for example remote closed socket.
 */
class OmLineBuf {
    private:
	/// disallow copies
	OmLineBuf(const OmLineBuf &other);
	void operator=(const OmLineBuf &other);

	/** Read one line
	 *  @param end_time	The time at which the read will
	 *  			fail with a timeout error.
	 */
	virtual std::string do_readline(const OmTime & end_time) = 0;

	/** Write one line to writefd
	 */
	virtual void do_writeline(std::string s, const OmTime & end_time) = 0;

	std::string line_buffer;
    public:
	/** The main constructor.
	 */
	OmLineBuf() {}

	virtual ~OmLineBuf() {}

	/** Read one line
	 *  @param end_time	The time at which the read will
	 *  			fail with a timeout error.
	 */
	std::string readline(const OmTime & end_time);

	/** Return true if there is data available to be read
	 *  immediately.
	 */
	virtual bool data_waiting() = 0;

	/** Block until at least a line of data has been read.
	 */
	virtual void wait_for_data(int msecs = 0);

	/** Write one line to writefd
	 */
	void writeline(std::string s, const OmTime & end_time);
};

#endif // OM_HGUARD_OMLINEBUF_H
