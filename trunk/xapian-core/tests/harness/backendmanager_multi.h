/** @file backendmanager_multi.h
 * @brief BackendManager subclass for multi databases.
 */
/* Copyright (C) 2007,2009 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_BACKENDMANAGER_MULTI_H
#define XAPIAN_INCLUDED_BACKENDMANAGER_MULTI_H

#include "backendmanager.h"

#include <string>

#include <xapian/types.h>
#include <xapian/postingiterator.h>

/// BackendManager subclass for multi databases.
class BackendManagerMulti : public BackendManager {
    /// The type to use for the sub-databases.
    std::string subtype;

    /// Don't allow assignment.
    void operator=(const BackendManagerMulti &);

    /// Don't allow copying.
    BackendManagerMulti(const BackendManagerMulti &);

    std::string createdb_multi(const std::vector<std::string> & files);

  protected:
    /// Get the path of the Xapian::Database instance.
    std::string do_get_database_path(const std::vector<std::string> & files);

  public:
    BackendManagerMulti(const std::string & subtype_);

    /// Return a string representing the current database type.
    std::string get_dbtype() const;

    /// Create a Multi Xapian::WritableDatabase object indexing a single file.
    Xapian::WritableDatabase get_writable_database(const std::string & name, const std::string & file);
};

#endif // XAPIAN_INCLUDED_BACKENDMANAGER_MULTI_H
