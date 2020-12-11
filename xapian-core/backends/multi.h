/** @file
 * @brief Multi-database support functions
 */
/* Copyright (C) 2017 Olly Betts
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

#ifndef XAPIAN_INCLUDED_MULTI_H
#define XAPIAN_INCLUDED_MULTI_H

#include <xapian/types.h>
#include "omassert.h"

/** Convert docid in the multi-db to the docid in the shard.
 *
 *  @param did		docid in the multi-db
 *  @param n_shards	number of dbs in the multi-db
 *
 *  @return docid in the shard
 */
inline Xapian::docid
shard_docid(Xapian::docid did, Xapian::doccount n_shards) {
    Assert(did != 0);
    Assert(n_shards != 0);
    return (did - 1) / n_shards + 1;
}

/** Convert docid in the multi-db to shard number.
 *
 *  @param did		docid in the multi-db
 *  @param n_shards	number of dbs in the multi-db
 *
 *  @return shard number between 0 and (n_shards - 1) inclusive
 */
inline Xapian::doccount
shard_number(Xapian::docid did, Xapian::doccount n_shards) {
    Assert(did != 0);
    Assert(n_shards != 0);
    return Xapian::doccount((did - 1) % n_shards);
}

/** Convert shard number and shard docid to docid in multi-db.
 *
 *  @param shard_did	docid in the shard
 *  @param shard	shard number between 0 and (n_shards - 1) inclusive
 *  @param n_shards	number of dbs in the multi-db
 *
 *  @return docid in the multi-db.
 */
inline Xapian::docid
unshard(Xapian::docid shard_did,
	Xapian::doccount shard,
	Xapian::doccount n_shards) {
    Assert(shard_did != 0);
    AssertRel(shard,<,n_shards);
    return (shard_did - 1) * n_shards + shard + 1;
}

#endif // XAPIAN_INCLUDED_MULTI_H
