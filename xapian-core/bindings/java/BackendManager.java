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
    public OmDatabase get_database(String[] dbnames) throws Throwable {
        return do_getdb(dbnames);
    }

    public OmDatabase do_getdb(String[] dbnames) throws Throwable {
        return do_getdb_sleepycat(dbnames);
    }

    public OmDatabase do_getdb_sleepycat(String[] dbnames) throws Throwable {
        String parent_dir = ".sleepycat";
	create_dir_if_needed(parent_dir);

	String dbdir = parent_dir + "/db";
	for (int i=0; i<dbnames.length; i++) {
	    dbdir += "=" + dbnames[i];
	}
	if (files_exist(change_names_to_paths(dbnames))) {
	    boolean created = create_dir_if_needed(dbdir);

	    if (created) {
	        OmWritableDatabase db =
		    new OmWritableDatabase("sleepycat", make_strvec(dbdir));
		System.err.println("Indexing to " + dbdir);
	        index_files_to_database(db, change_names_to_paths(dbnames));
		db = null;
		System.runFinalization();
		System.gc();
		return new OmDatabase("sleepycat", make_strvec(dbdir));
	    }
	    return new OmDatabase("sleepycat", make_strvec(dbdir));
	} else {
	    return new OmWritableDatabase("sleepycat", make_strvec(dbdir));
	}
    }

    public OmDatabase do_getdb_inmemory(String[] dbnames) throws Throwable {
        OmWritableDatabase db = new OmWritableDatabase("inmemory",
	                                               make_strvec());
	index_files_to_database(db, change_names_to_paths(dbnames));

	return db;
    }

    public OmDatabase get_database(String dbname1) throws Throwable {
        String[] dbnames = new String[1];

        dbnames[0] = dbname1;

	return do_getdb(dbnames);
    }

    public OmDatabase get_database(String dbname1, String dbname2) throws Throwable {
        String[] dbnames = new String[2];

        dbnames[0] = dbname1;
	dbnames[1] = dbname2;

	return do_getdb(dbnames);
    }

    public static String[] make_strvec(String s) {
        String[] retval = new String[1];
	retval[0] = s;
	return retval;
    }

    public static String[] make_strvec() {
        return new String[0];
    }

    public static boolean create_dir_if_needed(String dirname) throws Throwable {
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
                                                String[] paths) throws Throwable
    {
        for (int i=0; i<paths.length; ++i) {
	    FileReader from = new FileReader(paths[i]);
	    LineNumberReader from_byline = new LineNumberReader(from);

	    while (from_byline.ready()) {
	        String para = get_paragraph(from_byline);
		database.add_document(string_to_document(para));
	    }
	}
    }

    private static String get_paragraph(LineNumberReader input) throws Throwable {
        String para = "";
	String line;
	int linecount = 0;
	do {
	    if (!input.ready()) {
	        break;
	    }
	    line = input.readLine();
	    if (line != null) {
		para += line + "\n";
		linecount++;
		if (linecount > 30) {
		    break;
		}
	    }
	} while(linecount < 3 || (line != null && !line.trim().equals("")));
        return para;
    }

    private static OmDocumentContents string_to_document(String paragraph) throws OmError {
        OmStem stemmer = new OmStem("english");

	//System.out.println("Adding paragraph: " + paragraph);

	OmDocumentContents document = new OmDocumentContents();

	document.set_data(paragraph);

	for (int i=1; i<10; ++i) {
	    if (i >= paragraph.length()) {
	        break;
	    } else {
	        document.add_key(i, paragraph.substring(i, i+1));
	    }
	}

	int position = 1;

	while (paragraph.trim().length() > 0) {
	    paragraph = paragraph.trim();

	    int spacepos = 0;
	    while (spacepos < paragraph.length() &&
	           !isspace(paragraph.charAt(spacepos))) {
		spacepos++;
	    }
	    String word = paragraph.substring(0, spacepos);
	    word = stripword(word);
	    word = word.toLowerCase();
	    word = stemmer.stem_word(word);
	    if (word.length() != 0) {
	        document.add_posting(word, position++);
	    }
	    paragraph = paragraph.substring(spacepos);
	}

	return document;
    }

    private static boolean isspace(char c) {
        return (c == ' ' || c == '\t' || c == '\n');
    }

    private static boolean isalpha(char c) {
        return (c >= 'a' && c <= 'z') ||
	       (c >= 'A' && c <= 'Z');
    }

    private static String stripword(String word) {
        String result = "";

	for (int i=0; i<word.length(); ++i) {
	    if (isalpha(word.charAt(i))) {
	        result += word.charAt(i);
	    }
	}
	return result;
    }
}
