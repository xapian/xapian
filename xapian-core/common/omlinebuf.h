/* omlinebuf.h: An abstract line buffer class
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

#ifndef OM_HGUARD_OMLINEBUF_H
#define OM_HGUARD_OMLINEBUF_H

#include <string>

/** The OmLineBuf class is an interface to a line-buffered
 *  stream used by the network backends.
 *
 *  @exception OmNetworkTimeoutError is thrown if a timeout is exceeded
 *                                   waiting for remote input.
 *
 *  @exception OmNetworkError is thrown for any non-timeout network error,
 *                            for example remote closed socket.
 */
class OmLineBuf {
    private:
	/// disallow copies
	OmLineBuf(const OmLineBuf &other);
	void operator=(const OmLineBuf &other);

	/** Read one line
	 */
	virtual string do_readline() = 0;

	/** Write one line to writefd
	 */
	virtual void do_writeline(string s) = 0;

	string line_buffer;
    public:
	/** The main constructor.
	 */
	OmLineBuf() {};

	virtual ~OmLineBuf() {};

	/** Read one line
	 */
	string readline();

	/** Return true if there is data available to be read 
	 *  immediately.
	 */
	virtual bool data_waiting() = 0;

	/** Block until at least a line of data has been read.
	 */
	virtual void wait_for_data();

	/** Write one line to writefd
	 */
	void writeline(string s);
};

#endif // OM_HGUARD_OMLINEBUF_H
