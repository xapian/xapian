#include <strstream>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include "process.h"
#include "forward_range_map_algorithm.h"
#include "backward_line_map_algorithm.h"
#include "diff.h"
#include "cvs_log.h"

string scvs_root;

static string scvs_log = "cvs log -b ";

static string 
concat(uint argc, const char **argv, const char delim)
{
    ostrstream ost;
    for (uint i = 0; i < argc; ++i)
    {
        ost << argv[i] << delim;
    }
    ost << ends;
    return ost.str();
}

int 
main(unsigned int argc, const char **argv)
{
    unsigned int i = 1;
    string line_db = "/tmp/line.db";
    string file_db = "/tmp/file.db";
    bool use_line = true;
    char *cvsroot = getenv("CVSROOT");
    if (cvsroot)
    {
        scvs_root = cvsroot;
    }
    else
    {
        ifstream f("CVS/Root");
        if (f) {
            getline(f, scvs_root);
        }
    }

    while (i < argc)
    {
        if (0) {
        } else if (!strcmp(argv[i], "-r")) {
            use_line = false;
        } else if (!strcmp(argv[i], "-l")) {
            use_line = true;
        } else if (!strcmp(argv[i], "-f1")) {
            line_db = argv[++i];
        } else if (!strcmp(argv[i], "-f2")) {
            file_db = argv[++i];
        } else if (!strcmp(argv[i], "-d")) {
            scvs_root = argv[++i];
        } else {
            break;
        }
        ++i;
    }

    ofstream line_fout(line_db.c_str());
    ofstream file_fout(file_db.c_str());

    string args = concat(argc-i, &argv[i], ' ');
    string command = scvs_log + args;
    process p = process(command);

    istream *pis = p.process_output();
    unsigned int num_mappings = 0;
    unsigned int num_size = 0;

    unsigned int num_deletes = 0;
    unsigned int num_updates = 0;
    unsigned int num_searches = 0;

    unsigned int index = 0;

    unsigned int num_lines = 0;

    unsigned int num_version = 0;
    unsigned int max_version = 0;

    string max_version_file;
    unsigned int file_index = 1;
    if (pis)
    {
        while (1)
        {
            cvs_log log;
            if (*pis >> log)
            {
                map_algorithm * pcollection;
                if (use_line)
                {
                    pcollection = new backward_line_map_algorithm(log, ++index);
                } else 
                {
                    pcollection = new forward_range_map_algorithm(log, ++index);
                }
                num_mappings += pcollection->mappings();
                num_updates  += pcollection->updates();
                num_deletes  += pcollection->deletes();
                num_searches += pcollection->searches();

                num_lines    += pcollection->lines();
                num_version  += log.size();
                 
                if (max_version < log.size())
                {
                    max_version =  log.size();
                    max_version_file = log.path_name();
                }

                if (pcollection != 0)
                {
                    line_fout << *pcollection;
                    file_fout << log.path_name() << " " << file_index << "\002" << endl;
                    file_index += pcollection->lines();
                    delete pcollection;
                }
            } 
            else
            {
                break;
            }
        }
    }
    if (num_size != 0 && num_lines != 0)
    {
        cerr << ""
             << "total number of updates  " << num_updates << endl
             << "total number of deletes  " << num_deletes << endl
             << "total number of searches " << num_searches << endl

             << "total number of versions " << num_version << endl
             << "total number of files " << index << endl
             << "average number of versions in a file " << num_version / index << endl
             << "maximum number of versions in a file " << max_version << " " << max_version_file << endl
             << "total number of comments " << num_mappings << endl
             << "total number of lines " << num_lines << endl
             << "average number of comments per line " << (double) num_mappings / (double) num_lines << endl
             << "" << endl
            ;
    }
    exit (EXIT_SUCCESS);
}
