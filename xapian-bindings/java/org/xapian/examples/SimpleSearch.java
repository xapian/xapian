/**
 Copyright (c) 2003, Technology Concepts & Design, Inc.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without modification, are permitted
 provided that the following conditions are met:

 - Redistributions of source code must retain the above copyright notice, this list of conditions
 and the following disclaimer.

 - Redistributions in binary form must reproduce the above copyright notice, this list of conditions
 and the following disclaimer in the documentation and/or other materials provided with the distribution.

 - Neither the name of Technology Concepts & Design, Inc. nor the names of its contributors may be used to
 endorse or promote products derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 THE POSSIBILITY OF SUCH DAMAGE.
 **/
package org.xapian.examples;

import org.xapian.*;

public class SimpleSearch {


    public static void main(String[] args) throws Exception {
        // ensure we have enough arguments
        if (args.length < 2) {
            usage();
            System.exit(0);
        }

        String dbpath = args[0];

        // turn the remaining command-line arguments into our query
	Query query = new Query(args[1]);
        for (int x = 2; x < args.length; x++) {
	    query = new Query(Query.OP_OR, query, new Query(args[x]));
        }

        // open the specified database
        Database db = new Database(dbpath);

        // and query the database
        Enquire enquire = new Enquire(db);
        enquire.setQuery(query);
        MSet matches = enquire.getMSet(0, 2500);    // get up to 2500 matching documents
        MSetIterator itr = matches.iterator();

        System.err.println("Found " + matches.size() + " matching documents using " + query);
        while (itr.hasNext()) {
            itr = (MSetIterator) itr.next(); // TODO:  Make this more like a Java Iterator
            // by returning some kind of "MatchDescriptor" object
            Document doc = itr.getDocument();
            System.err.println(itr.getPercent() + "% [" + itr.getDocumentId() + "] " + doc.getValue(0));
        }
    }

    private static void usage() {
        System.err.println("Usage:");
        System.err.println("\tjava org.xapian.examples.SimpleSearch </path/to/database> <query>");
    }

}
