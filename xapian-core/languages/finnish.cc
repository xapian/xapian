/* This file was generated automatically by the Snowball to ISO C++ compiler */
/* http://snowballstem.org/ */

#include <config.h>
#include <limits.h>
#include "finnish.h"

static int tr_VI(Xapian::StemImplementation * this_ptr) {
    return (static_cast<Xapian::InternalStemFinnish *>(this_ptr))->r_VI();
}

static int tr_LONG(Xapian::StemImplementation * this_ptr) {
    return (static_cast<Xapian::InternalStemFinnish *>(this_ptr))->r_LONG();
}

static const among_function af[2] =
{
/*  1 */ tr_VI,
/*  2 */ tr_LONG
};

static const symbol s_pool[] = {
#define s_0_0 0
'p', 'a',
#define s_0_1 2
's', 't', 'i',
#define s_0_2 5
'k', 'a', 'a', 'n',
#define s_0_3 9
'h', 'a', 'n',
#define s_0_4 12
'k', 'i', 'n',
#define s_0_5 15
'h', 0xC3, 0xA4, 'n',
#define s_0_6 19
'k', 0xC3, 0xA4, 0xC3, 0xA4, 'n',
#define s_0_7 25
'k', 'o',
#define s_0_8 27
'p', 0xC3, 0xA4,
#define s_0_9 30
'k', 0xC3, 0xB6,
#define s_1_0 33
'l', 'l', 'a',
#define s_1_1 36
'n', 'a',
#define s_1_2 38
's', 's', 'a',
#define s_1_3 (s_1_4 + 1)
#define s_1_4 41
'l', 't', 'a',
#define s_1_5 44
's', 't', 'a',
#define s_2_0 47
'l', 'l', 0xC3, 0xA4,
#define s_2_1 51
'n', 0xC3, 0xA4,
#define s_2_2 54
's', 's', 0xC3, 0xA4,
#define s_2_3 (s_2_4 + 1)
#define s_2_4 58
'l', 't', 0xC3, 0xA4,
#define s_2_5 62
's', 't', 0xC3, 0xA4,
#define s_3_0 66
'l', 'l', 'e',
#define s_3_1 69
'i', 'n', 'e',
#define s_4_0 72
'n', 's', 'a',
#define s_4_1 75
'm', 'm', 'e',
#define s_4_2 78
'n', 'n', 'e',
#define s_4_3 81
'n', 'i',
#define s_4_4 83
's', 'i',
#define s_4_5 85
'a', 'n',
#define s_4_6 87
'e', 'n',
#define s_4_7 89
0xC3, 0xA4, 'n',
#define s_4_8 92
'n', 's', 0xC3, 0xA4,
#define s_5_0 96
'a', 'a',
#define s_5_1 98
'e', 'e',
#define s_5_2 100
'i', 'i',
#define s_5_3 102
'o', 'o',
#define s_5_4 104
'u', 'u',
#define s_5_5 106
0xC3, 0xA4, 0xC3, 0xA4,
#define s_5_6 110
0xC3, 0xB6, 0xC3, 0xB6,
#define s_6_0 (s_6_1 + 2)
#define s_6_1 114
'l', 'l', 'a',
#define s_6_2 117
'n', 'a',
#define s_6_3 119
's', 's', 'a',
#define s_6_4 (s_6_5 + 1)
#define s_6_5 122
'l', 't', 'a',
#define s_6_6 125
's', 't', 'a',
#define s_6_7 128
't', 't', 'a',
#define s_6_8 131
'l', 'l', 'e',
#define s_6_9 134
'i', 'n', 'e',
#define s_6_10 137
'k', 's', 'i',
#define s_6_11 s_6_2
#define s_6_12 140
'h', 'a', 'n',
#define s_6_13 143
'd', 'e', 'n',
#define s_6_14 146
's', 'e', 'e', 'n',
#define s_6_15 150
'h', 'e', 'n',
#define s_6_16 153
't', 't', 'e', 'n',
#define s_6_17 157
'h', 'i', 'n',
#define s_6_18 160
's', 'i', 'i', 'n',
#define s_6_19 164
'h', 'o', 'n',
#define s_6_20 167
'h', 0xC3, 0xA4, 'n',
#define s_6_21 171
'h', 0xC3, 0xB6, 'n',
#define s_6_22 (s_6_20 + 1)
#define s_6_23 175
'l', 'l', 0xC3, 0xA4,
#define s_6_24 179
'n', 0xC3, 0xA4,
#define s_6_25 182
's', 's', 0xC3, 0xA4,
#define s_6_26 (s_6_27 + 1)
#define s_6_27 186
'l', 't', 0xC3, 0xA4,
#define s_6_28 190
's', 't', 0xC3, 0xA4,
#define s_6_29 194
't', 't', 0xC3, 0xA4,
#define s_7_0 198
'e', 'j', 'a',
#define s_7_1 (s_7_2 + 1)
#define s_7_2 201
'i', 'm', 'm', 'a',
#define s_7_3 (s_7_4 + 1)
#define s_7_4 205
'i', 'm', 'p', 'a',
#define s_7_5 (s_7_6 + 1)
#define s_7_6 209
'i', 'm', 'm', 'i',
#define s_7_7 (s_7_8 + 1)
#define s_7_8 213
'i', 'm', 'p', 'i',
#define s_7_9 217
'e', 'j', 0xC3, 0xA4,
#define s_7_10 (s_7_11 + 1)
#define s_7_11 221
'i', 'm', 'm', 0xC3, 0xA4,
#define s_7_12 (s_7_13 + 1)
#define s_7_13 226
'i', 'm', 'p', 0xC3, 0xA4,
#define s_8_0 231
'i',
#define s_8_1 232
'j',
#define s_9_0 (s_9_1 + 1)
#define s_9_1 233
'i', 'm', 'm', 'a',
};


