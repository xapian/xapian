#ifndef __CVS_REVISION_LESS_H__
#define __CVS_REVISION_LESS_H__

#include <functional>
#include <assert.h>

using namespace std;

class cvs_revision_less : public binary_function<string, string, bool> 
{
public:
    bool operator() (const string & r1, const string & r2) const { 
        unsigned int l1 = 0;
        unsigned int l2 = 0;
        
        unsigned int i1 = 0;
        unsigned int i2 = 0;
        while (i1 < r1.length() && i2 < r2.length())
        {
            if (r1[i1] == '.' && r2[i2] == '.')
            {
                if (l1 == l2)
                {
                    l1 = 0;
                    l2 = 0;
                    ++i1;
                    ++i2;
                    continue;
                }
                else {
                    return l1 < l2;
                }
            }

            if (r2[i2] != '.') 
            {
                assert(r2[i2] >= '0' &&
                       r2[i2] <= '9');
                l2 = l2 * 10 + (r2[i2]-'0');
                ++i2;
            }

            if (r1[i1] != '.') 
            {
                assert(r1[i1] >= '0' &&
                       r1[i1] <= '9');
                l1 = l1 * 10 + (r1[i1]-'0');
                ++i1;
            }
        }
        if (i1 == r1.length() && i2 == r2.length())
        {
            return l1 < l2;
        }
        if (i1 == r1.length())
        {
            return true;
        }
        if (i2 == r2.length())
        {
            return false;
        }
        return false;
    }
};

#endif
