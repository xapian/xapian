/************************************************************
 *
 *  cvsquery.cpp the main entry point to query the database.
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
 *  $>cvsquery -h
 *
 *  $Id$
 *
 ************************************************************/

#include "cvs_db_file.h"
#include <string>
using std::string;

static void usage(char * progname);

static void query_filename        (cvs_db_file & db_file, unsigned int file_id);
static void query_comment         (cvs_db_file & db_file, unsigned int file_id, const string & revision);
static void query_line            (cvs_db_file & db_file, unsigned int file_id, const string & revision);
static void query_revision        (cvs_db_file & db_file, unsigned int file_id, unsigned int line);
static void query_revision_comment(cvs_db_file & db_file, unsigned int file_id);

int
main(unsigned int argc, char **argv) 
{
    unsigned int file_id = 0;
    unsigned int line = 0;
    string revision;
    string database_name;

    if (argc < 3 || !strcmp(argv[2], "-h")) {
        usage(argv[0]);
    }
   
    database_name = argv[1];
    cvs_db_file db_file(database_name);

    for (unsigned int i = 2; i < argc; ++i)
    {
        if (0) {
        } else if (!strcmp(argv[i],"-f") && i+2 <= argc) {
            file_id = strtoul(argv[++i], (char **) NULL, 10);

            query_filename(db_file, file_id);
        } else if (!strcmp(argv[i],"-c") && i+3 <= argc) {
            file_id = strtoul(argv[++i], (char **) NULL, 10);
            revision = argv[++i];

            query_comment(db_file, file_id, revision);
        } else if (!strcmp(argv[i],"-r") && i+3 <= argc) {
            file_id = strtoul(argv[++i], (char **) NULL, 10);
            line = strtoul(argv[++i], (char **) NULL, 10);

            query_revision(db_file, file_id, line);
        } else if (!strcmp(argv[i],"-l") && i+ 3 <= argc) {
            file_id = strtoul(argv[++i], (char **) NULL, 10);
            revision = argv[++i];

            query_line(db_file, file_id, revision);
        } else if (!strcmp(argv[i],"-a") && i+ 2 <= argc) {
            file_id = strtoul(argv[++i], (char **) NULL, 10);

            query_revision_comment(db_file, file_id);
        } else if (!strcmp(argv[i],"-h")) {
            usage(argv[0]);
        }
    }
    return 0;
}

void
usage(char * prog_name)
{
    cerr << "Usage: " << prog_name << " DatabaseFile [Options] [Options] ..." << endl
         << endl
         << "Options:" << endl
         << "  -h                     print out this message" << endl
         << "  -c file_id revision    query for cvs comments from a file_id and a revision" << endl
         << "  -r file_id line        query for revisions from a file_id and a line"<< endl
         << "  -l file_id revision    query for lines from a file_id and a revision" << endl
         << "  -a file_id             query for all revision,comment pairs" <<endl;
    
    exit(0);
}

void query_filename(cvs_db_file & db_file, unsigned int file_id)
{
    string filename;
    if (db_file.get_filename(file_id, filename) == 0)
    {
        cout << filename << endl;
    }
}

void query_comment(cvs_db_file & db_file, unsigned int file_id, const string & revision)
{
    string comment;
    if (db_file.get_comment(file_id, revision, comment) == 0)
    {
        cout << comment << endl;
    }
}

void query_revision(cvs_db_file & db_file, unsigned int file_id, unsigned int line)
{
    set<string, cvs_revision_less> revisions;
    if (db_file.get_revision(file_id, line, revisions) == 0)
    {
        set<string, cvs_revision_less>::iterator itr;
        for (itr = revisions.begin(); itr != revisions.end(); ++itr)
        {
            cout << *itr << endl;
        }
    }
}

void query_line(cvs_db_file & db_file, unsigned int file_id, const string & revision)
{
    set<unsigned int> lines;
    if (db_file.get_line(file_id, revision, lines) == 0)
    {
        set<unsigned int>::iterator itr;
        for (itr = lines.begin(); itr != lines.end(); ++itr)
        {
            cout << *itr << endl;
        }
    }
}

void query_revision_comment(cvs_db_file & db_file, unsigned int file_id)
{
    set<string, cvs_revision_less> revisions;
    vector<string> comments;
    if (db_file.get_revision_comment(file_id, revisions, comments) == 0)
    {
        set<string, cvs_revision_less>::iterator itr;
        unsigned int i;
        for (itr = revisions.begin(), i = 0;
             i < comments.size() && itr!= revisions.end();
             ++i, ++itr)
        {
            cout << *itr << "^C" << comments[i] << "^B" << endl;
        }
    }
}