static const struct among a_0[10] =
{
/*  0 */ { 2, s_0_0, -1, 1},
/*  1 */ { 3, s_0_1, -1, 2},
/*  2 */ { 4, s_0_2, -1, 1},
/*  3 */ { 3, s_0_3, -1, 1},
/*  4 */ { 3, s_0_4, -1, 1},
/*  5 */ { 4, s_0_5, -1, 1},
/*  6 */ { 6, s_0_6, -1, 1},
/*  7 */ { 2, s_0_7, -1, 1},
/*  8 */ { 3, s_0_8, -1, 1},
/*  9 */ { 3, s_0_9, -1, 1}
};


static const struct among a_1[6] =
{
/*  0 */ { 3, s_1_0, -1, -1},
/*  1 */ { 2, s_1_1, -1, -1},
/*  2 */ { 3, s_1_2, -1, -1},
/*  3 */ { 2, s_1_3, -1, -1},
/*  4 */ { 3, s_1_4, 3, -1},
/*  5 */ { 3, s_1_5, 3, -1}
};


static const struct among a_2[6] =
{
/*  0 */ { 4, s_2_0, -1, -1},
/*  1 */ { 3, s_2_1, -1, -1},
/*  2 */ { 4, s_2_2, -1, -1},
/*  3 */ { 3, s_2_3, -1, -1},
/*  4 */ { 4, s_2_4, 3, -1},
/*  5 */ { 4, s_2_5, 3, -1}
};


static const struct among a_3[2] =
{
/*  0 */ { 3, s_3_0, -1, -1},
/*  1 */ { 3, s_3_1, -1, -1}
};


static const struct among a_4[9] =
{
/*  0 */ { 3, s_4_0, -1, 3},
/*  1 */ { 3, s_4_1, -1, 3},
/*  2 */ { 3, s_4_2, -1, 3},
/*  3 */ { 2, s_4_3, -1, 2},
/*  4 */ { 2, s_4_4, -1, 1},
/*  5 */ { 2, s_4_5, -1, 4},
/*  6 */ { 2, s_4_6, -1, 6},
/*  7 */ { 3, s_4_7, -1, 5},
/*  8 */ { 4, s_4_8, -1, 3}
};


