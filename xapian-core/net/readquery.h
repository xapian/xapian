#include <om/omtypes.h>
#include <string>
#include "netutils.h"

struct querytok {
    enum etype {
	END,
	NULL_QUERY,
	BOOL_FLAG,
	OP_AND,
	OP_OR,
	OP_FILTER,
	OP_ANDMAYBE,
	OP_ANDNOT,
	OP_XOR,
	OP_NEAR,
	OP_PHRASE,
	TERM,
	OP_BRA,
	OP_KET,
	ERROR
    } type;
    om_termname tname;
    om_termcount wqf;
    om_termpos term_pos;
    om_termpos window; // for NEAR and PHRASE

    querytok(etype type_ = ERROR)
	    : type(type_) {}
    querytok(int type_)
	    : type(static_cast<etype>(type_)) {}
};

void qfs_start(string text);
querytok qfs_gettok();
void qfs_end();
