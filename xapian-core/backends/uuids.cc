/** @file
 * @brief Class for handling UUIDs
 */
/* Copyright (C) 2008 Lemur Consulting Ltd
 * Copyright (C) 2013,2015,2016,2017,2018,2026 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see
 * <https://www.gnu.org/licenses/>.
 */

#include <config.h>

#include "uuids.h"

#include "xapian/error.h"

#include <cerrno>
#include <cstring>
#include "stringutils.h"

#include <sys/types.h>
#include "safefcntl.h"
#include "safeunistd.h"

#ifdef HAVE_ARC4RANDOM_BUF
# include <stdlib.h>
#endif

#ifdef USE_PROC_FOR_UUID
# include "safesysstat.h"
#elif defined HAVE_UUID_UUID_H
# include <exception>
# include <uuid/uuid.h>
#elif defined HAVE_UUID_H
// UUID API on FreeBSD, NetBSD, OpenBSD and AIX.
# include <arpa/inet.h> // For htonl() and htons().
# include <exception>
# include <uuid.h>
#elif defined USE_WIN32_UUID_API
# include "safewindows.h"
# include <rpc.h>
# ifdef __WIN32__
#  include "safewinsock2.h" // For htonl() and htons().
# else
// Cygwin:
#  include <arpa/inet.h> // For htonl() and htons().
# endif
#endif

using namespace std;

/// Bit-mask to determine where to put hyphens in the string representation.
static constexpr unsigned UUID_GAP_MASK = 0x2a8;

void
Uuid::generate()
{
    // If the platform provides an API to get cryptographically secure random
    // data we just fill a buffer and then set/clear the appropriate bits to
    // turn it into a valid randomly-generated UUID.
    //
    // This avoids needing external libraries, using platform-specific APIs
    // or reading magic files in /proc.
    //
    // We could use std::random_device here, except:
    //
    //   std::random_device may be implemented in terms of an
    //   implementation-defined pseudo-random number engine if a
    //   non-deterministic source (e.g. a hardware device) is not available to
    //   the implementation.
    //
    // `std::random_device::entropy()` should allow us to tell but can't be
    // trusted due to various bad real-world implementations:
    // https://en.cppreference.com/cpp/numeric/random/random_device/entropy#Notes

    bool filled_with_randomness = false;
#if defined HAVE_ARC4RANDOM_BUF
    // Apparently available on:
    // * Android (all API levels)
    // * DragonFly 1.0
    // * FreeBSD 8.0
    // * glibc 2.36
    // * macOS
    // * NetBSD 1.6
    // * OpenBSD 2.1
    arc4random_buf(uuid_data, BINARY_SIZE);
    filled_with_randomness = true;
# define TRIED_RANDOMNESS
#elif defined HAVE_ARC4RANDOM
    // Apparently available before arc4random_buf() on some platforms, e.g.:
    // * FreeBSD 3.0
    static_assert(BINARY_SIZE % 4 == 0, "UUID binary size not multiple of 4");
    for (unsigned i = 0; i < BINARY_SIZE; i += 4) {
	uint32_t v = arc4random();
	memcpy(uuid_data + i, v, 4);
    }
    filled_with_randomness = true;
# define TRIED_RANDOMNESS
#elif defined HAVE_GETENTROPY
    // Specified by POSIX.1-2024, but has not been supported for as long
    // as arc4random_buf()/arc4random() on most platforms.  A notable exception
    // is glibc which has supported getentropy() since 2.25.
    if (getentropy(uuid_data, BINARY_SIZE) == 0) {
        filled_with_randomness = true;
    }
# define TRIED_RANDOMNESS
#endif
    if (filled_with_randomness) {
        uuid_data[6] = (uuid_data[6] & 0x0f) | 0x40; // version 4
        uuid_data[8] = (uuid_data[8] & 0x3f) | 0x80; // RFC 4122
        return;
    }

#ifdef USE_PROC_FOR_UUID
    /* Linux (since 2.3.16) has /proc/sys/kernel/random/uuid which generates
     * a new UUID each time it is read and returns it in string form.
     *
     * Some significant downsides of this are that it needs /proc to be
     * mounted, it requires an unused fd, and access might be blocked by
     * SELinux or similar (e.g. AOSP SELinux policy only allows access starting
     * with Android 9).
     */
    char buf[STRING_SIZE];
    int fd = open("/proc/sys/kernel/random/uuid", O_RDONLY);
    if (rare(fd == -1)) {
        throw Xapian::DatabaseCreateError("Opening UUID generator failed", errno);
    }
    bool failed = (read(fd, buf, STRING_SIZE) != STRING_SIZE);
    close(fd);
    if (failed) {
        throw Xapian::DatabaseCreateError("Generating UUID failed");
    }
    parse(buf);
#elif defined HAVE_UUID_UUID_H
    uuid_t uu;
    uuid_generate(uu);
    memcpy(uuid_data, &uu, BINARY_SIZE);
#elif defined HAVE_UUID_H
    uuid_t uu;
    uint32_t status;
    uuid_create(&uu, &status);
    if (status != uuid_s_ok) {
        // Can only be uuid_s_no_memory it seems.
        throw std::bad_alloc();
    }
    uu.time_low = htonl(uu.time_low);
    uu.time_mid = htons(uu.time_mid);
    uu.time_hi_and_version = htons(uu.time_hi_and_version);
    memcpy(uuid_data, &uu, BINARY_SIZE);
#elif defined USE_WIN32_UUID_API
    UUID uuid;
    if (rare(UuidCreate(&uuid) != RPC_S_OK)) {
        // Throw a DatabaseCreateError, since we can't make a UUID.  The
        // windows API documentation is a bit unclear about the situations in
        // which this can happen.
        throw Xapian::DatabaseCreateError("Cannot create UUID");
    }
    uuid.Data1 = htonl(uuid.Data1);
    uuid.Data2 = htons(uuid.Data2);
    uuid.Data3 = htons(uuid.Data3);
    memcpy(uuid_data, &uuid, BINARY_SIZE);
#elif defined TRIED_RANDOMNESS
    throw Xapian::DatabaseCreateError("Generating UUID failed");
#else
# error Do not know how to generate UUIDs
#endif
}

void
Uuid::parse(const char* in)
{
    for (unsigned i = 0; i != BINARY_SIZE; ++i) {
        uuid_data[i] = hex_decode(in[0], in[1]);
        in += ((UUID_GAP_MASK >> i) & 1) | 2;
    }
}

string
Uuid::to_string() const
{
    string result;
    result.reserve(STRING_SIZE);
    for (unsigned i = 0; i != BINARY_SIZE; ++i) {
        unsigned char ch = uuid_data[i];
        result += "0123456789abcdef"[ch >> 4];
        result += "0123456789abcdef"[ch & 0x0f];
        if ((UUID_GAP_MASK >> i) & 1)
           result += '-';
    }
    return result;
}
