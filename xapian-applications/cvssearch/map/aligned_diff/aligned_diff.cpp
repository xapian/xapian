#include "aligned_diff.h"
#include "alignment.h"
#include "line_sequence.h"
#include "process.h"

istream &
aligned_diff::read(istream & is)
{
    // ----------------------------------------
    // read each diff entry
    // ----------------------------------------
    while (is)
    {
        diff_entry entry;
        is >> entry;
        if (entry.read_status())
        {
            if (entry.type() == e_change)
            {
                line_sequence l1(entry.source_line());
                line_sequence l2(entry.dest_line());
                // cout << "BEGIN " << entry.source().begin()-1 << " " << entry.dest().begin()-1 << endl;
                alignment<line_sequence> alignment_o( l1, l2, entry.source().begin()-1, entry.dest().begin()-1);
                alignment_o.find_optimal_alignment();
//                cout << alignment_o;
                const list<diff_entry> & aligned_entries = alignment_o.entries();
                list<diff_entry>::const_iterator itr;
                for (itr = aligned_entries.begin();
                     itr!= aligned_entries.end();
                     ++itr)
                {
                    _entries.push_back(*itr);
                }
            }
            else
            {
                _entries.push_back(entry);
            }
        }
        else
        {
            break;
        }
    }

    // ----------------------------------------
    // because the diff entry are read in
    // increasing order, each entry affects
    // all subsequent entries, but the source 
    // range produced by cvs diff 
    // refers to the old position
    //
    // e.g.
    // 2a3,4   <- this causes a shift of +2
    //            add to the source of subsequent
    //            entries.
    // 
    // 5,6c7,8 <- this causes no shift
    // ----------------------------------------
    int offset = 0;
    for (unsigned int i = 0; i < _entries.size(); ++i)
    {
        try {
            _entries[i].source() += offset;
        }
        catch (range_exception & e)
        {
            cerr << e;
        }
        offset += _entries[i].size();
    }
     return is;
}

// int 
// main( int argc, char *argv[] ) 
// {
//     aligned_diff d;
//     cin >> d;
//     for (unsigned int i = 0; i < d.size(); ++i)
//     {
//         const diff_entry & entry = d[i];
//         cout << entry << endl;
//     }
//     return 0;
// }
