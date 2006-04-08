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
#include "parse_change_log.h"
#include <vector>

using std::vector;

string scvs_root;


string scvs_log    = "cvs -l -f log -b ";
string scvs_diff   = "cvs -l -f diff -N -b ";
string scvs_update = "cvs -l -f update -p ";
string slatest_version = "";
string sversion    = "";

unsigned int width = 40;

bool use_html = false;
bool short_html = false;
bool read_mode = false;
bool comp_mode = false;

static string smax_version_file;
static string scmt_db     = "/dev/null";
static string soffset_db  = "/dev/null";
static string sfile_db    = "/dev/null";
static string sstats_file = "/dev/null";
static string smodule_name= "";
static string scvs2cl_path= "";

static unsigned int num_mappings = 0;
static unsigned int num_deletes = 0;
static unsigned int num_updates = 0;
static unsigned int num_searches = 0;
static unsigned int num_lines = 0;
static unsigned int num_version = 0;
static unsigned int max_version = 0;
static unsigned int file_index = 1;
static unsigned int file_offset = 1;



static const int ssync_rate = 20;
static bool use_db = false;
static bool use_line = true;

static void usage(const string & prog_name);
static void cvsmap(istream & log_in, ostream & line_out, ostream & file_out, cvs_db_file *);

int 
main(unsigned int argc, const char **argv)
{
    unsigned int i = 1;
    string input_file = "";

    while (i < argc)
    {
        if (0) {
        } else if (!strcmp(argv[i], "-m") && i+1 < argc) {
            smodule_name = argv[++i];
        } else if (!strcmp(argv[i], "-cl") && i+1 < argc) {
            scvs2cl_path = argv[++i];
        } else if (!strcmp(argv[i], "-i") && i+1 < argc) {
            input_file = argv[++i];
        } else if (!strcmp(argv[i], "-f1") && i+1 < argc) {
            scmt_db = argv[++i];
        } else if (!strcmp(argv[i], "-f2") && i+1 < argc) {
            soffset_db = argv[++i];
        } else if (!strcmp(argv[i], "-s")) {
            short_html = true;
        } else if (!strcmp(argv[i], "-html") && i+3 < argc) {
            file_index = atoi(argv[++i]);
            sversion = argv[++i];
            width = atoi(argv[++i]);
            use_html = true;
            read_mode = true;
        } else if (!strcmp(argv[i], "-r")) {
            slatest_version = argv[++i];
        } else if (!strcmp(argv[i], "-h")) {
            usage(argv[0]);
        } else if (!strcmp(argv[i], "-d") && i+1 < argc) {
            scvs_root = argv[++i];
        } else if (!strcmp(argv[i], "-db") && i+1 < argc) {
            sfile_db = argv[++i];
            use_db = true;
        } else if (!strcmp(argv[i], "-st") && i+1 < argc) {
            sstats_file = argv[++i];
        } else if (!strcmp(argv[i], "-comp")) {
            comp_mode = true;
        } else {
            break;
        }
        ++i;
    }

    // ----------------------------------------
    // determine what the cvsroot is,
    // make sure cvs use it explicitly.
    // ----------------------------------------
    if (scvs_root.length() == 0)
    {
        usage(argv[0]);
        exit(1);
    }

    scvs_log    = "cvs -l -f -d " + scvs_root + " log -b ";
    scvs_diff   = "cvs -l -f -d " + scvs_root + " diff -kk -N -b ";
    scvs_update = "cvs -l -f -d " + scvs_root + " update -p ";

    ofstream cmt_fout   (scmt_db.c_str());
    ofstream offset_fout (soffset_db.c_str());
    ofstream stats_fout  (sstats_file.c_str());
    
    cvs_db_file * pdb_file = 0;
    if (use_db)
    {
        if (!read_mode && (smodule_name.length() == 0 || scvs2cl_path.length() == 0)) 
        {
            usage(argv[0]);
            exit(1);
        }
        unlink(sfile_db.c_str());
        pdb_file = new cvs_db_file(sfile_db, read_mode);
    }

    if (input_file == "")
    {
        while (i < argc)
        {
            string command = scvs_log + argv[i++];
            process p = process(command);
            istream * pis = p.process_output();
            if (pis)
            {
                cerr << "... mapping " << argv[i-1] << endl;
                cvsmap(*pis, cmt_fout, offset_fout, pdb_file);
                if (pdb_file && !read_mode && i % ssync_rate == 0)
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
            istream * pis = p.process_output();
            if (pis)
            {
                cerr << "... mapping " << line << endl;
                cvsmap(*pis, cmt_fout, offset_fout, pdb_file);
                if (pdb_file && !read_mode && i % ssync_rate == 0)
                {
                    pdb_file->sync();
                }
            }
            i++;
        }
    }
    
    // ----------------------------------------
    // print statistics information
    // ----------------------------------------
    if (!use_html && file_index != 0 && num_lines != 0)
    {
        stats_fout
            << "total   # of files             :\t" << file_index << endl
            << "total   # of lines of code     :\t" << num_lines << endl
            << "maximum # versions / file      :\t" << max_version << endl
            << "              the file is      :\t" << smax_version_file << endl
            << "average # versions / file      :\t" << num_version / file_index << endl
            << "average # cvs comments / line  :\t" << (double) num_mappings / (double) num_lines << endl
            ;
    }

    if (pdb_file)
    {
        if (!read_mode) {
            parse_change_log (smodule_name, scvs2cl_path, *pdb_file);
        }
        delete pdb_file;
    }


    exit (EXIT_SUCCESS);
}

static void usage(const string & prog_name)
{
    cerr << "Usage: " << prog_name << " -d CVSROOT [Options] ..." << endl
         << endl
         << "Options:" << endl
         << "  -h                   print this message" << endl
         << "  -i                   input file contain a list of files to map" << endl
         << "  -r version           backtrack from version instead from the latest version" << endl
         << "  -html fileid version output html comparason result" << endl
         << "  -s                   display abbreviated html output" << endl
         << "  -db pkg.db           specify the database file" << endl
         << "  -m                   module name (MUST SPECIFY), used only if -db option is turned on" << endl
         << "  -cl                  cvs2cl path (MUST SPECIFY), used only if -db option is turned on" << endl
         << "  -f1 pkg.cmt          specify the line-comment map file" << endl
         << "  -f2 pkg.offset       specify the filename index file" <<endl
         << "  -st stat.file        output statistical information to stat.file" << endl
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
            pcollection = new backward_line_map_algorithm(log, file_index++, pdb_file);
        } else {
            pcollection = new forward_range_map_algorithm(log, file_index++, pdb_file);
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
            offset_out << smodule_name << "/" << log.path_name() << " " << file_offset << "\002" << endl;
            file_offset += pcollection->lines();
            
            delete pcollection;
        }
    } 
}
