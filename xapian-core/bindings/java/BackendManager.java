/* BackendManager.java: class for managing test databases
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
import com.muscat.om.*;
import java.util.*;
import java.io.*;

public class BackendManager {
    
    public OmDatabase get_database(String[] dbnames) {
        return do_getdb(dbnames);
    }

    public OmDatabase do_getdb(String[] dbnames) {
        return do_getdb_sleepy(dbnames);
    }

    public OmDatabase do_getdb_sleepy(String[] dbnames) {
        String parent_dir = ".sleepy";
	create_dir_if_needed(parent_dir);

	String dbdir = parent_dir + "/db";
	for (int i=0; i<dbnames.length; i++) {
	    dbdir += "=" + dbnames[i];
	}
	if (files_exist(change_names_to_paths(dbnames))) {
	    boolean created = create_dir_if_needed(dbdir);

	    if (created) {
	        OmDatabase db =
		    new OmWritableDatabase("sleepycat", make_strvec(dbdir));
	        index_files_to_database(db, change_names_to_paths(dbnames));
		return db;
	    } else {
	        return OmDatabase("sleepycat", make_strvec(dbdir));
	    }
	} else {
	    return OmDatabase("sleepycat", make_strvec(dbdir));
	}
    }
}