static const struct among a_5[7] =
{
/*  0 */ { 2, s_5_0, -1, -1},
/*  1 */ { 2, s_5_1, -1, -1},
/*  2 */ { 2, s_5_2, -1, -1},
/*  3 */ { 2, s_5_3, -1, -1},
/*  4 */ { 2, s_5_4, -1, -1},
/*  5 */ { 4, s_5_5, -1, -1},
/*  6 */ { 4, s_5_6, -1, -1}
};


static const struct among a_6[30] =
{
/*  0 */ { 1, s_6_0, -1, 8},
/*  1 */ { 3, s_6_1, 0, -1},
/*  2 */ { 2, s_6_2, 0, -1},
/*  3 */ { 3, s_6_3, 0, -1},
/*  4 */ { 2, s_6_4, 0, -1},
/*  5 */ { 3, s_6_5, 4, -1},
/*  6 */ { 3, s_6_6, 4, -1},
/*  7 */ { 3, s_6_7, 4, 9},
/*  8 */ { 3, s_6_8, -1, -1},
/*  9 */ { 3, s_6_9, -1, -1},
/* 10 */ { 3, s_6_10, -1, -1},
/* 11 */ { 1, s_6_11, -1, 7},
/* 12 */ { 3, s_6_12, 11, 1},
/* 13 */ { 3, s_6_13, 11, -1},
/* 14 */ { 4, s_6_14, 11, -1},
/* 15 */ { 3, s_6_15, 11, 2},
/* 16 */ { 4, s_6_16, 11, -1},
/* 17 */ { 3, s_6_17, 11, 3},
/* 18 */ { 4, s_6_18, 11, -1},
/* 19 */ { 3, s_6_19, 11, 4},
/* 20 */ { 4, s_6_20, 11, 5},
/* 21 */ { 4, s_6_21, 11, 6},
/* 22 */ { 2, s_6_22, -1, 8},
/* 23 */ { 4, s_6_23, 22, -1},
/* 24 */ { 3, s_6_24, 22, -1},
/* 25 */ { 4, s_6_25, 22, -1},
/* 26 */ { 3, s_6_26, 22, -1},
/* 27 */ { 4, s_6_27, 26, -1},
/* 28 */ { 4, s_6_28, 26, -1},
/* 29 */ { 4, s_6_29, 26, 9}
};

static const unsigned char af_6[30] =
{
/*  0 */ 0,
/*  1 */ 0,
/*  2 */ 0,
/*  3 */ 0,
/*  4 */ 0,
/*  5 */ 0,
/*  6 */ 0,
/*  7 */ 0,
/*  8 */ 0,
/*  9 */ 0,
/* 10 */ 0,
/* 11 */ 0,
/* 12 */ 0,
/* 13 */ 1 /* tr_VI */,
/* 14 */ 2 /* tr_LONG */,
/* 15 */ 0,
/* 16 */ 1 /* tr_VI */,
/* 17 */ 0,
/* 18 */ 1 /* tr_VI */,
/* 19 */ 0,
/* 20 */ 0,
/* 21 */ 0,
/* 22 */ 0,
/* 23 */ 0,
/* 24 */ 0,
/* 25 */ 0,
/* 26 */ 0,
/* 27 */ 0,
/* 28 */ 0,
/* 29 */ 0
};


static const struct among a_7[14] =
{
/*  0 */ { 3, s_7_0, -1, -1},
/*  1 */ { 3, s_7_1, -1, 1},
/*  2 */ { 4, s_7_2, 1, -1},
/*  3 */ { 3, s_7_3, -1, 1},
/*  4 */ { 4, s_7_4, 3, -1},
/*  5 */ { 3, s_7_5, -1, 1},
/*  6 */ { 4, s_7_6, 5, -1},
/*  7 */ { 3, s_7_7, -1, 1},
/*  8 */ { 4, s_7_8, 7, -1},
/*  9 */ { 4, s_7_9, -1, -1},
/* 10 */ { 4, s_7_10, -1, 1},
/* 11 */ { 5, s_7_11, 10, -1},
/* 12 */ { 4, s_7_12, -1, 1},
/* 13 */ { 5, s_7_13, 12, -1}
};


