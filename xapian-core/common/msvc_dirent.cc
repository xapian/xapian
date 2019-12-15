/** @file
 *  @brief Implementation of dirent functions using WIN32 API.
 */
/*
    Implementation of POSIX directory browsing functions and types for Win32.

    Author:  Kevlin Henney (kevlin@acm.org, kevlin@curbralan.com)
    History: Created March 1997. Updated June 2003.
    Fixes since importing into Xapian:
    2008-03-04 Fixed readdir() not to set errno to ENOENT when the
	       end of the directory is reached.
    2018-04-01 Fix handle to be intptr_t not long to avoid truncation
	       with WIN64 (where long is still 32 bits).
    2019-12-16 Make dir->name a one element array member and over-allocate
	       the struct so there's enough room for its actual size.

    Copyright Kevlin Henney, 1997, 2003. All rights reserved.

    Permission to use, copy, modify, and distribute this software and its
    documentation for any purpose is hereby granted without fee, provided
    that this copyright and permissions notice appear in all copies and
    derivatives.
    
    This software is supplied "as is" without express or implied warranty.

    But that said, if there are any problems please get in touch.
*/

#include <config.h>
#ifdef __WIN32__

#include "msvc_dirent.h"
#include <cerrno>
#include <io.h> /* _findfirst and _findnext set errno iff they return -1 */
#include <cstdlib>
#include <cstring>

#ifdef __cplusplus
extern "C"
{
#endif

struct DIR
{
    intptr_t            handle; /* -1 for failed rewind */
    struct _finddata_t  info;
    struct dirent       result; /* d_name null iff first time */
    char                name[1];/* null-terminated char string */
};

DIR *opendir(const char *name)
{
    DIR *dir = 0;

    if(name && name[0])
    {
        size_t base_length = strlen(name);
        /* 2 allows for appending `/` and `*`.  We don't need to allow
         * 1 for the nul here as there will always be at least one byte
         * in struct DIR for name (plus padding). */
        size_t alloc_size = sizeof(DIR) + base_length + 2;

        if((dir = (DIR *) malloc(alloc_size)) != 0)
        {
            memcpy(dir->name, name, base_length);
            /* Search pattern must end with suitable wildcard */
            if(name[base_length - 1] != '/' && name[base_length - 1] != '\\')
                dir->name[base_length++] = '/';
            memcpy(dir->name + base_length, "*", 2);

            if((dir->handle = _findfirst(dir->name, &dir->info)) != -1)
            {
                dir->result.d_name = 0;
            }
            else /* rollback */
            {
                /* _findfirst() will have set errno suitably. */
                free(dir);
                dir = 0;
            }
        }
        else /* rollback */
        {
            free(dir);
            dir   = 0;
            errno = ENOMEM;
        }
    }
    else
    {
        errno = EINVAL;
    }

    return dir;
}

int closedir(DIR *dir)
{
    int result = -1;

    if(dir)
    {
        if(dir->handle != -1)
        {
            result = _findclose(dir->handle);
        }

        free(dir);
    }

    if(result == -1) /* map all errors to EBADF */
    {
        errno = EBADF;
    }

    return result;
}

struct dirent *readdir(DIR *dir)
{
    struct dirent *result = 0;

    if(dir && dir->handle != -1)
    {
	if(!dir->result.d_name) {
	    result = &dir->result;
	    result->d_name = dir->info.name;
	} else {
	    int orig_errno = errno;
	    if (_findnext(dir->handle, &dir->info) != -1) {
		result = &dir->result;
		result->d_name = dir->info.name;
	    } else if (errno == ENOENT) {
		// _findnext sets errno to ENOENT when the end of the directory
		// is reached.  However, according to POSIX, the value of errno
		// should not be changed by this condition.  Therefore, we have
		// to set it back to the original value.
		errno = orig_errno;
	    }
	}
    }
    else
    {
        errno = EBADF;
    }

    return result;
}

void rewinddir(DIR *dir)
{
    if(dir && dir->handle != -1)
    {
        _findclose(dir->handle);
        dir->handle = _findfirst(dir->name, &dir->info);
        dir->result.d_name = 0;
    }
    else
    {
        errno = EBADF;
    }
}

#ifdef __cplusplus
}
#endif

#endif
