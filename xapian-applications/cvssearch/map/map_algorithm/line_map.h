#ifndef __LINE_MAP_H__
#define __LINE_MAP_H__

#include "virtual_ostream.h"
#include "cvs_log_entry.h"
#include <vector>
using std::vector;

class line_map : public virtual_ostream
{
private:
    vector<const cvs_log_entry *> _entries;
    unsigned int _index;
protected:
    ostream & show(ostream &) const;
public:
    line_map(unsigned int i) : _index(i) {}
    virtual ~line_map() {}
    unsigned int line() const { return _index;}
    unsigned int size() const { return _entries.size(); }
    void add_log_entry(const cvs_log_entry & e) { _entries.push_back(&e);}
};

#endif
