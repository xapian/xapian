/** @file
 * @brief Compact a database, or merge and compact several.
 */
/* Copyright (C) 2003,2004,2005,2006,2007,2008,2009,2010,2011,2013,2014,2015,2018 Olly Betts
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

#if !defined XAPIAN_IN_XAPIAN_H && !defined XAPIAN_LIB_BUILD
# error Never use <xapian/compactor.h> directly; include <xapian.h> instead.
#endif

#include <xapian/constants.h>
#include <xapian/visibility.h>
#include <string>

namespace Xapian {

class Database;

/** Compact a database, or merge and compact several.
 */
class XAPIAN_VISIBILITY_DEFAULT Compactor {
  public:
    /** Compaction level. */
    typedef enum {
	/** Don't split items unnecessarily. */
	STANDARD = 0,
	/** Split items whenever it saves space (the default). */
	FULL = 1,
	/** Allow oversize items to save more space (not recommended if you
	 *  ever plan to update the compacted database). */
	FULLER = 2
    } compaction_level;

    Compactor() {}

    virtual ~Compactor();

    /** Update progress.
     *
     *  Subclass this method if you want to get progress updates during
     *  compaction.  This is called for each table first with empty status,
     *  And then one or more times with non-empty status.
     *
     *  The default implementation does nothing.
     *
     *  @param table	The table currently being compacted.
     *  @param status	A status message.
     */
    virtual void
    set_status(const std::string & table, const std::string & status);

    /** Resolve multiple user metadata entries with the same key.
     *
     *  When merging, if the same user metadata key is set in more than one
     *  input, then this method is called to allow this to be resolving in
     *  an appropriate way.
     *
     *  The default implementation just returns tags[0].
     *
     *  For multipass this will currently get called multiple times for the
     *  same key if there are duplicates to resolve in each pass, but this
     *  may change in the future.
     *
     *  Since 1.4.6, an implementation of this method can return an empty
     *  string to indicate that the appropriate result is to not set a value
     *  for this user metadata key in the output database.  In older versions,
     *  you should not return an empty string.
     *
     *  @param key	The metadata key with duplicate entries.
     *  @param num_tags	How many tags there are.
     *  @param tags	An array of num_tags strings containing the tags to
     *			merge.
     */
    virtual std::string
    resolve_duplicate_metadata(const std::string & key,
			       size_t num_tags, const std::string tags[]);
};

}

#endif /* XAPIAN_INCLUDED_COMPACTOR_H */
