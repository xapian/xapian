/** @file unixperm.cc
 * @brief Filter results according to Unix-style access permissions
 */
/* Copyright (C) 2010 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#include <config.h>

#include "unixperm.h"

#include "safeerrno.h"
#include <sys/types.h>
#include <grp.h>
#include <pwd.h>

#include <cstring>

using namespace std;

void
apply_unix_permissions(Xapian::Query & query, const char * user)
{
    // Apply permission filters.
    vector<string> allow;
    vector<string> deny;
    allow.push_back("I*");
    allow.push_back(string("I@") + user);
    deny.push_back(string("V@") + user);

    struct passwd * pwent = getpwnam(user);
    if (!pwent) {
	query = Xapian::Query();
	return;
    }

    // Add supplementary groups for user too.
#ifdef HAVE_GETGROUPLIST
    int ngroups = 32;

    gid_t *groups = new gid_t[ngroups];

    while (getgrouplist(user, pwent->pw_gid, groups, &ngroups) == -1) {
	delete [] groups;
	groups = new gid_t[ngroups];
    }

    for (int i = 0; i < ngroups; i++) {
	struct group * grentry = getgrgid(groups[i]);
	if (grentry) {
	    const char * group = grentry->gr_name;
	    allow.push_back(string("I#") + group);
	    deny.push_back(string("V#") + group);
	}
    }
#else
    gid_t main_gid = pwent->pw_gid;
    struct group * grentry = getgrgid(pwent->pw_gid);
    if (grentry) {
	const char * group = grentry->gr_name;
	allow.push_back(string("I#") + group);
	deny.push_back(string("V#") + group);
    }

    // Make sure we're rewound.
    setgrent();
    errno = 0;
    while ((grentry = getgrent()) != NULL) {
	// Don't process the main group again if it happens to be listed as
	// a supplementary group.
	if (grentry->gr_gid == main_gid)
	    continue;
	for (char ** members = grentry->gr_mem; *members; ++members) {
	    if (strcmp(*members, user) == 0) {
		const char * group = grentry->gr_name;
		allow.push_back(string("I#") + group);
		deny.push_back(string("V#") + group);
		break;
	    }
	}
    }
    if (errno) {
	// FIXME: handle?
    }
    endgrent();
#endif

    query = Xapian::Query(query.OP_FILTER,
			  query,
			  Xapian::Query(query.OP_OR,
					allow.begin(),
					allow.end()));
    query = Xapian::Query(query.OP_AND_NOT,
			  query,
			  Xapian::Query(query.OP_OR,
					deny.begin(),
					deny.end()));
}
