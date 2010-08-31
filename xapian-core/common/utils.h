/* utils.h: Various useful utilities
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2004,2005,2006,2007,2009 Olly Betts
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

#ifndef OM_HGUARD_UTILS_H
#define OM_HGUARD_UTILS_H

#include <xapian/visibility.h>

#include <string>
using std::string;

#include <cstdlib>
#include <sys/types.h>
#include "safesysstat.h"
#include "safeunistd.h"

/** Return true if the file fname exists.
 */
XAPIAN_VISIBILITY_DEFAULT
bool file_exists(const string &fname);

/** Return true if the directory dirname exists.
 */
XAPIAN_VISIBILITY_DEFAULT
bool dir_exists(const string &dirname);

/// Allow unlink to work directly on C++ strings.
inline int unlink(const string &filename) { return unlink(filename.c_str()); }

/// Allow system to work directly on C++ strings.
inline int system(const string &command) { return system(command.c_str()); }

/// Allow mkdir to work directly on C++ strings.
inline int mkdir(const string &filename, mode_t mode) {
    return mkdir(filename.c_str(), mode);
}

/// Allow stat to work directly on C++ strings.
inline int stat(const string &filename, struct stat *buf) {
    return stat(filename.c_str(), buf);
}

/** Remove a directory, and its contents.
 *
 *  If dirname doesn't refer to a file or directory, no error is generated.
 *
 *  Note - this doesn't currently cope with directories which contain
 *  subdirectories.
 */
void removedir(const string &dirname);

namespace Xapian {
    namespace Internal {
	bool within_DBL_EPSILON(double a, double b);
    }
}

/** A tiny class used to close a filehandle safely in the presence
 *  of exceptions.
 */
class fdcloser {
    public:
	fdcloser(int fd_) : fd(fd_) {}
	~fdcloser() {
	    if (fd >= 0) {
		(void)close(fd);
	    }
	}
    private:
	int fd;
};

#endif /* OM_HGUARD_UTILS_H */