static const struct among a_8[2] =
{
/*  0 */ { 1, s_8_0, -1, -1},
/*  1 */ { 1, s_8_1, -1, -1}
};


static const struct among a_9[2] =
{
/*  0 */ { 3, s_9_0, -1, 1},
/*  1 */ { 4, s_9_1, 0, -1}
};

static const unsigned char g_AEI[] = { 17, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8 };

static const unsigned char g_V1[] = { 17, 65, 16, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 32 };

static const unsigned char g_V2[] = { 17, 65, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 32 };

static const unsigned char g_particle_end[] = { 17, 97, 24, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 32 };

static const symbol s_0[] = { 'k', 's', 'e' };
static const symbol s_1[] = { 'k', 's', 'i' };
static const symbol s_2[] = { 0xC3, 0xA4 };
static const symbol s_3[] = { 0xC3, 0xB6 };
static const symbol s_4[] = { 'i', 'e' };
static const symbol s_5[] = { 'p', 'o' };
static const symbol s_6[] = { 'p', 'o' };

int Xapian::InternalStemFinnish::r_mark_regions() { /* forwardmode */
    I_p1 = l; /* $p1 = <integer expression>, line 44 */
    I_p2 = l; /* $p2 = <integer expression>, line 45 */
    if (out_grouping_U(g_V1, 97, 246, 1) < 0) return 0; /* goto */ /* grouping V1, line 47 */
    {    /* gopast */ /* non V1, line 47 */
        int ret = in_grouping_U(g_V1, 97, 246, 1);
        if (ret < 0) return 0;
        c += ret;
    }
    I_p1 = c; /* setmark p1, line 47 */
    if (out_grouping_U(g_V1, 97, 246, 1) < 0) return 0; /* goto */ /* grouping V1, line 48 */
    {    /* gopast */ /* non V1, line 48 */
        int ret = in_grouping_U(g_V1, 97, 246, 1);
        if (ret < 0) return 0;
        c += ret;
    }
    I_p2 = c; /* setmark p2, line 48 */
    return 1;
}

int Xapian::InternalStemFinnish::r_R2() { /* backwardmode */
    if (!(I_p2 <= c)) return 0; /* $p2 <= <integer expression>, line 53 */
    return 1;
}

int Xapian::InternalStemFinnish::r_particle_etc() { /* backwardmode */
    int among_var;
    {   int m1 = l - c; /*(void)m1*/; /* setlimit, line 56 */
        int mlimit1;
        if (c < I_p1) return 0;
        c = I_p1; /* tomark, line 56 */
        mlimit1 = lb; lb = c;
        c = l - m1;
        ket = c; /* [, line 56 */
        among_var = find_among_b(s_pool, a_0, 10, 0, 0); /* substring, line 56 */
        if (!(among_var)) { lb = mlimit1; return 0; }
        bra = c; /* ], line 56 */
        lb = mlimit1;
    }
    switch (among_var) { /* among, line 57 */
        case 0: return 0;
        case 1:
            if (in_grouping_b_U(g_particle_end, 97, 246, 0)) return 0; /* grouping particle_end, line 63 */
            break;
        case 2:
            {   int ret = r_R2(); /* call R2, line 65 */
                if (ret <= 0) return ret;
            }
            break;
    }
    if (slice_del() == -1) return -1; /* delete, line 67 */
    return 1;
}

