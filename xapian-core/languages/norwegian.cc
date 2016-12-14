/* This file was generated automatically by the Snowball to ISO C++ compiler */
/* http://snowballstem.org/ */

#include <config.h>
#include <limits.h>
#include "norwegian.h"

#define s_0_0 s_0_3
#define s_0_1 (s_0_2 + 2)
#define s_0_2 s_0_17
static const symbol s_pool[] = {
#define s_0_3 0
'a', 'n', 'd', 'e',
#define s_0_4 s_0_18
#define s_0_5 4
'a', 'n', 'e',
#define s_0_6 (s_0_7 + 3)
#define s_0_7 s_0_20
#define s_0_8 7
'e', 'r', 't', 'e',
#define s_0_9 s_0_4
#define s_0_10 s_0_7
#define s_0_11 11
'a', 'r',
#define s_0_12 s_0_8
#define s_0_13 13
'h', 'e', 't', 'e', 'r',
#define s_0_14 (s_0_15 + 1)
#define s_0_15 s_0_28
#define s_0_16 (s_0_17 + 2)
#define s_0_17 18
'e', 'd', 'e', 's',
#define s_0_18 22
'e', 'n', 'd', 'e', 's',
#define s_0_19 (s_0_20 + 3)
#define s_0_20 27
'h', 'e', 't', 'e', 'n', 'e', 's',
#define s_0_21 (s_0_22 + 3)
#define s_0_22 34
'h', 'e', 't', 'e', 'n', 's',
#define s_0_23 40
'e', 'r', 's',
#define s_0_24 43
'e', 't', 's',
#define s_0_25 (s_0_7 + 1)
#define s_0_26 s_0_7
#define s_0_27 s_0_8
#define s_0_28 46
'a', 's', 't',
#define s_1_0 49
'd', 't',
#define s_1_1 51
'v', 't',
#define s_2_0 (s_2_1 + 1)
#define s_2_1 53
'e', 'l', 'e', 'g',
#define s_2_2 (s_2_3 + 1)
#define s_2_3 57
'e', 'i', 'g',
#define s_2_4 (s_2_5 + 1)
#define s_2_5 60
'e', 'l', 'i', 'g',
#define s_2_6 64
'e', 'l', 's',
#define s_2_7 (s_2_8 + 1)
#define s_2_8 67
'e', 'l', 'o', 'v',
#define s_2_9 (s_2_10 + 3)
#define s_2_10 71
'h', 'e', 't', 's', 'l', 'o', 'v',
};


static const struct among a_0[29] =
{
/*  0 */ { 1, s_0_0, -1, 1},
/*  1 */ { 1, s_0_1, -1, 1},
/*  2 */ { 3, s_0_2, 1, 1},
/*  3 */ { 4, s_0_3, 1, 1},
/*  4 */ { 4, s_0_4, 1, 1},
/*  5 */ { 3, s_0_5, 1, 1},
/*  6 */ { 3, s_0_6, 1, 1},
/*  7 */ { 6, s_0_7, 6, 1},
/*  8 */ { 4, s_0_8, 1, 3},
/*  9 */ { 2, s_0_9, -1, 1},
/* 10 */ { 5, s_0_10, 9, 1},
/* 11 */ { 2, s_0_11, -1, 1},
/* 12 */ { 2, s_0_12, -1, 1},
/* 13 */ { 5, s_0_13, 12, 1},
/* 14 */ { 1, s_0_14, -1, 2},
/* 15 */ { 2, s_0_15, 14, 1},
/* 16 */ { 2, s_0_16, 14, 1},
/* 17 */ { 4, s_0_17, 16, 1},
/* 18 */ { 5, s_0_18, 16, 1},
/* 19 */ { 4, s_0_19, 16, 1},
/* 20 */ { 7, s_0_20, 19, 1},
/* 21 */ { 3, s_0_21, 14, 1},
/* 22 */ { 6, s_0_22, 21, 1},
/* 23 */ { 3, s_0_23, 14, 1},
/* 24 */ { 3, s_0_24, 14, 1},
/* 25 */ { 2, s_0_25, -1, 1},
/* 26 */ { 3, s_0_26, 25, 1},
/* 27 */ { 3, s_0_27, -1, 3},
/* 28 */ { 3, s_0_28, -1, 1}
};


