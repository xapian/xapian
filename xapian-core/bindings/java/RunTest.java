/* RunTest.java: test class for Open Muscat java bindings.
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

public class RunTest {
    public static int main (String[] args) throws Throwable {
        boolean success = true;
    	// First some quick playing with stemmers and queries.
    	OmStem foo = new OmStem("english");

	System.out.println(foo.stem_word("giraffe"));

	OmQuery bar = new OmQuery("word", 1, 2);
	System.out.println(bar.get_description());

	String[] baz_ = { "giraff", "lion" };
	OmQuery baz = new OmQuery("FILTER", baz_);
	System.out.println(baz.get_description());

	baz = new OmQuery("NEAR", baz_, 3);
	System.out.println(baz.get_description());

	OmQuery myquery = new OmQuery("OR",
				      new OmQuery("AND",
				                  new OmQuery("one", 1, 1),
						  new OmQuery("three", 1, 3)),
				      new OmQuery("OR",
				                  new OmQuery("four", 1, 4),
						  new OmQuery("two", 1, 2)));

	String[] terms = myquery.get_terms();
	String[] correct_terms = { "one", "two", "three", "four" };
	System.out.print("terms = ");
	for (int i=0; i<terms.length; ++i) {
	    System.out.print(terms[i] + " ");
	    if (i < correct_terms.length) {
	        if (terms[i] != correct_terms[i]) {
	            System.err.println("Incorrect term.");
		    success = false;
		}
	    } else {
	        System.err.println("Incorrect number of terms.");
		success = false;
	    }
	}
	System.out.println("");

	OmDatabaseGroup dbgrp = new OmDatabaseGroup();

//	dbgrp.add_database("inmemory", {});

	return (success? 0 : -1);
    }
}
