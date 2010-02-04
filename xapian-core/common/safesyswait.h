/** @file safesyswait.h
 * @brief #include <sys/wait.h>, with portability stuff.
 */
/* Copyright (C) 2010 Olly Betts
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

#ifndef XAPIAN_INCLUDED_SAFESYSWAIT_H
#define XAPIAN_INCLUDED_SAFESYSWAIT_H

#ifndef __WIN32__
# include <sys/wait.h>
#else
// We don't try to replace waitpid(), etc - they're only useful for us when
// we can fork().  But it's handy to be able to use WIFEXITED() and
// WEXITSTATUS().
# ifndef WIFEXITED
#  define WIFEXITED(STATUS) (STATUS != -1)
# endif
# ifndef WEXITSTATUS
#  define WEXITSTATUS(STATUS) (STATUS)
# endif
#endif

#endif /* XAPIAN_INCLUDED_SAFESYSWAIT_H */
