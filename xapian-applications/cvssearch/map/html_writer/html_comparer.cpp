#include "html_comparer.h"
#include <strstream>

html_comparer::html_comparer(const vector<unsigned int> & inputs, 
                             const set<unsigned int> & adds,
                             const set<unsigned int> & changes,
                             const set<unsigned int> & deletes,
                             const string & filename, 
                             const string & pathname,
                             const cvs_revision & revision, 
                             const cvs_revision & revision1, const cvs_revision & revision2, const diff & diff)
    : html_writer(string("comparing")),
      _aligns(inputs),
      _adds(adds),
      _changes(changes),
      _deletes(deletes),
      _revision(revision),
      _revision1(revision1),
      _revision2(revision2),
      _filename(filename),
      _pathname(pathname),
      _diff(diff)
{
    p1 = p2 = 0;
    ostrstream ost1;
    ost1 << "cvs update -p -r" << revision 
         << " " << filename << " 2>/dev/null" << ends;
    
    ostrstream ost2;
    ost2 << "cvs update -p -r" << revision2 
         << " " << filename << " 2>/dev/null" << ends;

    p1 = new process(ost1.str());
    p2 = new process(ost2.str());
}

html_comparer::~html_comparer()
{
    delete p1;
    delete p2;
}

void
html_comparer::get_class_type (unsigned int index1, unsigned int index2, string & select1, string & select2, const diff_type & type) const
{
    if (0) {
    } else if (_adds.find(index2) != _adds.end() && type == e_add) {
        select1 = " class=\"current\"";
        select2 = " class=\"add\"";
    } else if (_changes.find(index1) != _changes.end() && type == e_change) {
        select1 = " class=\"change\"";
        select2 = " class=\"change\"";
    } else if (_deletes.find(index1) != _deletes.end() && type == e_delete) {
        select1 = " class=\"delete\"";
        select2 = " class=\"current\"";
    } else {
        select1 = "";
        select2 = "";
    }
}

static void write_line(ostream & os, 
                       const string & select1, unsigned int index1, const string & line1, bool do1,
                       const string & select2, unsigned int index2, const string & line2, bool do2)
{
    os << "<TR>";
    os << "<TD" << select2 << "> "; if (do2) os << index2; os << "</TD>";
    os << "<TD" << select2 << "> "; if (do2) os << line2 ; os << "</TD>";
    os << "<TD" << select1 << "> "; if (do1) os << index1; os << "</TD>";
    os << "<TD" << select1 << "> "; if (do1) os << line1 ; os << "</TD>";
    os << "</TR>" << endl;
}

ostream &
html_comparer::write(ostream & os) const
{
    string select1 = " class=\"select\"";
    string select2 = " class=\"select\"";
    
    os << "<H3 align=center>aligned diff for " << _pathname 
       << " between "
       << " version " << _revision2
       << " and"
       << " version " << _revision1
       << "</H3>" << endl
       << " Here we show where the differences occurred between "
       << " version " << _revision2
       << " and"
       << " revision" << _revision1
       << ", and the propagation of the affected lines to version " << _revision 
       << endl;

    os << "<TABLE align=center border=0 width=100% cellspacing=0 cellpadding=1>" << endl;
    os << "<TR>";
    os << "<TD" << select2 << "> </TD>";
    os << "<TD" << select2 << "align=center> <B>version " << _revision2 << "</B></TD>";
    os << "<TD" << select1 << "> </TD>";
    os << "<TD" << select1 << "align=center> <B>version " << _revision  << "</B></TD>";
    os << "</TR>" << endl;

    unsigned int index1 = 1, index2 = 1;
    string line1, line2;

    istream *pis1, *pis2;


    if (p1 && (pis1 = p1->process_output()) &&
        p2 && (pis2 = p2->process_output()))
    {
        while (*pis1 && *pis2)
        {
            if (0) {
            } else if (_aligns[index1] == index2) {
                getline(*pis1, line1);
                getline(*pis2, line2);
                get_class_type (index1, index2, select1, select2, e_change);
                write_line(os,
                           select1, index1, line1, true,
                           select2, index2, line2, true);
                ++index1;
                ++index2;
            } else if (_aligns[index1] > index2) {
                getline(*pis2, line2);
                get_class_type (index1, index2, select1, select2, e_add);
                write_line(os,
                           select1, index1, line1, false,
                           select2, index2, line2, true);
                ++index2;
            } else if (_aligns[index1] < index2) {
                getline(*pis1, line1);
                get_class_type (index1, index2, select1, select2, e_delete);
                write_line(os,
                           select1, index1, line1, true,
                           select2, index2, line2, false);
                ++index1;
            }
        }
    }
    os << "</TABLE>" << endl;
    os << "<HR width=100%>" << endl;
    os << "<TABLE border=0>" << endl;
    os << "<TR><TD colspan=2>Legend:</TD></TR>" << endl;
    os << "<TR><TD class=\"current\"> </TD><TD align=center class=\"delete\"> added in v."
       << _revision1 << " and propagated to v." << _revision << "</TD></TR>" << endl;
    os << "<TR><TD colspan=2 align=center class=\"change\"> changed lines from v." << _revision2 << " to v." 
       << _revision1 << " and propagated to v." << _revision << "</TD></TR>" << endl;
    os << "<TR><TD align=center class=\"add\"> removed in v." << _revision1 << "</TD><TD class=\"current\"> </TD></TR>" << endl;
    os << "</TABLE>" << endl;

    return os;
}

ostream &
html_comparer::style(ostream & os) const
{
    os << "<STYLE TYPE-\"type/css\">" << endl;
    os << "BODY  {background-color:#EEEEEE;}" << endl;
    os << "TABLE {background-color:#FFFFFF;}" << endl;
    os << "TD    {font-family:fixed;white-space:pre; overflow:hidden}" << endl;
    os << ".select {background-color:yellow;}" << endl;
    os << ".delete {background-color:#CCCCFF;}" << endl;
    os << ".change {background-color:#99FF99;}" << endl;
    os << ".add    {background-color:#FF9999;}" << endl;
    os << ".current{background-color:#CCCCCC;}" << endl;
    os << "</STYLE>" << endl;
    return os;
}
