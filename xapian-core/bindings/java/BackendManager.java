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
    
    public OmDatabase get_database(String[] dbnames) throws OmError {
        return do_getdb(dbnames);
    }

    public OmDatabase do_getdb(String[] dbnames) throws OmError {
        return do_getdb_sleepy(dbnames);
    }

    public OmDatabase do_getdb_sleepy(String[] dbnames) throws OmError {
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
	        return new OmDatabase("sleepycat", make_strvec(dbdir));
	    }
	} else {
	    return new OmDatabase("sleepycat", make_strvec(dbdir));
	}
    }

    public OmDatabase get_database(String dbname1) throws OmError {
        String[] dbnames = new String[1];

        dbnames[0] = dbname1;

	return do_getdb(dbnames);
    }

    public static String[] make_strvec(String s) {
        String[] retval = new String[1];
	retval[0] = s;
	return retval;
    }

    public static boolean create_dir_if_needed(String dirname) throws OmError {
        File dir = new File(dirname);

	if (!dir.exists()) {
	    if (!dir.mkdir()) {
	        throw new OmOpeningError("Can't create directory");
	    }
	    return true;
	} else {
	    if (!dir.isDirectory()) {
	        throw new OmOpeningError("Not a directory");
	    }
	    return false;
	}
    }

    private String[] change_names_to_paths(String[] dbnames) {
        String[] paths = new String[dbnames.length];

	for (int i=0; i<dbnames.length; ++i) {
	    if (dbnames[i].length() > 0) {
	        if (datadir.length() == 0) {
		    paths[i] = dbnames[i];
		} else {
		    paths[i] = datadir + "/" + dbnames[i] + ".txt";
		}
	    } else {
	        paths[i] = "";
	    }
	}
	return paths;
    }

    private static boolean files_exist(String[] fnames) {
        for (int i=0; i<fnames.length; i++) {
	    File f = new File(fnames[i]);
	    if (!f.exists()) {
	        return false;
	    }
	}
	return true;
    }

    private String datadir;

    public void set_datadir(String datadir_) {
        datadir = datadir_;
    }

    private static void index_files_to_database(OmWritableDatabase database,
                                                String[] paths)
    {
        for (int i=0; i<paths.length; ++i) {
	    TextfileIndexerSource source = new TextfileIndexerSource(paths[i]);

	}
    }
}
