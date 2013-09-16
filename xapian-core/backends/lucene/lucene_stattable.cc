
#include <config.h>

#include "lucene_stattable.h"

#include "debuglog.h"

#include "safeerrno.h"

#include <sstream>

using namespace std;

LuceneStatTable::LuceneStatTable(const string & db_dir_)
        : wdf_upper_bound(0),
        db_dir(db_dir_)
{
    LOGCALL_CTOR(DB, "LuceneStatTable", db_dir);
    string file_path = db_dir + "/" + "stat.xapian";

    fin.open(file_path.c_str());
    //When doing lucene-copydatabase, file stat.xapian is not exist, should not
    //throw error exception.
    //If not doing lucene-copydatabase, throws exception when get_wdf_upper_bound(),
    //just return here
    if (!fin) {
        return ;
    }

    string line = string();
    while (getline(fin, line)) {
        std::size_t found = line.find_first_of("=");
        if (found == std::string::npos) {
            throw Xapian::DatabaseCreateError("LuceneStatTable error format");
        }

        string key = line.substr(0, found - 0);
        string value = line.substr(found + 1);

        //Only one line, so just check it
        if (key != "wdf_upper_bound") {
            throw Xapian::DatabaseCreateError("LuceneStatTable error format");
        }

        std::stringstream ss;
        ss << value;
        ss >> wdf_upper_bound;
    }
}

Xapian::termcount
LuceneStatTable::get_wdf_upper_bound() const
{
    LOGCALL(DB, Xapian::termcount, "LuceneStatTable::get_wdf_upper_bound", NO_ARGS);

    if (0 == wdf_upper_bound) {
        throw Xapian::DatabaseCreateError("LuceneStatTable wdf_upper_bound is error", 0);
    }

    RETURN(wdf_upper_bound);
}

void
LuceneStatTable::debug_get_table() const
{
    LOGCALL(DB, void, "LuceneStatTable::debug_get_table", NO_ARGS);

    LOGLINE(DB, "wdf_upper_bound=" << wdf_upper_bound);
}
