
#include <config.h>

#include <xapian/intrusive_ptr.h>

#include "debuglog.h"
#include "lucene_fdtxtable.h"
#include "lucene_segdb.h"
#include "internaltypes.h"

//this line is for debug
#include <cstdlib>
#include <iostream>

using namespace std;
using Xapian::Internal::intrusive_ptr;

LuceneFdtxTable::LuceneFdtxTable(const string & db_dir_)
        : db_dir(db_dir_),
          fdx_reader(db_dir),
          fdt_reader(db_dir)
{
    LOGCALL_CTOR(DB, "LuceneFdtxTable", NO_ARGS);
}

LuceneFdtxTable::~LuceneFdtxTable() {
    LOGCALL_DTOR(DB, "LuceneFdtxTable");
}

void
LuceneFdtxTable::set_fdx_filename(const string & prefix) {
    //The file name has fixed suffix '.fdx'
    fdx_filename = prefix + ".fdx";
    fdx_reader.set_filename(fdx_filename);

    return ;
}

void
LuceneFdtxTable::set_fdt_filename(const string & prefix) {
    //The file name has fixed suffix '.fdt'
    fdt_filename = prefix + ".fdt";
    fdt_reader.set_filename(fdt_filename);

    return ;
}

void
LuceneFdtxTable::set_filename(const string & prefix) {
    set_fdt_filename(prefix);
    set_fdx_filename(prefix);
}

bool
LuceneFdtxTable::open() {
    LOGCALL(DB, bool, "LuceneFdtxTable::open", fdt_filename | fdx_filename);

    //Just open files
    fdx_reader.open_stream();
    fdt_reader.open_stream();

    return true;
}

//TODO, just support data type of string now
void
LuceneFdtxTable::get_record(Xapian::docid did, map<int, string> & string_map,
            map<int, int> & int_map, map<int, long> & long_map,
            map<int, float> & float_map, map<int, double> double_map) const {
    LOGCALL(API, string, "(not realized)LuceneFdtxTable::get_record",
                did);

    //There's a Version num in the front of .fdx, which size is 4 bytes, so skip it
    int fdx_offset = 4 + did * 8;
    //TODO, seek_to must support int64 later
    fdx_reader.seek_to(fdx_offset);
    long long fdt_offset = 0;
    fdx_reader.read_int64(fdt_offset);

    fdt_reader.seek_to(fdt_offset);
    int field_count = 0;
    fdt_reader.read_vint32(field_count);

    /*
    cout << "LuceneFdtxTable::get_record field_count=" << field_count << 
        " fdx_offset=" << fdx_offset << ",fdt_offset=" << fdt_offset <<
        endl;
        */

    //Just read data type of string, others(tokenized/binary/compression/
    //numeric) not support yet
    for (int i = 0; i < field_count; ++i) {
        int field_num = 0;
        fdt_reader.read_vint32(field_num);

        char bits = '\0';
        fdt_reader.read_byte(bits);

        string record = "";
        fdt_reader.read_string(record);

        /*
        cout << "LuceneFdtxTable::get_record field_num=" << field_num << 
            ", bits=" << (int)bits << 
            ", record=" << record << endl;
            */
        string_map.insert(pair<int, string>(field_num, record));
    }

    (void)int_map;
    (void)long_map;
    (void)float_map;
    (void)double_map;
}

