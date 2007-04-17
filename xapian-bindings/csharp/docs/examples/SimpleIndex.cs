// Index each paragraph of a textfile as a document
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
using System.Text.RegularExpressions;

class SimpleIndex {
    const int MAX_PROB_TERM_LENGTH = 64;

    public static void Main(string[] argv) {
	if (argv.Length != 1) {
	    Console.Error.WriteLine("Usage: SimpleIndex <path to database>");
	    Environment.Exit(1);
	}

	try {
	    Xapian.WritableDatabase database;
	    database = new Xapian.WritableDatabase(argv[0], Xapian.Xapian.DB_CREATE_OR_OPEN);

	    // Use a regex to tokenise words.
	    Regex word = new Regex("([A-Za-z0-9]+(?:[-+]+(?![-+A-Za-z0-9]))?)");

	    Xapian.Stem stemmer = new Xapian.Stem("english");

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
			Xapian.Document doc = new Xapian.Document();
			doc.SetData(para);
			uint pos = 0;
			int i = 0;
			while (true) {
			    Match match = word.Match(para, i);
			    if (!match.Success) break;
			    Group g = match.Groups[1];
			    if (g.Length <= MAX_PROB_TERM_LENGTH) {
				string term = g.Value.ToLower();
				term = stemmer.Apply(term);
				doc.AddPosting(term, pos);
				++pos;
			    }
			    i = match.Index + match.Length;
			}
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
