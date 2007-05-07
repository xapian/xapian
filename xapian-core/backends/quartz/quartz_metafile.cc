/* quartz_metafile.cc: Management of quartz meta-file
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004 Olly Betts
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
#include <string>
#include "quartz_metafile.h"
#include "btree_util.h"
#include "omassert.h"
#include "omdebug.h"

using std::string;

static const string metafile_magic = "OMMETA";
static const unsigned int metafile_version = 1;

static const size_t min_metafile_size = metafile_magic.length() + 4;

static const size_t max_metafile_size = min_metafile_size;

static string encode_version(unsigned int version)
{
    string data;

    for (size_t i = 0; i < 4; ++i) {
	data += char(version);
	version >>= 8;
    }

    return data;
}

static unsigned int decode_version(const string &s)
{
    unsigned int version = 0;

    for (size_t i = 1; i <= 4; ++i) {
	version = (version << 8) + s[4 - i];
    }

    return version;
}

void QuartzMetaFile::open()
{
    int fd = sys_open_to_read(filename);
    string data = sys_read_all_bytes(fd, min_metafile_size + 1);
    sys_close(fd);

    if (data.length() < min_metafile_size) {
	throw Xapian::DatabaseCorruptError("Quartz metafile " + filename +
				     " too short; may be truncated.");
    }

    if (data.substr(0, metafile_magic.length()) != metafile_magic) {
	throw Xapian::DatabaseCorruptError("Quartz metafile " + filename +
				     " is invalid: magic string not found.");
    }

    unsigned int version;
    version = decode_version(data.substr(metafile_magic.length(), 4));
    if (version != metafile_version) {
	throw Xapian::DatabaseVersionError("Unknown Quartz metafile version " +
			     om_tostring(version) + " in " +
			     filename);
    }

    if (data.length() > max_metafile_size) {
	throw Xapian::DatabaseCorruptError("Quartz metafile " + filename +
				     " contains extra garbage.");
    }
}

void QuartzMetaFile::create()
{
    string data = metafile_magic;
    data += encode_version(metafile_version);

    int fd = sys_open_to_write(filename);
    sys_write_string(fd, data);
    sys_close(fd);
}
