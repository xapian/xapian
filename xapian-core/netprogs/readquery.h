#include <om/omtypes.h>
#include <string>

struct querytok {
    enum etype {
	END,
	NULL_QUERY,
	OP_AND,
	OP_OR,
	OP_FILTER,
	OP_ANDMAYBE,
	OP_ANDNOT,
	OP_XOR,
	TERM,
	OP_BRA,
	OP_KET,
	ERROR
    } type;
    om_termname tname;
    om_termcount wqf;
    om_termpos term_pos;

    querytok(etype type_ = ERROR)
	    : type(type_) {}
    querytok(int type_)
	    : type(static_cast<etype>(type_)) {}
};

char hextochar(char high, char low);

void qfs_start(string text);
querytok qfs_gettok();
void qfs_end();
