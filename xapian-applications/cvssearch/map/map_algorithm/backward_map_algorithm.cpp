#include "backward_map_algorithm.h"
#include <strstream>
#include "process.h"
#include "aligned_diff.h"

static unsigned int get_length(const cvs_log & log, unsigned int j)
{
    ostrstream ost;
    ost << "cvs update -p -r" << log[j].revision() 
        << " " << log.file_name() << " 2>/dev/null|wc -l" << ends;
    process p(ost.str());
    istream *pis = p.process_output();
    unsigned int length = 0;
    if (*pis)
    {
        *pis >> length;
    }
    return length;
}

void
backward_map_algorithm::parse_log(const cvs_log & log)
{
    for (unsigned int j = 0; j < log.size(); ++j)
    {
        ostrstream ost;
        if (j == 0)
        {
            unsigned int length = get_length(log, j);
            init(length);
        }

        if (j == log.size()-1)
        {
            unsigned int length = get_length(log, j);
            if (length > 0)
            {
                ostrstream ost2;
                ost2 << "1," << length << "d0" << endl << ends;
                istrstream ist(ost2.str());
                diff diff_output;
                ist >> diff_output;

                if (diff_output.read_status())
                {
                    parse_diff(log[j], diff_output);
                }
            }
        }
        else
        {
            ost << "cvs diff -b " 
                << "-r" << log[j].revision()   << " " 
                << "-r" << log[j+1].revision() << " "
                << log.file_name() << ends;
            
            process p(ost.str());
            istream *pis = p.process_output();
            if (*pis)
            {    
                aligned_diff diff_output;
                *pis >> diff_output;
                parse_diff(log[j], diff_output);
            }
        }
    }
}

