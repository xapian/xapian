/** @file
 * @brief Fallback implementation of mkdtemp()
 */
/* Copyright 2013,2025 Olly Betts
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

#ifndef OMEGA_INCLUDED_MKDTEMP_H
#define OMEGA_INCLUDED_MKDTEMP_H

#ifndef PACKAGE
# error You must #include <config.h> before #include "mkdtemp.h"
#endif

#include <stdlib.h>

#ifndef HAVE_MKDTEMP
char* fallback_mkdtemp(char *);

#define mkdtemp(TEMPLATE) fallback_mkdtemp(TEMPLATE)
#endif

#endif
