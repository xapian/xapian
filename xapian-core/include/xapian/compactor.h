/** @file compactor.h
 * @brief Compact a database, or merge and compact several.
 */
/* Copyright (C) 2003,2004,2005,2006,2007,2008,2009,2010 Olly Betts
 * Copyright (C) 2008 Lemur Consulting Ltd
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

#ifndef XAPIAN_INCLUDED_COMPACTOR_H
#define XAPIAN_INCLUDED_COMPACTOR_H

#include <xapian/base.h>
#include <xapian/visibility.h>
#include <string>

namespace Xapian {

/** Compact a database, or merge and compact several.
 */
class XAPIAN_VISIBILITY_DEFAULT Compactor {
  public:
    /// Class containing the implementation.
    class Internal;

    typedef enum { STANDARD, FULL, FULLER } compaction_level;

  private:
    /// @internal Reference counted internals.
    Xapian::Internal::RefCntPtr<Internal> internal;

  public:
    Compactor();

    virtual ~Compactor();

    void set_block_size(size_t block_size);
    void set_renumber(bool renumber);
    void set_multipass(bool multipass);
    void set_compaction_level(compaction_level compaction);
    void set_destdir(const std::string & destdir);
    void add_source(const std::string & srcdir);
    void compact();

    virtual void
    set_status(const std::string & table, const std::string & status);

    virtual std::string
    resolve_duplicate_metadata(const std::string & key,
			       const std::string & tag1,
			       const std::string & tag2);
};

}

#endif /* XAPIAN_INCLUDED_COMPACTOR_H */