int Xapian::InternalStemFinnish::r_possessive() { /* backwardmode */
    int among_var;
    {   int m1 = l - c; /*(void)m1*/; /* setlimit, line 70 */
        int mlimit1;
        if (c < I_p1) return 0;
        c = I_p1; /* tomark, line 70 */
        mlimit1 = lb; lb = c;
        c = l - m1;
        ket = c; /* [, line 70 */
        among_var = find_among_b(s_pool, a_4, 9, 0, 0); /* substring, line 70 */
        if (!(among_var)) { lb = mlimit1; return 0; }
        bra = c; /* ], line 70 */
        lb = mlimit1;
    }
    switch (among_var) { /* among, line 71 */
        case 0: return 0;
        case 1:
            {   int m2 = l - c; /*(void)m2*/; /* not, line 73 */
                if (c <= lb || p[c - 1] != 'k') goto lab0; /* literal, line 73 */
                c--;
                return 0;
            lab0:
                c = l - m2;
            }
            if (slice_del() == -1) return -1; /* delete, line 73 */
            break;
        case 2:
            if (slice_del() == -1) return -1; /* delete, line 75 */
            ket = c; /* [, line 75 */
            if (!(eq_s_b(3, s_0))) return 0; /* literal, line 75 */
            bra = c; /* ], line 75 */
            {   int ret = slice_from_s(3, s_1); /* <-, line 75 */
                if (ret < 0) return ret;
            }
            break;
        case 3:
            if (slice_del() == -1) return -1; /* delete, line 79 */
            break;
        case 4:
            if (c - 1 <= lb || p[c - 1] != 97) return 0; /* among, line 82 */
            if (!(find_among_b(s_pool, a_1, 6, 0, 0))) return 0;
            if (slice_del() == -1) return -1; /* delete, line 82 */
            break;
        case 5:
            if (c - 2 <= lb || p[c - 1] != 164) return 0; /* among, line 84 */
            if (!(find_among_b(s_pool, a_2, 6, 0, 0))) return 0;
            if (slice_del() == -1) return -1; /* delete, line 85 */
            break;
        case 6:
            if (c - 2 <= lb || p[c - 1] != 101) return 0; /* among, line 87 */
            if (!(find_among_b(s_pool, a_3, 2, 0, 0))) return 0;
            if (slice_del() == -1) return -1; /* delete, line 87 */
            break;
    }
    return 1;
}

int Xapian::InternalStemFinnish::r_LONG() { /* backwardmode */
    if (!(find_among_b(s_pool, a_5, 7, 0, 0))) return 0; /* among, line 92 */
    return 1;
}

int Xapian::InternalStemFinnish::r_VI() { /* backwardmode */
    if (c <= lb || p[c - 1] != 'i') return 0; /* literal, line 94 */
    c--;
    if (in_grouping_b_U(g_V2, 97, 246, 0)) return 0; /* grouping V2, line 94 */
    return 1;
}

