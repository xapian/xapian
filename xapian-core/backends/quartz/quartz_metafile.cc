/* quartz_metafile.cc: Management of quartz meta-file
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

#include "config.h"
#include <string>
#include "quartz_metafile.h"
#include "btree_util.h"
#include "omassert.h"

static const std::string metafile_magic = "OMMETA";
static const unsigned int metafile_version = 1;

#ifndef MUS_DEBUG  // FIXME: CompiletimeAssert is in an #ifndef MUS_DEBUG in omassert.h
CompiletimeAssert(sizeof(metafile_version == 4))
#endif

static const size_t min_metafile_size = metafile_magic.length() +
					sizeof(metafile_version);

static const size_t max_metafile_size = min_metafile_size;

QuartzMetaFile::QuartzMetaFile(const std::string &filename_)
	: filename(filename_)
{
}

QuartzMetaFile::~QuartzMetaFile()
{
}

static std::string encode_version(unsigned int version)
{
    std::string data;

    for (size_t i=0; i<sizeof(metafile_version); ++i) {
	data += (char)(version & 0xff);
	version >>= 8;
    }

    return data;
}

static unsigned int decode_version(const std::string &s)
{
    unsigned int version = 0;

    for (size_t i = 1;
	 i <= sizeof(metafile_version);
	 ++i) {
	version = (version << 8) + s[sizeof(metafile_version) - i];
    }

    return version;
}

void QuartzMetaFile::open()
{
    int fd = sys_open_to_read(filename);
    fdcloser fdc(fd);

    std::string data = sys_read_all_bytes(fd, min_metafile_size + 1);

    if (data.length() < min_metafile_size) {
	throw OmDatabaseCorruptError("Quartz metafile " + filename +
				     " too short; may be truncated.");
    }

    if (data.substr(0, metafile_magic.length()) != metafile_magic) {
	throw OmDatabaseCorruptError("Quartz metafile " + filename +
				     " is invalid: magic string not found.");
    }

    unsigned int version = decode_version(data.substr(metafile_magic.length(),
						   sizeof(metafile_version)));
    if (version != metafile_version) {
	throw OmOpeningError("Unknown Quartz metafile version " +
			     om_tostring(version) + " in " +
			     filename);
    }

    if (data.length() > max_metafile_size) {
	throw OmDatabaseCorruptError("Quartz metafile " + filename +
				     " contains extra garbage.");
    }
}

void QuartzMetaFile::create()
{
    int fd = sys_open_to_write(filename);

    fdcloser fdc(fd);

    std::string data = metafile_magic;

    data += encode_version(metafile_version);

    sys_write_bytes(fd, data.length(), data.data());
}

void QuartzMetaFile::erase()
{
    sys_unlink_if_exists(filename);
}
