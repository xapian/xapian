
#include "lucene_fnmtable.h"
#include "config.h"
#include "debuglog.h"
#include <iostream>

using namespace std;

LuceneFnmTable::LuceneFnmTable(const string &db_dir_) 
        : db_dir(db_dir_),
        stream_reader(db_dir_)
{
}

bool
LuceneFnmTable::set_filename(const string & prefix) {
    file_name = prefix + ".fnm";
    stream_reader.set_filename(file_name);

    return true;
}

void
LuceneFnmTable::open() {
    LOGCALL(DB, bool, "LuceneFnmTable::open", NO_ARGS);

    stream_reader.open_stream();

    stream_reader.read_vint32(fnm_version);
    stream_reader.read_vint32(fields_count);
    string name;
    char bit;
    for (int i = 0; i < fields_count; ++i) {
        stream_reader.read_string(name);
        stream_reader.read_byte(bit);
        field_name.push_back(name);
        field_bits.push_back(bit);

        field_map.insert(pair<string, int>(name, i));
    }

    return ;
}

vector<string>
LuceneFnmTable::get_field_name() const {
    return field_name;
}

/**
 * below is for debug
 */
void LuceneFnmTable::debug_get_table() {
    cout << "fnm-->FNMVersion[" << fnm_version << "],fieldsCount[" <<
        fields_count << "],fields<" << endl;
    for (int i = 0; i < fields_count; ++i) {
        cout << "FieldName[" << field_name[i] << "], bits[";
        for (int j = 7; j >= 0; --j) {
            int t = (field_bits[i] >> j) & 0x00000001;
            cout << t << ",";
        }
        cout << "]," << endl;
    }

    cout << ">" << endl;
}
