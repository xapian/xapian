#ifndef __COLLECTION_H__
#define __COLLECTION_H__

#include <vector>

template<class T>
class collection
{
protected:
    vector<T> _entries;
public:
    collection() {}
    collection(const vector<T> & entries) : _entries(entries) {}
    unsigned int size() const {  return _entries.size(); }
    const T & operator[](unsigned int i) const { assert (i < size()); return _entries[i]; }
          T & operator[](unsigned int i)       { assert (i < size()); return _entries[i]; }
};

#endif
