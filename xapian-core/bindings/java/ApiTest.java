/* ApiTest.java: class containing java implementations of apitest tests.
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

public class ApiTest {
    public static void main(String[] args) throws Throwable {
        backendmanager = new BackendManager();

	backendmanager.set_datadir(System.getProperty("srcdir", ".") +
	                           "/../../tests/testdata/" );

        System.out.println("pctcutoff1: " + test_pctcutoff1());
    }

    public static boolean test_pctcutoff1() throws Throwable
    {
	boolean success = true;

	OmEnquire enquire = new OmEnquire(get_simple_database());
	init_simple_enquire(enquire);

	OmMSet mymset1 = enquire.get_mset(0, 100);

	if (verbose) {
	    System.out.println("Original mset pcts:");
	    print_mset_percentages(mymset1);
	    System.out.println("");
	}

	int num_items = 0;
	int my_pct = 100;
	int changes = 0;
	OmVector mymset1_items = mymset1.get_items();
	for (int i=0; i<mymset1_items.size(); ++i) {
	    int new_pct = mymset1.convert_to_percent((OmMSetItem)mymset1_items.elementAt(i));
	    if (new_pct != my_pct) {
		changes++;
		if (changes <= 3) {
		    num_items = i;
		    my_pct = new_pct;
		}
	    }
	}

	if (changes <= 3) {
	    success = false;
	    if (verbose) {
		System.out.println("MSet not varied enough to test");
	    }
	}
	if (verbose) {
	    System.out.println("Cutoff percent: " + my_pct);
	}

	OmMatchOptions mymopt = new OmMatchOptions();
	mymopt.set_percentage_cutoff(my_pct);
	OmMSet mymset2 = enquire.get_mset(0, 100, null, mymopt);
	OmVector mymset2_items = mymset2.get_items();

	if (verbose) {
	    System.out.println("Percentages after cutoff:");
	    print_mset_percentages(mymset2);
	    System.out.println("");
	}

	if (mymset2_items.size() < num_items) {
	    success = false;
	    if (verbose) {
		System.out.println("Match with % cutoff lost too many items");
	    }
	} else if (mymset2_items.size() > num_items) {
	    for (int i=num_items; i<mymset2_items.size(); ++i) {
		if (mymset2.convert_to_percent((OmMSetItem)mymset2_items.elementAt(i)) != my_pct) {
		    success = false;
		    if (verbose) {
			System.out.println("Match with % cutoff returned "
				+ " too many items");
		    }
		    break;
		}
	    }
	}

	return success;
    }

    public static OmDatabaseGroup get_simple_database() throws Throwable {
        OmDatabase mydb = get_database("apitest_simpledata");
	return make_dbgrp(mydb);
    }

    public static void init_simple_enquire(OmEnquire enq) throws Throwable {
        enq.set_query(new OmQuery("this"));
    }

    public static void print_mset_percentages(OmMSet mset)
    {
        OmVector mset_items = mset.get_items();
        for (int i=0; i<mset_items.size(); ++i) {
	    System.out.print(" ");
	    System.out.print(mset.convert_to_percent(
	                     (OmMSetItem)mset_items.elementAt(i)));
	}
    }

    public static boolean verbose = true;

    private static BackendManager backendmanager;

    private static OmDatabase get_database(String dbname) throws Throwable {
        return backendmanager.get_database(dbname);
    }

    private static OmDatabaseGroup make_dbgrp(OmDatabase db1) throws Throwable {
        OmDatabaseGroup result = new OmDatabaseGroup();

	result.add_database(db1);

	return result;
    }
}
