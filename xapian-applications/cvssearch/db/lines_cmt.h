#ifndef __LINES_CMT_H__
#define __LINES_CMT_H__

#include <fstream>
#include "lines.h"

class lines_cmt : public lines
{
private:
    ifstream *in_comments;
    ifstream *in_code;

    string prev_file;

    unsigned int line_no;
    string path;
    int current_offset;
    int file_count;

    vector<string> files;
    vector<string> offsets;
    string root;
    string package;
    string message;

protected:
    void load_offset_file(  const string& file_offset, vector<string>& files, vector<string>& offsets );
    void readVector( const string& line, const string& field, vector<string>& field_vector );
public:
    lines_cmt( const string& p,           // path (e.g., "cvsdata/root0/src/")
               const string& sroot,       // ?????????
               const string& pkg,         // package 
               const string& file_db,     // name of cmt file
               const string& file_offset, // name of offset file 
               const string& mes);
    
    ~lines_cmt() ;
    bool readNextLine();
    unsigned int        getLineNumber() const { return line_no; }
};

#endif
