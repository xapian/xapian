/************************************************************
 *
 *  alignment.cpp main entry point to align two files.
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
 *
 *  Usage:
 *
 *  $>alignment file1 file2
 * 
 *  $Id$
 *
 ************************************************************/

#include <string>
#include <assert.h>

#include "line_sequence.h"
#include "alignment.h"
#include "process.h"
#include "diff.h"

int 
main( int argc, char *argv[] ) 
{
    assert( argc == 3 );

    string f1 = argv[1];
    string f2 = argv[2];

    diff d;
    string command = "diff -b " + f1 + ' ' + f2;
    process p = process(command);
    istream *pis = p.process_output();
    if (pis)
    {
        *pis >> d;
    }

    for (unsigned int i = 0; i < d.size(); ++i)
    {
        const diff_entry & entry = d[i];
        if (entry.type() == e_change)
        {
            line_sequence l1(entry.source_line());
            line_sequence l2(entry.dest_line());
            alignment<line_sequence> alignment_o( l1, l2, entry.source().begin()-1, entry.dest().begin()-1);
            alignment_o.find_optimal_alignment();
            cout << alignment_o;
        }
        else
        {
            cout << entry << endl;
        }
    }
    return 0;
}
