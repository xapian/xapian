#ifndef __SEQUENCE_H__
#define __SEQUENCE_H__

#include <vector>
using std::vector;

template<class T>
class sequence 
{
protected:
    vector<T> _entries;
public:
    sequence() {}
    sequence(const vector<T> & entries) : _entries(entries) {}

    unsigned int size() const {  return _entries.size(); }
    const T & operator[](unsigned int i) const { assert (i > 0 && i <= size()); return _entries[i-1]; }
          T & operator[](unsigned int i)        { assert (i > 0 && i <= size()); return _entries[i-1]; }
};

#endif
