/* l1.h -- interface to a trivial library
   Copyright (C) 1998-1999 Thomas Tanner <tanner@ffii.org>
   This file is part of GNU Libtool.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
USA. */

/* Only include this header file once. */
#ifndef _L1_H_
#define _L1_H_ 1

#include "sysdep.h"

__BEGIN_DECLS
extern int var_l1;
int	func_l1 __P((int));
__END_DECLS

#endif /* !_L1_H_ */
