/* error.h
 */

#ifndef _error_h_
#define _error_h_

#include <string>

#include "omtypes.h"

class OmError {
    private:
        string msg;
    public:
        OmError(const string &error_msg)
        {
            msg = error_msg;
        }
        string get_msg()
        {
            return msg;
        }
};

class RangeError : public OmError {
    public:
	RangeError(const string &msg) : OmError(msg) {};
};

class OpeningError : public OmError {
    public:
        OpeningError(const string &msg) : OmError(msg) {};
};

class AssertionFailed : public OmError {
    public:
        AssertionFailed(const string &msg) : OmError(msg + " - assertion failed") {};
};

#endif /* _error_h_ */
