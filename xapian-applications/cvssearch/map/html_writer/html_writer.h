#ifndef __HTML_WRITER_H__
#define __HTML_WRITER_H__

#include "virtual_ostream.h"
#include <string>
using std::string;

class html_writer : public virtual_ostream 
{
protected:
    const string _title;
    ostream & show (ostream &) const;
    virtual ostream & write(ostream &) const = 0;
    ostream & init (ostream &) const;
    ostream & last (ostream &) const;
    virtual ostream & style(ostream &) const = 0;
public:
    html_writer(const string & title) : _title(title) {}
};


#endif
