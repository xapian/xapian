// Simple command-line search program
//
// Copyright (c) 2003 James Aylett
// Copyright (c) 2004,2006 Olly Betts
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
	if (argv.Length < 2) {
	    Console.Error.WriteLine("Usage: SimpleSearch <path to database> <search terms>");
	    Environment.Exit(1);
	}

	try {
	    Xapian.Database database = new Xapian.Database(argv[0]);
	    Xapian.Enquire enquire = new Xapian.Enquire(database);
	    Xapian.Stem stemmer = new Xapian.Stem("english");

	    Xapian.Query q = null, term;
	    for (int i = 1; i < argv.Length; ++i) {
		term = new Xapian.Query(stemmer.stem_word(argv[i].ToLower()));
		if (q == null) {
		    q = term;
		} else {
		    q = new Xapian.Query(Xapian.Query.op.OP_OR, q, term);
		}
	    }
		
	    Console.WriteLine("Performing query `" + q.get_description()+ "'");

	    enquire.set_query(q);
	    Xapian.MSet matches = enquire.get_mset(0, 10);

	    Console.WriteLine("{0} results found", matches.get_matches_estimated());

	    Xapian.MSetIterator m = matches.begin();
	    while (m != matches.end()) {
		Console.WriteLine("ID {0} {1}% [{2}]", m.get_docid(), m.get_percent(), m.get_document().get_data());
		++m;
	    }
	} catch (Exception e) {
	    Console.Error.WriteLine("Exception: " + e.ToString());
	    Environment.Exit(1);
	}
    }
}
