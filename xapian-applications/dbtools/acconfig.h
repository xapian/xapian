/* acconfig.h : config.h template
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
 * Copyright 2001 tangozebra ltd
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

#ifndef OM_HGUARD_DBTOOL_CONFIG_H
#define OM_HGUARD_DBTOOL_CONFIG_H
@TOP@

/* Name of package */
#undef PACKAGE

/* Version number of package */
#undef VERSION

@BOTTOM@
/* This clump of defines is for compatibility across libxml1 and 2.
 *  libxml2 and later versions of libxml1 should have these already.
 *  These should work for earlier libxml1 versions.
 */
#ifndef xmlChildrenNode
#define xmlChildrenNode childs
#define xmlRootNode root
#endif

#endif /* OM_HGUARD_DBTOOL_CONFIG_H */