static const struct among a_1[2] =
{
/*  0 */ { 2, s_1_0, -1, -1},
/*  1 */ { 2, s_1_1, -1, -1}
};


static const struct among a_2[11] =
{
/*  0 */ { 3, s_2_0, -1, 1},
/*  1 */ { 4, s_2_1, 0, 1},
/*  2 */ { 2, s_2_2, -1, 1},
/*  3 */ { 3, s_2_3, 2, 1},
/*  4 */ { 3, s_2_4, 2, 1},
/*  5 */ { 4, s_2_5, 4, 1},
/*  6 */ { 3, s_2_6, -1, 1},
/*  7 */ { 3, s_2_7, -1, 1},
/*  8 */ { 4, s_2_8, 7, 1},
/*  9 */ { 4, s_2_9, 7, 1},
/* 10 */ { 7, s_2_10, 9, 1}
};

static const unsigned char g_v[] = { 17, 65, 16, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 48, 0, 128 };

static const unsigned char g_s_ending[] = { 119, 125, 149, 1 };

static const symbol s_0[] = { 'e', 'r' };

int Xapian::InternalStemNorwegian::r_mark_regions() { /* forwardmode */
    I_p1 = l; /* $p1 = <integer expression>, line 32 */
    {   int c_test1 = c; /* test, line 34 */
        {   int ret = skip_utf8(p, c, 0, l, + 3); /* hop, line 34 */
            if (ret < 0) return 0;
            c = ret;
        }
        I_x = c; /* setmark x, line 34 */
        c = c_test1;
    }
    if (out_grouping_U(g_v, 97, 248, 1) < 0) return 0; /* goto */ /* grouping v, line 35 */
    {    /* gopast */ /* non v, line 35 */
        int ret = in_grouping_U(g_v, 97, 248, 1);
        if (ret < 0) return 0;
        c += ret;
    }
    I_p1 = c; /* setmark p1, line 35 */
    /* try, line 36 */
    if (!(I_p1 < I_x)) goto lab0; /* $p1 < <integer expression>, line 36 */
    I_p1 = I_x; /* $p1 = <integer expression>, line 36 */
lab0:
    return 1;
}

int Xapian::InternalStemNorwegian::r_main_suffix() { /* backwardmode */
    int among_var;
    {   int m1 = l - c; /*(void)m1*/; /* setlimit, line 42 */
        int mlimit1;
        if (c < I_p1) return 0;
        c = I_p1; /* tomark, line 42 */
        mlimit1 = lb; lb = c;
        c = l - m1;
        ket = c; /* [, line 42 */
        if (c <= lb || p[c - 1] >> 5 != 3 || !((1851426 >> (p[c - 1] & 0x1f)) & 1)) { lb = mlimit1; return 0; } /* substring, line 42 */
        among_var = find_among_b(s_pool, a_0, 29, 0, 0);
        if (!(among_var)) { lb = mlimit1; return 0; }
        bra = c; /* ], line 42 */
        lb = mlimit1;
    }
    switch (among_var) { /* among, line 43 */
        case 0: return 0;
        case 1:
            if (slice_del() == -1) return -1; /* delete, line 48 */
            break;
        case 2:
            {   int m2 = l - c; /*(void)m2*/; /* or, line 50 */
                if (in_grouping_b_U(g_s_ending, 98, 122, 0)) goto lab1; /* grouping s_ending, line 50 */
                goto lab0;
            lab1:
                c = l - m2;
                if (c <= lb || p[c - 1] != 'k') return 0; /* literal, line 50 */
                c--;
                if (out_grouping_b_U(g_v, 97, 248, 0)) return 0; /* non v, line 50 */
            }
        lab0:
            if (slice_del() == -1) return -1; /* delete, line 50 */
            break;
        case 3:
            {   int ret = slice_from_s(2, s_0); /* <-, line 52 */
                if (ret < 0) return ret;
            }
            break;
    }
    return 1;
}

