/**
 Copyright (c) 2003, Technology Concepts & Design, Inc.
 Copyright (c) 2011,2017 Olly Betts
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

import org.xapian.Document;
import org.xapian.WritableDatabase;
import org.xapian.Xapian;

public class SimpleIndex {


    public static void main(String[] args) throws Exception {
        // ensure we have enough args
        if (args.length < 2) {
            usage();
            System.exit(0);
        }

        // create or *overwrite an existing* Xapian database
        String dbpath = args[0];
        WritableDatabase db = new WritableDatabase(dbpath, Xapian.DB_CREATE_OR_OVERWRITE);

        // walk through remaining command-line arguments and
        // add each argument as a single term to a new document.
        for (int x = 1; x < args.length; x++) {

            String term = args[x];
            Document doc = new Document();
            doc.addTerm(term);
            doc.setData(term);
            db.addDocument(doc);
        }

        // make sure to commit the database so the documents get written to disk
        db.commit();
    }


    private static void usage() {
        System.err.println("Usage:");
        System.err.println("\tjava org.xapian.examples.SimpleIndex </path/to/database> <term 1> ... <term N>");
    }

}
