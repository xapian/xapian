/* pool.h: Header for managing exception lists for stemming algorithms
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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
 *
 */

#ifndef _pool_h_
#define _pool_h_

/* Make header file work when included from C++ */
#ifdef __cplusplus
extern "C" {
#endif

struct pool {

    int size;
    struct pool_entry * entries;

};

extern struct pool * create_pool(char * s[]);
extern char * search_pool(struct pool * p, int length, char * s);
extern void free_pool(struct pool * p);

#ifdef __cplusplus
}
#endif

#endif /* _pool_h_ */