int Xapian::InternalStemNorwegian::r_consonant_pair() { /* backwardmode */
    {   int m_test1 = l - c; /* test, line 57 */
        {   int m2 = l - c; /*(void)m2*/; /* setlimit, line 58 */
            int mlimit2;
            if (c < I_p1) return 0;
            c = I_p1; /* tomark, line 58 */
            mlimit2 = lb; lb = c;
            c = l - m2;
            ket = c; /* [, line 58 */
            if (c - 1 <= lb || p[c - 1] != 116) { lb = mlimit2; return 0; } /* substring, line 58 */
            if (!(find_among_b(s_pool, a_1, 2, 0, 0))) { lb = mlimit2; return 0; }
            bra = c; /* ], line 58 */
            lb = mlimit2;
        }
        c = l - m_test1;
    }
    {   int ret = skip_utf8(p, c, lb, 0, -1);
        if (ret < 0) return 0;
        c = ret; /* next, line 63 */
    }
    bra = c; /* ], line 63 */
    if (slice_del() == -1) return -1; /* delete, line 63 */
    return 1;
}

int Xapian::InternalStemNorwegian::r_other_suffix() { /* backwardmode */
    int among_var;
    {   int m1 = l - c; /*(void)m1*/; /* setlimit, line 67 */
        int mlimit1;
        if (c < I_p1) return 0;
        c = I_p1; /* tomark, line 67 */
        mlimit1 = lb; lb = c;
        c = l - m1;
        ket = c; /* [, line 67 */
        if (c - 1 <= lb || p[c - 1] >> 5 != 3 || !((4718720 >> (p[c - 1] & 0x1f)) & 1)) { lb = mlimit1; return 0; } /* substring, line 67 */
        among_var = find_among_b(s_pool, a_2, 11, 0, 0);
        if (!(among_var)) { lb = mlimit1; return 0; }
        bra = c; /* ], line 67 */
        lb = mlimit1;
    }
    switch (among_var) { /* among, line 68 */
        case 0: return 0;
        case 1:
            if (slice_del() == -1) return -1; /* delete, line 71 */
            break;
    }
    return 1;
}

int Xapian::InternalStemNorwegian::stem() { /* forwardmode */
    {   int c1 = c; /* do, line 78 */
        {   int ret = r_mark_regions(); /* call mark_regions, line 78 */
            if (ret == 0) goto lab0;
            if (ret < 0) return ret;
        }
    lab0:
        c = c1;
    }
    lb = c; c = l; /* backwards, line 79 */

    {   int m2 = l - c; /*(void)m2*/; /* do, line 80 */
        {   int ret = r_main_suffix(); /* call main_suffix, line 80 */
            if (ret == 0) goto lab1;
            if (ret < 0) return ret;
        }
    lab1:
        c = l - m2;
    }
    {   int m3 = l - c; /*(void)m3*/; /* do, line 81 */
        {   int ret = r_consonant_pair(); /* call consonant_pair, line 81 */
            if (ret == 0) goto lab2;
            if (ret < 0) return ret;
        }
    lab2:
        c = l - m3;
    }
    {   int m4 = l - c; /*(void)m4*/; /* do, line 82 */
        {   int ret = r_other_suffix(); /* call other_suffix, line 82 */
            if (ret == 0) goto lab3;
            if (ret < 0) return ret;
        }
    lab3:
        c = l - m4;
    }
    c = lb;
    return 1;
}

Xapian::InternalStemNorwegian::InternalStemNorwegian()
    : I_x(0), I_p1(0)
{
}

Xapian::InternalStemNorwegian::~InternalStemNorwegian()
{
}

std::string
Xapian::InternalStemNorwegian::get_description() const
{
    return "norwegian";
}
