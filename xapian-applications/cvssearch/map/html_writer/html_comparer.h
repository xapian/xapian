#ifndef __HTML_COMPARER_H__
#define __HTML_COMPARER_H__

#include "virtual_ostream.h"
#include "html_writer.h"
#include "cvs_revision.h"
#include "process.h"
#include "diff.h"
#include <vector>
using std::vector;
#include <set>
using std::set;

class html_comparer : public html_writer 
{
private:
    process * p1;
    process * p2;
    const vector<unsigned int> & _aligns;
    const set<unsigned int> & _adds;
    const set<unsigned int> & _changes;
    const set<unsigned int> & _deletes;
    const cvs_revision & _revision;
    const cvs_revision & _revision1;
    const cvs_revision & _revision2;
    const string & _filename;
    const string & _pathname;
    const diff & _diff;
    unsigned int _diff_index;
    ostream & write (ostream &) const;
    void get_class_type (unsigned int index1, unsigned int index2, string & select1, string & select2, const diff_type & type) const;
protected:
    virtual ostream & style(ostream &) const;    
public:
    html_comparer(const vector<unsigned int> & inputs, 
                  const set<unsigned int> & deletes,
                  const set<unsigned int> & changes,
                  const set<unsigned int> & adds,
                  const string & filename, 
                  const string & pathname,
                  const cvs_revision & revision,
                  const cvs_revision & revision1, const cvs_revision & revision2,
                  const diff & diff);
    ~html_comparer();
};

#endif
