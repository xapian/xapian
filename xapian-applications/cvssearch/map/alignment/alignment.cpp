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
    ostrstream ost;
    ost << "diff -b " << f1 << " " << f2 << ends;
    string command = ost.str();
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
