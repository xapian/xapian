/* omdatabase.h
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
 */

#ifndef OM_HGUARD_OMDATABASE_H
#define OM_HGUARD_OMDATABASE_H

#include <vector>

///////////////////////////////////////////////////////////////////
// OmDatabaseGroup class
// =====================

/** This class encapsulates a set of databases.
 *
 *  This class is used in conjunction with an OmEnquire object.
 *
 *  @exception OmInvalidArgumentError will be thrown if an invalid
 *  argument is supplied, for example, an unknown database type.
 *
 *  @exception OmOpeningError may be thrown if the database cannot
 *  be opened (for example, a required file cannot be found).
 */
class OmDatabaseGroup {
    private:
	class Internal;
	Internal *internal;

	/** InternalInterface is a class used externally to interact
	 *  with OmDatabaseGroup objects.
	 */
	class InternalInterface;
	friend class InternalInterface;

    public:
	OmDatabaseGroup();
	~OmDatabaseGroup();
	OmDatabaseGroup(const OmDatabaseGroup &other);
	void operator=(const OmDatabaseGroup &other);

	/** Add a new database to use.
	 *
	 *  The database may not be opened by this call: the system may wait
	 *  until a get_mset (or similar call).
	 *  Thus, failure to open the database may not result in an
	 *  OmOpeningError exception being thrown until the database is used.
	 *
	 *  The database will always be opened read-only.
	 *
	 *  @param type    a string describing the database type.
	 *  @param params  a vector of parameters to be used to open the
	 *  database: meaning and number required depends on database type.
	 *
	 *  @exception OmInvalidArgumentError  See class documentation.
	 *  @exception OmOpeningError          See class documentation.
	 */
	void add_database(const string & type,
			  const vector<string> & params);
};

#endif /* OM_HGUARD_OMDATABASE_H */
