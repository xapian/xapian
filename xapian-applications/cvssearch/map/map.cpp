/************************************************************
 *
 *  map.cpp the main entry point to the cvs line mapper,
 *  it creates a profile of {line, cvs_comments};
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
 *  Usage:
 *
 *  $>cvsmap -h
 *
 *  $Id$
 *
 ************************************************************/

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

string scvs_log    = "cvs -f log -b ";
string scvs_diff   = "cvs -f diff -b ";
string scvs_update = "cvs -f update -p ";

static string smax_version_file;
static string sline_db    = "/tmp/line.db";
static string soffset_db  = "/tmp/line.offset";
static string sfile_db    = "/tmp/file.db";
static string sstats_file = "";

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

static bool use_db = false;
static bool use_line = true;

static void usage(const string & prog_name);
static void cvsmap(istream & log_in, ostream & line_out, ostream & file_out, cvs_db_file *);

int 
main(unsigned int argc, const char **argv)
{
    unsigned int i = 1;
    string input_file = "";

    // ----------------------------------------
    // determine what the cvsroot is
    // ----------------------------------------
    char *cvsroot = getenv("CVSROOT");
    if (cvsroot) {
        scvs_root = cvsroot;
    } else {
        ifstream f("CVS/Root");
        if (f) {
            getline(f, scvs_root);
        }
    }

    while (i < argc)
    {
        if (0) {
        } else if (!strcmp(argv[i], "-i") && i+1 < argc) {
            input_file = argv[++i];
        } else if (!strcmp(argv[i], "-r")) {
            use_line = false;
        } else if (!strcmp(argv[i], "-l")) {
            use_line = true;
        } else if (!strcmp(argv[i], "-f1") && i+1 < argc) {
            sline_db = argv[++i];
        } else if (!strcmp(argv[i], "-f2") && i+1 < argc) {
            soffset_db = argv[++i];
        } else if (!strcmp(argv[i], "-h")) {
            usage(argv[0]);
        } else if (!strcmp(argv[i], "-d") && i+1 < argc) {
            scvs_root = argv[++i];
        } else if (!strcmp(argv[i], "-db") && i+1 < argc) {
            sfile_db = argv[++i];
            use_db = true;
        } else if (!strcmp(argv[i], "-st") && i+1 < argc) {
            sstats_file = argv[++i];
        } else {
            break;
        }
        ++i;
    }

    ofstream line_fout  (sline_db.c_str());
    ofstream offset_fout(soffset_db.c_str());
    ostream * pstats_fout = 0;
    if (strcmp(sstats_file.c_str(), "")) {
        pstats_fout = new ofstream(sstats_file.c_str());
    } else {
        pstats_fout = &cerr;
    }

    cvs_db_file * pdb_file = 0;
    if (use_db)
    {
        unlink(sfile_db.c_str());
        pdb_file = new cvs_db_file(sfile_db);
    }

    if (input_file == "")
    {
        while (i < argc)
        {
            string command = scvs_log + argv[i++];
            process p = process(command);
            cout << "RUNNING " << command << endl;
            istream * pis = p.process_output();
            if (pis)
            {
                cvsmap(*pis, line_fout, offset_fout, pdb_file);
                if (pdb_file)
                {
                   pdb_file->sync();
                }
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
            cout << "RUNNING " << command << endl;
            istream * pis = p.process_output();
            if (pis)
            {
                cvsmap(*pis, line_fout, offset_fout, pdb_file);
                if (pdb_file)
                {
                   pdb_file->sync();
                }
            }

        }
    }

    if (num_size != 0 && num_lines != 0)
    {
        *pstats_fout << "************************************************************"
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
                     << "************************************************************" << endl;
        if (strcmp(sstats_file.c_str(), "")) {
            delete pstats_fout;
        }
    }
    if (pdb_file)
    {
        delete pdb_file;
    }
    exit (EXIT_SUCCESS);
}

static void usage(const string & prog_name)
{
    cerr << "Usage: " << prog_name << " [OPTION] [FILE] ..." << endl
         << endl
         << "Options:" << endl
         << "  -h                 print this message" << endl
         << "  -i                 input file contain a list of files to map" << endl
         << "  -r                 use the forward range method" << endl
         << "  -l                 use the backward line method" << endl
         << "  -d                 specify the cvsroot" << endl
         << "  -db file.db        specify the database file" << endl
         << "  -f1 line.db        specify the line-comment mapped file" << endl
         << "  -f2 line.offset    the file-name index file" <<endl
         << "  -st stat.file      output statistical information to stat.file" << endl
        ;
    
    exit(0);
}

static void
cvsmap(istream & log_in, ostream & line_out, ostream & offset_out, cvs_db_file * pdb_file)
{
    cvs_log log;
    if (log_in >> log)
    {
        map_algorithm * pcollection;
        if (use_line) {
	    cout << "BEGIN NEW COLLECTION" << endl;
            pcollection = new backward_line_map_algorithm(log, ++file_index, pdb_file);
	    cout << "END   NEW COLLECTION" << endl;
        } else {
            pcollection = new forward_range_map_algorithm(log, ++file_index, pdb_file);
        }
        if (pcollection != 0) {
            num_mappings += pcollection->mappings();
            num_updates  += pcollection->updates();
            num_deletes  += pcollection->deletes();
            num_searches += pcollection->searches();
            
            num_lines    += pcollection->lines();
            num_version  += log.size();
            
            if (max_version < log.size()) {
                max_version =  log.size();
                smax_version_file = log.path_name();
            }

            line_out   << *pcollection;
            offset_out << log.path_name() << " " << file_offset << "\002" << endl;
            file_offset += pcollection->lines();
            
            delete pcollection;
        }
    } 
}
