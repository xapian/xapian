/** @file
 * @brief Communication with worker processes
 */
/* Copyright (C) 2011,2022,2025 Olly Betts
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

#ifndef PACKAGE
# error config.h must be included first in each C++ source file
#endif

#include <cstdio>
#include <string>

#ifdef HAVE_FREAD_UNLOCKED
# define FREAD(P, S, N, F) fread_unlocked(P, S, N, F)
#else
# define FREAD(P, S, N, F) fread(P, S, N, F)
#endif
#ifdef HAVE_FWRITE_UNLOCKED
# define FWRITE(P, S, N, F) fwrite_unlocked(P, S, N, F)
#else
# define FWRITE(P, S, N, F) fwrite(P, S, N, F)
#endif
#ifdef HAVE_GETC_UNLOCKED
# define GETC(F) getc_unlocked(F)
#else
# define GETC(F) getc(F)
#endif
#ifdef HAVE_PUTC_UNLOCKED
# define PUTC(C, F) putc_unlocked(C, F)
#else
# define PUTC(C, F) putc(C, F)
#endif

/// Read a string from the file descriptor @a f and storage it in @a s.
bool read_string(std::FILE* f, std::string& s);

/// Write the string @a p of length @a len to the file descriptor @a f.
bool write_string(std::FILE* f, const char* p, size_t len);

/// Write the string @a p of length @a len to the file descriptor @a f.
inline bool write_string(std::FILE* f, const std::string& s) {
    return write_string(f, s.data(), s.size());
}

bool read_unsigned(std::FILE* f, unsigned& v);

bool read_unsigned(std::FILE* f, unsigned long& v);

bool write_unsigned(std::FILE* f, unsigned v);

bool write_unsigned(std::FILE* f, unsigned long v);

// Exit codes for situations not enumerated by <sysexits.h>.
enum {
    OMEGA_EX_SOCKET_READ_ERROR = 10,
    OMEGA_EX_SOCKET_WRITE_ERROR
};
