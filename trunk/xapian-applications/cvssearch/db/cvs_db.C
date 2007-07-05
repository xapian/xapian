/************************************************************
 *
 *  cvs_db.C implementation.
 *
 *  (c) 2001 Andrew Yao (andrewy@users.sourceforge.net)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id$
 *
 ************************************************************/

#include "cvs_db.h"

using namespace std;

int
cvs_db::open(const string & filename, bool read_only) 
{
    if (_opened) return 0;
    string filename1 = filename + _db_index;
    int val = do_open(filename1, read_only);
    if (val == 0) {
	_opened = true;
    }
    return val;
}

int
cvs_db::close(int flags) {
    try {
        _opened = false;
        return _db.close(flags);
    }  catch (DbException& e ) {
        cerr << "SleepyCat Exception: " << e.what() << endl;
    }
    return 0;
}

int
cvs_db::remove(const string & filename, int flags) 
{
    try {
	close();
	return _db.remove(filename.c_str(), _db_name.c_str(), flags);
    }  catch (DbException& e ) {
	cerr << "SleepyCat Exception: " << e.what() << endl;
    }
    return 0;
}

int
cvs_db::sync()
{
    if (!_opened) return 0;
    try {
	return _db.sync(0);
    } catch (DbException &e) {
	cerr << "SleepyCat Exception: " << e.what() << endl;
	abort();
    }
    return 0;
}
