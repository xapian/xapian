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
#include <vector>
using std::vector;

string scvs_root;

static string scvs_log = "cvs log -b ";
static string smax_version_file;
static unsigned int num_mappings = 0;
static unsigned int num_size = 0;
static unsigned int num_deletes = 0;
static unsigned int num_updates = 0;
static unsigned int num_searches = 0;
static unsigned int num_lines = 0;
static unsigned int num_version = 0;
static unsigned int max_version = 0;
static unsigned int file_index = 0;
static unsigned int file_offset = 1;

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

void
cvsmap(istream & log_in, ostream & line_out, ostream & file_out, bool use_line)
{
    cvs_log log;
    if (log_in >> log)
    {
        map_algorithm * pcollection;
        if (use_line)
        {
            pcollection = new backward_line_map_algorithm(log, ++file_index);
        } else 
        {
            pcollection = new forward_range_map_algorithm(log, ++file_index);
        }
        if (pcollection != 0)
        {
            num_mappings += pcollection->mappings();
            num_updates  += pcollection->updates();
            num_deletes  += pcollection->deletes();
            num_searches += pcollection->searches();
            
            num_lines    += pcollection->lines();
            num_version  += log.size();
            
            if (max_version < log.size())
            {
                max_version =  log.size();
                smax_version_file = log.path_name();
            }
            
            line_out << *pcollection;
            file_out << log.path_name() << " " << file_offset << "\002" << endl;
            file_offset += pcollection->lines();
            delete pcollection;
        }
    } 
}

int 
main(unsigned int argc, const char **argv)
{
    unsigned int i = 1;
    string line_db = "/tmp/line.db";
    string file_db = "/tmp/file.db";
    string input_file = "";
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
        } else if (!strcmp(argv[i], "-i")) {
            input_file = argv[++i];
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


    if (input_file == "")
    {
        while (i < argc)
        {
            string command = scvs_log + argv[i++];
            process p = process(command);
            istream * pis = p.process_output();
            if (pis)
            {
                cvsmap(*pis, line_fout, file_fout, use_line);
            }
        }
    } 
    else
    {
        ifstream input(input_file.c_str());
        string line;
        while (getline(input, line))
        {
            string command = scvs_log + line;
            process p = process(command);
            istream * pis = p.process_output();
            if (pis)
            {
                cvsmap(*pis, line_fout, file_fout, use_line);
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
             << "total number of files " << file_index << endl
             << "average number of versions in a file " << num_version / file_index << endl
             << "maximum number of versions in a file " << max_version << " " << smax_version_file << endl
             << "total number of comments " << num_mappings << endl
             << "total number of lines " << num_lines << endl
             << "average number of comments per line " << (double) num_mappings / (double) num_lines << endl
             << "" << endl
            ;
    }
    exit (EXIT_SUCCESS);
}
