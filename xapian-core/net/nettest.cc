#include "progclient.h"
#include <om/omenquire.h>
#include <typeinfo>

int main()
{
    try {
	OmDatabase db;
	vector<string> params;
	params.push_back("prog");
	params.push_back("./omnetclient");
	db.add_database("net", params);

	OmEnquire enq(db);

	enq.set_query(OmQuery("foo"));

	OmMSet mset(enq.get_mset(0, 10));
    } catch (OmError &e) {
	cout << "OmError exception (" << typeid(e).name()
	     << "): " << e.get_msg() << endl;
    }
}
