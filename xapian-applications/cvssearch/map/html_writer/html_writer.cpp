#include "html_writer.h"

ostream & 
html_writer::show (ostream & os) const
{
    return last(write(init(os)));
}

ostream & html_writer::init (ostream & os) const
{
    os << "<HTML>" << endl;
    os << "<HEAD>" << endl;
    os << "<TITLE>" << _title << "</TITLE>" << endl;
    style(os);
    os << "</HEAD>" << endl;
    os << "<BODY>" << endl;
    return os;
}

ostream & html_writer::last (ostream & os) const
{
    os << "</BODY>" << endl;
    os << "</HTML>" << endl;
    return os;
}
