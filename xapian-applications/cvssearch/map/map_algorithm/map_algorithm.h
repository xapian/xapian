#ifndef __MAP_ALGORITHM_H__
#define __MAP_ALGORITHM_H__

#include "cvs_log.h"
#include "diff.h"
#include "virtual_iostream.h"

class map_algorithm : public virtual_iostream
{
protected:
    istream & read(istream &);
    virtual void parse_diff(const cvs_log_entry & log_entry1, const cvs_log_entry & log_entry2, const diff &);
    virtual void parse_log(const cvs_log &) = 0;
    virtual void parse_diff_entry(const cvs_log_entry &, const diff_entry &) = 0;
    
    unsigned int _updates;
    unsigned int _deletes;
    unsigned int _searches;

public:
    virtual ~map_algorithm () {}
    map_algorithm();
    virtual unsigned int lines() const = 0;
    virtual unsigned int size()  const = 0;
    virtual unsigned int mappings()  const = 0;
    
    unsigned int updates()  const { return _updates;}
    unsigned int deletes()  const { return _deletes;}
    unsigned int searches() const { return _searches;}
};

#endif
