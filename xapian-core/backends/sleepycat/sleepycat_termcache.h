/* sleepy_termcache.h: C++ class definition for sleepycat access routines
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

#ifndef OM_HGUARD_SLEEPY_TERMCACHE_H
#define OM_HGUARD_SLEEPY_TERMCACHE_H

class SleepyDatabaseInternals;
class SleepyDatabase;

/** Termname to termID mappings.
 *
 *  This class maintains a cache of termname to termID mappings, looking
 *  up in the database only when required.
 *
 *  Entries are not deleted from the cache until the object is destroyed.
 */
class SleepyDatabaseTermCache {
    friend class SleepyDatabase;
    private:
	SleepyDatabaseInternals * internals;
	SleepyDatabaseTermCache(SleepyDatabaseInternals *i) : internals(i) {}
    public:
	om_termname term_id_to_name(om_termid tid) const;
	om_termid term_name_to_id(const om_termname & tname) const;
};

#endif /* OM_HGUARD_SLEEPY_TERMCACHE_H */
