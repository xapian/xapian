#ifndef __CODE_TO_HTML_H__
#define __CODE_TO_HTML_H__

#include "virtual_ostream.h"
#include <string>
using std::string;

class code_to_html : public virtual_ostream {
private:
    const string & _line;
    unsigned int _width;
protected:
    ostream & show(ostream &) const;
public:
    code_to_html(const string & line, unsigned int width) : _line(line), _width(width) {}
};

#endif
