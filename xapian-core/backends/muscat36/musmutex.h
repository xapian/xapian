/* musmutex.h: Header file for muscat mutex macros
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#ifndef OM_HGUARD_MUSMUTEX_H
#define OM_HGUARD_MUSMUTEX_H

/* Make header file work when included from C++ */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "config.h"

#ifdef MUS_USE_PTHREAD
#include <pthread.h>
#define MUS_PTHREAD_MUTEX(a) pthread_mutex_t a
#define MUS_PTHREAD_MUTEX_INIT(a) pthread_mutex_init(&(a), 0)
#define MUS_PTHREAD_MUTEX_DESTROY(a) pthread_mutex_destroy(&(a))
#define MUS_PTHREAD_MUTEX_LOCK(a) pthread_mutex_lock(&(a))
#define MUS_PTHREAD_MUTEX_UNLOCK(a) pthread_mutex_unlock(&(a))
#else /* MUS_USE_PTHREAD */
#define MUS_PTHREAD_MUTEX(a)
#define MUS_PTHREAD_MUTEX_INIT(a)
#define MUS_PTHREAD_MUTEX_DESTROY(a)
#define MUS_PTHREAD_MUTEX_LOCK(a)
#define MUS_PTHREAD_MUTEX_UNLOCK(a)
#endif /* MUS_USE_PTHREAD */


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* OM_HGUARD_MUSMUTEX_H */
