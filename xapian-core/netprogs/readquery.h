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

inline char hextochar(char high, char low)
{
    int h;
    if (high >= '0' && high <= '9') {
	h = high - '0';
    } else {
	high = toupper(high);
	h = high - 'A' + 10;
    }
    int l;
    if (low >= '0' && low <= '9') {
	l = low - '0';
    } else {
	low = toupper(low);
	l = low - 'A' + 10;
    }
    return l + (h << 4);
}

void qfs_start(string text);
querytok qfs_gettok();
void qfs_end();
