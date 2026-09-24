#ifndef SNOWBALL_API_H_INCLUDED
#define SNOWBALL_API_H_INCLUDED

#ifdef SNOWBALL_WIDE
# ifndef __cplusplus
/* wchar_t is a built-in type in C++, but not in C. */
#  include <stddef.h>
# endif
typedef wchar_t symbol;
// Use a different typename - for C++ this then means that the runtime helper
// functions that depend on sizeof(symbol) will be distinct thanks to overloading
// on SN_env and/or symbol.
# define SN_env SN_env_w
#else
typedef unsigned char symbol;
#endif

struct SN_env {
    symbol * p;
    int c; int l; int lb; int bra; int ket;
    int af;
};

#ifdef __cplusplus
extern "C" {
#endif

extern struct SN_env * SN_new_env(int alloc_size);
extern struct SN_env * SN_new_env_no_vars(void);
extern void SN_delete_env(struct SN_env * z);

extern int SN_set_current(struct SN_env * z, int size, const symbol * s);

#ifdef __cplusplus
}
#endif

#endif