int Xapian::InternalStemFinnish::r_case_ending() { /* backwardmode */
    int among_var;
    {   int m1 = l - c; /*(void)m1*/; /* setlimit, line 97 */
        int mlimit1;
        if (c < I_p1) return 0;
        c = I_p1; /* tomark, line 97 */
        mlimit1 = lb; lb = c;
        c = l - m1;
        ket = c; /* [, line 97 */
        among_var = find_among_b(s_pool, a_6, 30, af_6, af); /* substring, line 97 */
        if (!(among_var)) { lb = mlimit1; return 0; }
        bra = c; /* ], line 97 */
        lb = mlimit1;
    }
    switch (among_var) { /* among, line 98 */
        case 0: return 0;
        case 1:
            if (c <= lb || p[c - 1] != 'a') return 0; /* literal, line 99 */
            c--;
            break;
        case 2:
            if (c <= lb || p[c - 1] != 'e') return 0; /* literal, line 100 */
            c--;
            break;
        case 3:
            if (c <= lb || p[c - 1] != 'i') return 0; /* literal, line 101 */
            c--;
            break;
        case 4:
            if (c <= lb || p[c - 1] != 'o') return 0; /* literal, line 102 */
            c--;
            break;
        case 5:
            if (!(eq_s_b(2, s_2))) return 0; /* literal, line 103 */
            break;
        case 6:
            if (!(eq_s_b(2, s_3))) return 0; /* literal, line 104 */
            break;
        case 7:
            {   int m2 = l - c; /*(void)m2*/; /* try, line 112 */
                {   int m3 = l - c; /*(void)m3*/; /* and, line 114 */
                    {   int m4 = l - c; /*(void)m4*/; /* or, line 113 */
                        {   int ret = r_LONG(); /* call LONG, line 112 */
                            if (ret == 0) goto lab2;
                            if (ret < 0) return ret;
                        }
                        goto lab1;
                    lab2:
                        c = l - m4;
                        if (!(eq_s_b(2, s_4))) { c = l - m2; goto lab0; } /* literal, line 113 */
                    }
                lab1:
                    c = l - m3;
                    {   int ret = skip_utf8(p, c, lb, 0, -1);
                        if (ret < 0) { c = l - m2; goto lab0; }
                        c = ret; /* next, line 114 */
                    }
                }
                bra = c; /* ], line 114 */
            lab0:
                ;
            }
            break;
        case 8:
            if (in_grouping_b_U(g_V1, 97, 246, 0)) return 0; /* grouping V1, line 120 */
            if (out_grouping_b_U(g_V1, 97, 246, 0)) return 0; /* non V1, line 120 */
            break;
        case 9:
            if (c <= lb || p[c - 1] != 'e') return 0; /* literal, line 122 */
            c--;
            break;
    }
    if (slice_del() == -1) return -1; /* delete, line 139 */
    B_ending_removed = 1; /* set ending_removed, line 140 */
    return 1;
}

int Xapian::InternalStemFinnish::r_other_endings() { /* backwardmode */
    int among_var;
    {   int m1 = l - c; /*(void)m1*/; /* setlimit, line 143 */
        int mlimit1;
        if (c < I_p2) return 0;
        c = I_p2; /* tomark, line 143 */
        mlimit1 = lb; lb = c;
        c = l - m1;
        ket = c; /* [, line 143 */
        among_var = find_among_b(s_pool, a_7, 14, 0, 0); /* substring, line 143 */
        if (!(among_var)) { lb = mlimit1; return 0; }
        bra = c; /* ], line 143 */
        lb = mlimit1;
    }
    switch (among_var) { /* among, line 144 */
        case 0: return 0;
        case 1:
            {   int m2 = l - c; /*(void)m2*/; /* not, line 147 */
                if (!(eq_s_b(2, s_5))) goto lab0; /* literal, line 147 */
                return 0;
            lab0:
                c = l - m2;
            }
            break;
    }
    if (slice_del() == -1) return -1; /* delete, line 152 */
    return 1;
}

int Xapian::InternalStemFinnish::r_i_plural() { /* backwardmode */
    {   int m1 = l - c; /*(void)m1*/; /* setlimit, line 155 */
        int mlimit1;
        if (c < I_p1) return 0;
        c = I_p1; /* tomark, line 155 */
        mlimit1 = lb; lb = c;
        c = l - m1;
        ket = c; /* [, line 155 */
        if (c <= lb || (p[c - 1] != 105 && p[c - 1] != 106)) { lb = mlimit1; return 0; } /* substring, line 155 */
        if (!(find_among_b(s_pool, a_8, 2, 0, 0))) { lb = mlimit1; return 0; }
        bra = c; /* ], line 155 */
        lb = mlimit1;
    }
    if (slice_del() == -1) return -1; /* delete, line 159 */
    return 1;
}

