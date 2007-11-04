/** @file backendmanager_quartz.h
 * @brief BackendManager subclass for quartz databases.
 */
/* Copyright (C) 2007 Olly Betts
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

#ifndef XAPIAN_INCLUDED_BACKENDMANAGER_QUARTZ_H
#define XAPIAN_INCLUDED_BACKENDMANAGER_QUARTZ_H

#include "backendmanager.h"

#include <string>

#include <xapian/base.h>
#include <xapian/types.h>
#include <xapian/postingiterator.h>

#include "positionlist.h"

/// BackendManager subclass for quartz databases.
class BackendManagerQuartz : public BackendManager {
    /// Don't allow assignment.
    void operator=(const BackendManagerQuartz &);

    /// Don't allow copying.
    BackendManagerQuartz(const BackendManagerQuartz &);

  public:
    BackendManagerQuartz() { }

    /** We have virtual methods and want to be able to delete derived classes
     *  using a pointer to the base class, so we need a virtual destructor.
     */
    virtual ~BackendManagerQuartz();

    /// Return a string representing the current database type.
    const char * get_dbtype() const;

    /// Create a Quartz Xapian::Database object indexing multiple files.
    Xapian::Database get_database(const std::vector<std::string> & files);

    /// Create a Quartz Xapian::Database object indexing a single file.
    Xapian::Database get_database(const std::string & file);

    /// Create a Quartz Xapian::WritableDatabase object indexing a single file.
    Xapian::WritableDatabase get_writable_database(const std::string & dbname);
};

#endif // XAPIAN_INCLUDED_BACKENDMANAGER_QUARTZ_H
