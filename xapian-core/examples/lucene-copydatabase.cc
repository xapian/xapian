
#include <config.h>
#include <xapian.h>

#include "safesysstat.h"

#include <dirent.h>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <cstring>
#include <errno.h>
#include <sys/types.h>

using namespace std;

static void
show_usage(int rc)
{
    cout << "Usage: lucene-copydatabase SOURCE_DATABASE ... DESTINATION_DATABASE\n"
        << endl;
    exit(rc);
}

static void
copy_files(const char * src, const char * dest)
{
    DIR * dp = NULL;
    struct dirent * dirp;
    if (NULL == (dp = opendir(src))) {
        cout << "open dir failed, dir=" << src << endl;
        exit(1);
    }

    const char * end = dest;
    const char * begin = dest;

    //make destination directory if it is not exists, just available for Linux
    while (end && '\0' != *begin) {
        end = strchr(begin, '/');
        string dir = "";
        if (NULL == end) {
            dir = string(dest);
        } else {
            dir = string(dest, end - dest);
            begin = end + 1;
        }

        if ("/" == dir || dir.empty())
            continue;

        //directory exists, no need to create it
        if (NULL != opendir(dir.c_str()))
            continue;

        if (ENOENT != errno)
            continue;
        
        if (-1 == mkdir(dir.c_str(), S_IRWXU | S_IRWXG))
            cout << "mkdir failed, dir=" << dir << endl;
    }

    while (NULL != (dirp = readdir(dp))) {
        if (0 == strcmp(dirp->d_name, ".") || 0 == strcmp(dirp->d_name, "..")) {
            continue;
        }

        string src_file = string(src) + string(dirp->d_name);
        ifstream input;
        input.open(src_file.c_str(), ios::in | ios::binary);
        if (!input) {
            cout << "open input failed" << endl;
        }

        string dest_file = string(dest) + string("/")+ string(dirp->d_name);
        cout << dest_file.c_str() << endl;
        ofstream output;
        output.open(dest_file.c_str(), ios::out | ios::binary);
        if (!output) {
            cout << "open output failed" << endl;
        }

        output << input.rdbuf();
        input.close();
        output.close();
    }
}

int main(int argc, char **argv)
{
    if (argc < 3) show_usage(1);

    const char * dest = argv[argc - 1];
    //Just one source database is supported
    const char * src = argv[argc - 2];

    //Copy all files from source directory to destination directory
    copy_files(src, dest);

    Xapian::Database db_in(src);
    //Xapian::ValueIterator::Internal, when ValueIterator destruction is executed,
    //delete internal is called too. So delete internal is not needed
    Xapian::TermIterator it = db_in.allterms_begin();
    int i = 0;
    Xapian::PostingIterator it_pl;

    unsigned int max_wdf = 0;
    unsigned int max_docid = 0;
    string max_termname = "";
    while (it != db_in.allterms_end()) {
        it_pl = it.postlist_begin();
        //cout << "internal_refs=" << it_pl.get_internal_refs() << endl;
        while (it_pl != it.postlist_end()) {
            //cout << "copydata term=" << it.get_termname() << "copydata wdf=" <<
                //it_pl.get_wdf() << ", did=" << *it_pl << endl;
            if (it_pl.get_wdf() > max_wdf) {
                max_wdf = it_pl.get_wdf();
                max_docid = *it_pl;
                max_termname = it.get_termname();
            }

            ++it_pl;
        }
        ++it;
        ++i;
        //cout << "i=" << i << endl;
    }

    cout << "copydata max_wdf=" << max_wdf << ", max_docid=" << max_docid <<
        ", max_termname=" << max_termname << endl;
    std::ofstream opf;
    string stat_file = string(dest) + "/" + "stat.xapian";
    opf.open(stat_file.c_str(), ios::trunc);
    if (!opf) {
        cout << "open statistics file error, file=" << stat_file << endl;
        exit(1);
    }
    opf << "wdf_upper_bound=" << max_wdf << endl;
    opf.close();
}
