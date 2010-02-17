#include <config.h>

#include "fdtracker.h"

#include "safeunistd.h"
#include "safedirent.h"
#include "safeerrno.h"

#include <iostream>
#include <cstdlib>
#include <cstring> // For strerror().
#include <set>

#include "str.h"

using namespace std;

FDTracker::~FDTracker()
{
#ifndef __WIN32__
    DIR * dir = static_cast<DIR*>(dir_void);
    closedir(dir);
#endif
}

void
FDTracker::init()
{
#ifndef __WIN32__
    DIR * dir = opendir("/proc/self/fd");
    // Not all platforms have /proc/self/fd.
    if (!dir) return;
    dir_void = static_cast<void*>(dir);

    while (true) {
	errno = 0;
	struct dirent * entry = readdir(dir);
	if (!entry) {
	    if (errno == 0)
		break;
	    cout << "readdir failed: " << strerror(errno) << endl;
	    exit(1);
	}

	const char * name = entry->d_name;
	if (name[0] < '0' || name[0] > '9')
	    continue;

	int fd = atoi(name);
	fds.insert(fd);
    }
#endif
}

bool
FDTracker::check()
{
    bool ok = true;
#ifndef __WIN32__
    DIR * dir = static_cast<DIR*>(dir_void);
    if (!dir) return true;
    rewinddir(dir);

    message.resize(0);

    while (true) {
	errno = 0;
	struct dirent * entry = readdir(dir);
	if (!entry) {
	    if (errno == 0)
		break;
	    cout << "readdir failed: " << strerror(errno) << endl;
	    exit(1);
	}

	const char * name = entry->d_name;

	// Ignore at least '.' and '..'.
	if (name[0] < '0' || name[0] > '9')
	    continue;

	int fd = atoi(name);
	if (fds.find(fd) != fds.end()) continue;

	message += ' ';
	message += str(fd);

	string filename = "/proc/self/fd/";
	filename += name;

	char buf[1024];
	int res = readlink(filename.c_str(), buf, sizeof(buf));
	if (res > 0) {
	    message += " -> ";
	    message.append(buf, res);
	}

	// Insert the leaked fd so we don't report it for future tests.
	fds.insert(fd);
	ok = false;
    }
#endif
    return ok;
}


