// Index each paragraph of a text file as a Xapian document.
//
// Copyright (c) 2003 James Aylett
// Copyright (c) 2004,2006,2007 Olly Betts
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
// USA

using System;

class SimpleIndex {
    public static void Main(string[] argv) {
	if (argv.Length != 1) {
	    Console.Error.WriteLine("Usage: SimpleIndex PATH_TO_DATABASE");
	    Environment.Exit(1);
	}

	try {
	    // Open the database for update, creating a new database if
	    // necessary.
	    Xapian.WritableDatabase database;
	    database = new Xapian.WritableDatabase(argv[0], Xapian.Xapian.DB_CREATE_OR_OPEN);

	    Xapian.TermGenerator indexer = new Xapian.TermGenerator();
	    Xapian.Stem stemmer = new Xapian.Stem("english");
	    indexer.SetStemmer(stemmer);

	    string para = "";
	    while (true) {
		string line = Console.In.ReadLine();
		if (line == null) {
		    if (para == "") break;
		    line = "";
		}
		line = line.Trim();
		if (line == "") {
		    if (para != "") {
			// We've reached the end of a paragraph, so index it.
			Xapian.Document doc = new Xapian.Document();
			doc.SetData(para);

			indexer.SetDocument(doc);
			indexer.IndexText(para);

			// Add the document to the database.
			database.AddDocument(doc);
			para = "";
		    }
		} else {
		    if (para != "") para += " ";
		    para += line;
		}
	    }
	} catch (Exception e) {
	    Console.Error.WriteLine("Exception: " + e.ToString());
	    Environment.Exit(1);
	}
    }
}
