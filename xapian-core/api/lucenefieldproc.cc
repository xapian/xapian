
#include <config.h>
#include <xapian.h>
#include "debuglog.h"

#include <string>

using namespace std;

namespace Xapian {

Xapian::Query
LuceneFieldProcessor::operator()(const string & str) {
    LOGCALL(API, Xapian::Query, "LuceneFieldProcessor::operator", str);

    return Xapian::Query(prefix + ":" + str);
}

}
