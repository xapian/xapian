#include "map_algorithm.h"

istream &
map_algorithm::read(istream & is)
{
    cvs_log log;
    is >> log;
    if (log.read_status())
    {
        parse_log(log);
    }
    return is;
}

void
map_algorithm::parse_diff(const cvs_log_entry & log_entry1, const cvs_log_entry & log_entry2, const diff & diff)
{
    for (unsigned int j = 0; j < diff.size(); ++j)
    {
        parse_diff_entry(log_entry1, diff[j]);
    }
}


map_algorithm::map_algorithm()
    :_updates(0),
     _deletes(0),
     _searches(0)
{
}
