#include "forward_map_algorithm.h"
#include <strstream>
#include "process.h"
#include "aligned_diff.h"


void
forward_map_algorithm::parse_log(const cvs_log & log)
{
    for (unsigned int j = log.size(); j >= 1; --j)
    {
        ostrstream ost;
        if (j <= log.size() -1)
        {
            ost << "cvs diff -b " 
                << "-r" << log[j].revision()   << " " 
                << "-r" << log[j-1].revision() << " "
                << log.file_name() << ends;
            
            process * p;
            if ((p = new process(ost.str())) != 0)
            {
                istream *pis = p->process_output();
                if (*pis)
                {    
                    aligned_diff diff_output;
                    *pis >> diff_output;
                    parse_diff(log[j-1], log[j], diff_output);
                }
                delete p;
            }
        }
        else
        {
            ost << "cvs update -p -r" << log[j-1].revision() 
                << " " << log.file_name() << " 2>/dev/null|wc -l" << ends;
            process * p;
            if ((p = new process(ost.str())) != 0)
            {
                istream *pis = p->process_output();
                if (*pis)
                {
                    unsigned int length;
                    *pis >> length;
                    ostrstream ost2;
                    if (length > 0)
                    {
                        ost2 << "0a1," << length << endl << ends;
                        diff diff_output;
                        istrstream ist(ost2.str());
                        ist >> diff_output;
                        parse_diff(log[j-1], log[j], diff_output);
                    }
                }
                delete p;
            }
        }
    }
}