int Xapian::InternalStemFinnish::r_t_plural() { /* backwardmode */
    int among_var;
    {   int m1 = l - c; /*(void)m1*/; /* setlimit, line 162 */
        int mlimit1;
        if (c < I_p1) return 0;
        c = I_p1; /* tomark, line 162 */
        mlimit1 = lb; lb = c;
        c = l - m1;
        ket = c; /* [, line 163 */
        if (c <= lb || p[c - 1] != 't') { lb = mlimit1; return 0; } /* literal, line 163 */
        c--;
        bra = c; /* ], line 163 */
        {   int m_test2 = l - c; /* test, line 163 */
            if (in_grouping_b_U(g_V1, 97, 246, 0)) { lb = mlimit1; return 0; } /* grouping V1, line 163 */
            c = l - m_test2;
        }
        if (slice_del() == -1) return -1; /* delete, line 164 */
        lb = mlimit1;
    }
    {   int m3 = l - c; /*(void)m3*/; /* setlimit, line 166 */
        int mlimit3;
        if (c < I_p2) return 0;
        c = I_p2; /* tomark, line 166 */
        mlimit3 = lb; lb = c;
        c = l - m3;
        ket = c; /* [, line 166 */
        if (c - 2 <= lb || p[c - 1] != 97) { lb = mlimit3; return 0; } /* substring, line 166 */
        among_var = find_among_b(s_pool, a_9, 2, 0, 0);
        if (!(among_var)) { lb = mlimit3; return 0; }
        bra = c; /* ], line 166 */
        lb = mlimit3;
    }
    switch (among_var) { /* among, line 167 */
        case 0: return 0;
        case 1:
            {   int m4 = l - c; /*(void)m4*/; /* not, line 168 */
                if (!(eq_s_b(2, s_6))) goto lab0; /* literal, line 168 */
                return 0;
            lab0:
                c = l - m4;
            }
            break;
    }
    if (slice_del() == -1) return -1; /* delete, line 171 */
    return 1;
}

int Xapian::InternalStemFinnish::r_tidy() { /* backwardmode */
    {   int m1 = l - c; /*(void)m1*/; /* setlimit, line 174 */
        int mlimit1;
        if (c < I_p1) return 0;
        c = I_p1; /* tomark, line 174 */
        mlimit1 = lb; lb = c;
        c = l - m1;
        {   int m2 = l - c; /*(void)m2*/; /* do, line 175 */
            {   int m3 = l - c; /*(void)m3*/; /* and, line 175 */
                {   int ret = r_LONG(); /* call LONG, line 175 */
                    if (ret == 0) goto lab0;
                    if (ret < 0) return ret;
                }
                c = l - m3;
                ket = c; /* [, line 175 */
                {   int ret = skip_utf8(p, c, lb, 0, -1);
                    if (ret < 0) goto lab0;
                    c = ret; /* next, line 175 */
                }
                bra = c; /* ], line 175 */
                if (slice_del() == -1) return -1; /* delete, line 175 */
            }
        lab0:
            c = l - m2;
        }
        {   int m4 = l - c; /*(void)m4*/; /* do, line 176 */
            ket = c; /* [, line 176 */
            if (in_grouping_b_U(g_AEI, 97, 228, 0)) goto lab1; /* grouping AEI, line 176 */
            bra = c; /* ], line 176 */
            if (out_grouping_b_U(g_V1, 97, 246, 0)) goto lab1; /* non V1, line 176 */
            if (slice_del() == -1) return -1; /* delete, line 176 */
        lab1:
            c = l - m4;
        }
        {   int m5 = l - c; /*(void)m5*/; /* do, line 177 */
            ket = c; /* [, line 177 */
            if (c <= lb || p[c - 1] != 'j') goto lab2; /* literal, line 177 */
            c--;
            bra = c; /* ], line 177 */
            {   int m6 = l - c; /*(void)m6*/; /* or, line 177 */
                if (c <= lb || p[c - 1] != 'o') goto lab4; /* literal, line 177 */
                c--;
                goto lab3;
            lab4:
                c = l - m6;
                if (c <= lb || p[c - 1] != 'u') goto lab2; /* literal, line 177 */
                c--;
            }
        lab3:
            if (slice_del() == -1) return -1; /* delete, line 177 */
        lab2:
            c = l - m5;
        }
        {   int m7 = l - c; /*(void)m7*/; /* do, line 178 */
            ket = c; /* [, line 178 */
            if (c <= lb || p[c - 1] != 'o') goto lab5; /* literal, line 178 */
            c--;
            bra = c; /* ], line 178 */
            if (c <= lb || p[c - 1] != 'j') goto lab5; /* literal, line 178 */
            c--;
            if (slice_del() == -1) return -1; /* delete, line 178 */
        lab5:
            c = l - m7;
        }
        lb = mlimit1;
    }
    if (in_grouping_b_U(g_V1, 97, 246, 1) < 0) return 0; /* goto */ /* non V1, line 180 */
    ket = c; /* [, line 180 */
    {   int ret = skip_utf8(p, c, lb, 0, -1);
        if (ret < 0) return 0;
        c = ret; /* next, line 180 */
    }
    bra = c; /* ], line 180 */
    {   symbol * ret = slice_to(S_x); /* -> x, line 180 */
        if (ret == 0) return -1;
        S_x = ret;
    }
    if (!(eq_v_b(S_x))) return 0; /* name x, line 180 */
    if (slice_del() == -1) return -1; /* delete, line 180 */
    return 1;
}

