// cvscomments.C
//
// (c) 2001 Amir Michail (amir@users.sourceforge.net)

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

//
// Usage:  cvscomments package file:line
//                       => returns combined comments for that line
// Example:
//
// cvscomments kdebase_konqueror kdebase/konqueror/konq_actions.cc:25  
//
//  yields 
//
// Drag the "Location" label to start a drag with the current URL. 
// Kinda cool to open the current page into another window 
// (now that Simon implemented handling of drops), 
// or to drop the current URL directly at the right position in the bookmark editor :-

#include <db_cxx.h>
#include <string>

int main(int argc, char *argv[]) {
     
  if(argc < 3) {
    cout << "Usage: " << argv[0] <<
      " <path to database> <file:line>" << endl;
    exit(1);
  }
  
  string package = argv[1];
  string file_and_line = argv[2];

  if ( package[ package.length()-1] == '/' ) {
    // get rid of trailing /
    package = string( package, 0, package.length()-1 );
  }

  // read entry
  try {
    Db db(0,0);
    db.open((package+"/comments.db").c_str(), 0, DB_HASH, DB_RDONLY, 0 );
    
    // read entry with key hello
    Dbt key((void*)file_and_line.c_str(), file_and_line.length()+1); // include 0 at end
    Dbt data;
    string s;
    int rc = db.get( 0, &key, &data, 0);
    if ( rc == DB_NOTFOUND ) {
      s = "ERROR:  Could not find comments for " + file_and_line + ".";
    } else {
      s = (char*)data.get_data();
    }
    cout << s << endl;
    db.close(0);
  } catch ( DbException& e ) {
    cerr << "Exception:  " << e.what() << endl;
  }
 

}


