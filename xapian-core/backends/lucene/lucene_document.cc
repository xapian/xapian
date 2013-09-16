
#include <config.h>

#include "lucene_document.h"

using Xapian::Internal::intrusive_ptr;

LuceneDocument::LuceneDocument(intrusive_ptr<const Xapian::Database::Internal> db_,
            Xapian::docid did_,
            intrusive_ptr<const LuceneSegdb> seg_db_)
        : Xapian::Document::Internal(db_, did_),
          seg_db(seg_db_),
          has_read(false)
{
    LOGCALL_CTOR(DB, "LuceneDocument", did_);
}

LuceneDocument::~LuceneDocument() {
    LOGCALL_DTOR(DB, "LuceneDocument");
}

void
LuceneDocument::get_data() {
    LOGCALL(DB, void, "LuceneDocument::get_data", did);

    seg_db->get_record(did, string_map, int_map, long_map, float_map,
                double_map);
}

string
LuceneDocument::get_data_string(const string & field) {
    LOGCALL(DB, string, "LuceneDocument::get_data_string", field);

    if (false == has_read) {
        get_data();
        has_read = true;
    }

    int field_num = seg_db->get_fieldnum(field);
    map<int, string>::const_iterator it = string_map.begin();
    it = string_map.find(field_num);
    if (it == string_map.end()) {
        RETURN("");
    }

    RETURN(it->second);
}

int
LuceneDocument::get_data_int(const string & field) {
    LOGCALL(DB, int, "LuceneDocument::get_data_int", field);

    if (false == has_read) {
        get_data();
        has_read = true;
    }

    int field_num = seg_db->get_fieldnum(field);
    map<int, int>::const_iterator it = int_map.begin();
    it = int_map.find(field_num);
    //FIXME It's suitable to return 0 when no found
    if (it == int_map.end()) {
        RETURN(0);
    }

    RETURN(it->second);
}

long
LuceneDocument::get_data_long(const string & field) {
    LOGCALL(DB, long, "LuceneDocument::get_data_long", field);

    if (false == has_read) {
        get_data();
        has_read = true;
    }

    int field_num = seg_db->get_fieldnum(field);
    map<int, long>::const_iterator it = long_map.begin();
    it = long_map.find(field_num);
    if (it == long_map.end()) {
        RETURN(0.0);
    }

    RETURN(it->second);
}

float
LuceneDocument::get_data_float(const string & field) {
    LOGCALL(DB, float, "LuceneDocument::get_data_float", field);

    if (false == has_read) {
        get_data();
        has_read = true;
    }

    int field_num = seg_db->get_fieldnum(field);
    map<int, float>::const_iterator it = float_map.begin();
    it = float_map.find(field_num);
    if (it == float_map.end()) {
        RETURN(0.0);
    }

    RETURN(it->second);
}

double
LuceneDocument::get_data_double(const string & field) {
    LOGCALL(DB, double, "LuceneDocument::get_data_double", field);

    if (false == has_read) {
        get_data();
        has_read = true;
    }

    int field_num = seg_db->get_fieldnum(field);
    map<int, double>::const_iterator it = double_map.begin();
    it = double_map.find(field_num);
    if (it == double_map.end()) {
        RETURN(0.0);
    }

    RETURN(it->second);
}
