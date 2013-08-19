
#include "lucene_fnmtable.h"
#include "config.h"
#include "debuglog.h"
#include <iostream>

using namespace std;

FnmBitsInfo::FnmBitsInfo()
        : has_indexed_field(false),
        has_term_vector(false),
        has_norm(false),
        has_payload(false),
        has_freq_pos(false),
        has_position(false)
{
}

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

    transfer_bits();

    return ;
}

vector<string>
LuceneFnmTable::get_field_name() const {
    return field_name;
}

void
LuceneFnmTable::transfer_bits() {
    LOGCALL(DB, void, "LuceneFnmTable::transfer_bits", NO_ARGS);

    vector<char>::const_iterator it = field_bits.begin();
    for (; it != field_bits.end(); ++it) {
        FnmBitsInfo info;
        info.has_indexed_field = ((*it & 0x00000001) ? true : false);
        info.has_term_vector = ((*it & 0x00000010) ? true : false);
        info.has_norm = ((*it & 0x00010000) ? false : true);
        info.has_payload = ((*it & 0x00100000) ? true : false);
        info.has_freq_pos = ((*it & 0x01000000) ? false : true);
        info.has_position = ((*it & 0x10000000) ? false : true);

        LOGLINE(DB, "LuceneFnmTable::transfer_bits, indexed_field=" << info.has_indexed_field <<
                    ", has_term_vector=" << info.has_term_vector << ", has_norm=" <<
                    info.has_norm << ", has_payload=" << info.has_payload << ", has_position=" <<
                    info.has_position);

        field_bitsinfo.push_back(info);
    }
}

int
LuceneFnmTable::get_field_num(const string & fn) const {
    //LOGCALL(DB, int, "LuceneFnmTable::get_field_num", fn);

    map<string, int>::const_iterator it = field_map.find(fn);
    if (it == field_map.end()) {
        //-1 means error
        return -1;
    }

    return it->second;
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
