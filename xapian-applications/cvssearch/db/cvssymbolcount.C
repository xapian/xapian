// cvssymbolcount.C
//
// (c) 2001 Amir Michail (amir@users.sourceforge.net)

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

//
// Usage:  cvssymbolcount package.count symbol_name
//                       => returns # lines in package with symbol_name
//
// Examples:
//
// cvssymbolcount kword.count QPopupMenu
// cvssymbolcount kword.count addItem\(\)
//

#include <db_cxx.h>
#include <string>

int main(int argc, char *argv[]) {
     
  if(argc < 3) {
    cout << "Usage: " << argv[0] <<
      " <path to database> <symbol>" << endl;
    exit(1);
  }
  
  string package = argv[1];
  string symbol = argv[2];

  assert( package.find(".count") != -1 );

  // read entry
  try {
    Db db(0,0);
    db.open((package).c_str(), 0, DB_HASH, DB_RDONLY, 0 );
    
    // read entry with key hello
    Dbt key((void*)symbol.c_str(), symbol.length()+1); // include 0 at end
    Dbt data;
    string s;
    int rc = db.get( 0, &key, &data, 0);
    if ( rc == DB_NOTFOUND ) {
      s = "0";
    } else {
      s = (char*)data.get_data();
    }
    cout << s << endl;
    db.close(0);
  } catch ( DbException& e ) {
    cerr << "Exception:  " << e.what() << endl;
  }
 

}


