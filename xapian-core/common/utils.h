/* utils.h: Various useful utilities
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
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

#ifndef OM_HGUARD_UTILS_H
#define OM_HGUARD_UTILS_H

#include <string>
using std::string;
#include <vector>
using std::vector;

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>

#include <fcntl.h>

#ifdef __WIN32__
#include <windows.h>
#ifndef FOF_NOERRORUI
#define FOF_NOERRORUI 1024
#endif
#undef min
#undef max
#endif

// Sun's fcntl.h pollutes the namespace by #define-ing open to open64 when
// largefile support is enabled (also creat to creat64 but that's not a problem
// for us).  So we do a little dance to mop up this pollution.  Otherwise we'd
// have to rename all our open methods.
inline int fcntl_open(const char *filename, int flags, mode_t mode) {
#ifdef __SUNPRO_CC
    // cast needed to resolve overload ambiguity
    return open(filename, flags, (int)mode);
#else
    return open(filename, flags, mode);
#endif
}

inline int fcntl_open(const char *filename, int flags) {
    return open(filename, flags);
}

#ifdef open
#undef open
#endif

inline int open(const string &filename, int flags, mode_t mode) {
    return fcntl_open(filename.c_str(), flags, mode);
}

// If filename is a char* and mode is an int, we get ambiguity warnings
// when the compiler tries to pick which overloaded open function to use
// - this avoids that problem.
inline int open(const char *filename, int flags, int mode) {
    return fcntl_open(filename, flags, (mode_t)mode);
}

inline int open(const string &filename, int flags) {
    return fcntl_open(filename.c_str(), flags);
}

inline string om_tostring(const string &s) { return s; }

/// Convert an integer to a string
string om_tostring(int a);

/// Convert an unsigned integer to a string
string om_tostring(unsigned int a);

/// Convert a long integer to a string
string om_tostring(long int a);

/// Convert an unsigned long integer to a string
string om_tostring(unsigned long int a);

/// Convert a double to a string
string om_tostring(double a);

/// Convert a bool to a string
string om_tostring(bool a);

/// Convert a pointer to a string
string om_tostring(const void * a);

/** Split a string into a vector of strings, using a given separator
 *  character (default space)
 */
void split_words(string text,
		 vector<string> &words,
		 char wspace = ' ');

///////////////////////////////////////////
// Mapping of types as strings to enums  //
///////////////////////////////////////////

struct StringAndValue {
    const char * name;
    int value;
};

/** Get the value associated with the given string.  If the string
 *  isn't found, the value returned is the value in the terminating
 *  object (which has a zero length string).
 *
 *  Note: this just uses a list of entries, and searches linearly
 *  through them.  Could at make this do a binary chop, but probably
 *  not worth doing so, unless list gets large.
 */
int map_string_to_value(const StringAndValue * haystack, const string & needle);

/** Return true if the file fname exists.
 */
bool file_exists(const string &fname);

/** Return true if all the files fnames exist.
 */
bool files_exist(const vector<string> &fnames);

/// Allow atoi to work directly on C++ strings.
inline int atoi(const string &s) { return atoi(s.c_str()); }

/// Allow unlink to work directly on C++ strings.
inline int unlink(const string &filename) { return unlink(filename.c_str()); }

/// Allow system to work directly on C++ strings.
inline int system(const string &command) { return system(command.c_str()); }

/// Allow link to work directly on C++ strings.
inline int link(const string &o, const string &n) {
    return link(o.c_str(), n.c_str());
}

/// Allow mkdir to work directly on C++ strings.
inline int mkdir(const string &filename, mode_t mode) {
#ifdef __WIN32__
    (void)mode; // Ignored by win32
    return mkdir(filename.c_str());
#else
    return mkdir(filename.c_str(), mode);
#endif
}

/// Allow stat to work directly on C++ strings.
inline int stat(const string &filename, struct stat *buf) {
    return stat(filename.c_str(), buf);
}

/// Touch a file.
inline void touch(const string &filename) {
   int fd = open(filename.c_str(), O_CREAT|O_WRONLY, 0644);
   if (fd >= 0) close(fd);
}

/// Remove a directory and contents.
inline void rmdir(const string &filename) {
    // Check filename exists and is actually a directory
    struct stat sb;
    if (stat(filename, &sb) != 0 || !S_ISDIR(sb.st_mode)) return;

    string safefile = filename;
# ifdef __WIN32__
    if (getenv("USE_SHFILEOPSTRUCT") == 0) {
	string::iterator i;
	// Check for illegal characters in filename
	for (i = safefile.begin(); i != safefile.end(); ++i) {
	    if (*i == '/') {
		*i = '\\';
	    } else if (*i < 32 || strchr("<>\"|*?", *i)) {
		return;
	    }
	}

	static int win95 = -1;
	if (win95 == -1) {
	    OSVERSIONINFO info;
	    memset(&info, 0, sizeof(OSVERSIONINFO));
	    info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	    if (GetVersionEx(&info)) {
		win95 = (info.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS);
	    }
	}

	if (win95) {
	    // for 95 like systems:
	    system("deltree /y \"" + safefile + "\"");
	} else {
	    // for NT like systems:
	    system("rd /s /q \"" + safefile + "\"");
	}
    } else {
	safefile.append("\0", 2);
	SHFILEOPSTRUCT shfo;
	memset((void*)&shfo, 0, sizeof(shfo));
	shfo.hwnd = 0;
	shfo.wFunc = FO_DELETE;
	shfo.pFrom = safefile.data();
	shfo.fFlags = FOF_NOCONFIRMATION|FOF_NOERRORUI|FOF_SILENT;
	(void)SHFileOperation(&shfo);
    }
# else
    string::size_type p = 0;
    while (p < safefile.size()) {
	// Don't escape a few safe characters which are common in filenames
	if (!isalnum(safefile[p]) && strchr("/._-", safefile[p]) == NULL) {
	    safefile.insert(p, "\\");
	    ++p;
	}
	++p;
    }
    system("rm -rf " + safefile);
# endif
}

# ifdef __WIN32__
inline unsigned int sleep(unsigned int secs) {
    _sleep(secs * 1000);
    return 0;
}
# endif

#endif /* OM_HGUARD_UTILS_H */