int Xapian::InternalStemFinnish::stem() { /* forwardmode */
    {   int c1 = c; /* do, line 186 */
        {   int ret = r_mark_regions(); /* call mark_regions, line 186 */
            if (ret == 0) goto lab0;
            if (ret < 0) return ret;
        }
    lab0:
        c = c1;
    }
    B_ending_removed = 0; /* unset ending_removed, line 187 */
    lb = c; c = l; /* backwards, line 188 */

    {   int m2 = l - c; /*(void)m2*/; /* do, line 189 */
        {   int ret = r_particle_etc(); /* call particle_etc, line 189 */
            if (ret == 0) goto lab1;
            if (ret < 0) return ret;
        }
    lab1:
        c = l - m2;
    }
    {   int m3 = l - c; /*(void)m3*/; /* do, line 190 */
        {   int ret = r_possessive(); /* call possessive, line 190 */
            if (ret == 0) goto lab2;
            if (ret < 0) return ret;
        }
    lab2:
        c = l - m3;
    }
    {   int m4 = l - c; /*(void)m4*/; /* do, line 191 */
        {   int ret = r_case_ending(); /* call case_ending, line 191 */
            if (ret == 0) goto lab3;
            if (ret < 0) return ret;
        }
    lab3:
        c = l - m4;
    }
    {   int m5 = l - c; /*(void)m5*/; /* do, line 192 */
        {   int ret = r_other_endings(); /* call other_endings, line 192 */
            if (ret == 0) goto lab4;
            if (ret < 0) return ret;
        }
    lab4:
        c = l - m5;
    }
    {   int m6 = l - c; /*(void)m6*/; /* or, line 193 */
        if (!(B_ending_removed)) goto lab6; /* Boolean test ending_removed, line 193 */
        {   int m7 = l - c; /*(void)m7*/; /* do, line 193 */
            {   int ret = r_i_plural(); /* call i_plural, line 193 */
                if (ret == 0) goto lab7;
                if (ret < 0) return ret;
            }
        lab7:
            c = l - m7;
        }
        goto lab5;
    lab6:
        c = l - m6;
        {   int m8 = l - c; /*(void)m8*/; /* do, line 193 */
            {   int ret = r_t_plural(); /* call t_plural, line 193 */
                if (ret == 0) goto lab8;
                if (ret < 0) return ret;
            }
        lab8:
            c = l - m8;
        }
    }
lab5:
    {   int m9 = l - c; /*(void)m9*/; /* do, line 194 */
        {   int ret = r_tidy(); /* call tidy, line 194 */
            if (ret == 0) goto lab9;
            if (ret < 0) return ret;
        }
    lab9:
        c = l - m9;
    }
    c = lb;
    return 1;
}

Xapian::InternalStemFinnish::InternalStemFinnish()
    : B_ending_removed(0), S_x(0), I_p2(0), I_p1(0)
{
    S_x = create_s();
}

Xapian::InternalStemFinnish::~InternalStemFinnish()
{
    lose_s(S_x);
}

std::string
Xapian::InternalStemFinnish::get_description() const
{
    return "finnish";
}
