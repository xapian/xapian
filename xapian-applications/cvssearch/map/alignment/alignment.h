#ifndef __ALIGNMENT_H__
#define __ALIGNMENT_H__

#include "virtual_iostream.h"
#include "diff_entry.h"

#include <vector>
#include <list>
#include <iostream>
#include <string>
using std::vector;
using std::ostream;
using std::list;
using std::string;

template<class T>
class alignment : public virtual_iostream
{
private:
    const T &S;
    const T &D;
    vector< vector< int > > V;
    list<diff_entry> _entries;
    unsigned int _source_offset;
    unsigned int _dest_offset;
    ostream & diff_output(ostream &, 
                          unsigned int s1,
                          unsigned int s2,
                          unsigned int d1,
                          unsigned int d2,
                          diff_type type) const;
protected:
    istream & read(istream &);
    ostream & show(ostream &) const;
public:
    alignment(const T &s, const T &d, unsigned int source_offset = 0, unsigned int dest_offset = 0);
    void find_optimal_alignment(bool = true);
    int optimal_alignment_value() const;

    const list<diff_entry> & entries() const {return _entries;}
    ostream & dump (ostream &) const;
    ostream & diff1(ostream &) const;
    ostream & diff2(ostream &) const;                          
                        
};

#include "alignment.hpp"
#endif
