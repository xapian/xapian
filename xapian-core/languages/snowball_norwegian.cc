
/* This file was generated automatically by the Snowball to ANSI C compiler */

#include "header.h"

extern int snowball_norwegian_stem(struct SN_env * z);
static int r_other_suffix(struct SN_env * z);
static int r_consonant_pair(struct SN_env * z);
static int r_main_suffix(struct SN_env * z);
static int r_mark_regions(struct SN_env * z);

static symbol s_0_0[1] = { 'a' };
static symbol s_0_1[1] = { 'e' };
static symbol s_0_2[3] = { 'e', 'd', 'e' };
static symbol s_0_3[4] = { 'a', 'n', 'd', 'e' };
static symbol s_0_4[4] = { 'e', 'n', 'd', 'e' };
static symbol s_0_5[3] = { 'a', 'n', 'e' };
static symbol s_0_6[3] = { 'e', 'n', 'e' };
static symbol s_0_7[6] = { 'h', 'e', 't', 'e', 'n', 'e' };
static symbol s_0_8[4] = { 'e', 'r', 't', 'e' };
static symbol s_0_9[2] = { 'e', 'n' };
static symbol s_0_10[5] = { 'h', 'e', 't', 'e', 'n' };
static symbol s_0_11[2] = { 'a', 'r' };
static symbol s_0_12[2] = { 'e', 'r' };
static symbol s_0_13[5] = { 'h', 'e', 't', 'e', 'r' };
static symbol s_0_14[1] = { 's' };
static symbol s_0_15[2] = { 'a', 's' };
static symbol s_0_16[2] = { 'e', 's' };
static symbol s_0_17[4] = { 'e', 'd', 'e', 's' };
static symbol s_0_18[5] = { 'e', 'n', 'd', 'e', 's' };
static symbol s_0_19[4] = { 'e', 'n', 'e', 's' };
static symbol s_0_20[7] = { 'h', 'e', 't', 'e', 'n', 'e', 's' };
static symbol s_0_21[3] = { 'e', 'n', 's' };
static symbol s_0_22[6] = { 'h', 'e', 't', 'e', 'n', 's' };
static symbol s_0_23[3] = { 'e', 'r', 's' };
static symbol s_0_24[3] = { 'e', 't', 's' };
static symbol s_0_25[2] = { 'e', 't' };
static symbol s_0_26[3] = { 'h', 'e', 't' };
static symbol s_0_27[3] = { 'e', 'r', 't' };
static symbol s_0_28[3] = { 'a', 's', 't' };

static struct among a_0[29] =
{
/*  0 */ { 1, s_0_0, -1, 1, 0},
/*  1 */ { 1, s_0_1, -1, 1, 0},
/*  2 */ { 3, s_0_2, 1, 1, 0},
/*  3 */ { 4, s_0_3, 1, 1, 0},
/*  4 */ { 4, s_0_4, 1, 1, 0},
/*  5 */ { 3, s_0_5, 1, 1, 0},
/*  6 */ { 3, s_0_6, 1, 1, 0},
/*  7 */ { 6, s_0_7, 6, 1, 0},
/*  8 */ { 4, s_0_8, 1, 3, 0},
/*  9 */ { 2, s_0_9, -1, 1, 0},
/* 10 */ { 5, s_0_10, 9, 1, 0},
/* 11 */ { 2, s_0_11, -1, 1, 0},
/* 12 */ { 2, s_0_12, -1, 1, 0},
/* 13 */ { 5, s_0_13, 12, 1, 0},
/* 14 */ { 1, s_0_14, -1, 2, 0},
/* 15 */ { 2, s_0_15, 14, 1, 0},
/* 16 */ { 2, s_0_16, 14, 1, 0},
/* 17 */ { 4, s_0_17, 16, 1, 0},
/* 18 */ { 5, s_0_18, 16, 1, 0},
/* 19 */ { 4, s_0_19, 16, 1, 0},
/* 20 */ { 7, s_0_20, 19, 1, 0},
/* 21 */ { 3, s_0_21, 14, 1, 0},
/* 22 */ { 6, s_0_22, 21, 1, 0},
/* 23 */ { 3, s_0_23, 14, 1, 0},
/* 24 */ { 3, s_0_24, 14, 1, 0},
/* 25 */ { 2, s_0_25, -1, 1, 0},
/* 26 */ { 3, s_0_26, 25, 1, 0},
/* 27 */ { 3, s_0_27, -1, 3, 0},
/* 28 */ { 3, s_0_28, -1, 1, 0}
};

static symbol s_1_0[2] = { 'd', 't' };
static symbol s_1_1[2] = { 'v', 't' };

static struct among a_1[2] =
{
/*  0 */ { 2, s_1_0, -1, -1, 0},
/*  1 */ { 2, s_1_1, -1, -1, 0}
};

static symbol s_2_0[3] = { 'l', 'e', 'g' };
static symbol s_2_1[4] = { 'e', 'l', 'e', 'g' };
static symbol s_2_2[2] = { 'i', 'g' };
static symbol s_2_3[3] = { 'e', 'i', 'g' };
static symbol s_2_4[3] = { 'l', 'i', 'g' };
static symbol s_2_5[4] = { 'e', 'l', 'i', 'g' };
static symbol s_2_6[3] = { 'e', 'l', 's' };
static symbol s_2_7[3] = { 'l', 'o', 'v' };
static symbol s_2_8[4] = { 'e', 'l', 'o', 'v' };
static symbol s_2_9[4] = { 's', 'l', 'o', 'v' };
static symbol s_2_10[7] = { 'h', 'e', 't', 's', 'l', 'o', 'v' };

static struct among a_2[11] =
{
/*  0 */ { 3, s_2_0, -1, 1, 0},
/*  1 */ { 4, s_2_1, 0, 1, 0},
/*  2 */ { 2, s_2_2, -1, 1, 0},
/*  3 */ { 3, s_2_3, 2, 1, 0},
/*  4 */ { 3, s_2_4, 2, 1, 0},
/*  5 */ { 4, s_2_5, 4, 1, 0},
/*  6 */ { 3, s_2_6, -1, 1, 0},
/*  7 */ { 3, s_2_7, -1, 1, 0},
/*  8 */ { 4, s_2_8, 7, 1, 0},
/*  9 */ { 4, s_2_9, 7, 1, 0},
/* 10 */ { 7, s_2_10, 9, 1, 0}
};

static unsigned char g_v[] = { 17, 65, 16, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 48, 0, 128 };

static unsigned char g_s_ending[] = { 119, 127, 149, 1 };

static symbol s_0[] = { 'e', 'r' };

static int r_mark_regions(struct SN_env * z) {
    z->I[0] = z->l;
    while(1) { /* goto, line 30 */
        int c = z->c;
        if (!(in_grouping(z, g_v, 97, 248))) goto lab0;
        z->c = c;
        break;
    lab0:
        z->c = c;
        if (z->c >= z->l) return 0;
        z->c++;
    }
    while(1) { /* gopast, line 30 */
        if (!(out_grouping(z, g_v, 97, 248))) goto lab1;
        break;
    lab1:
        if (z->c >= z->l) return 0;
        z->c++;
    }
    z->I[0] = z->c; /* setmark p1, line 30 */
     /* try, line 31 */
    if (!(z->I[0] < 3)) goto lab2;
    z->I[0] = 3;
lab2:
    return 1;
}

static int r_main_suffix(struct SN_env * z) {
    int among_var;
    {   int m = z->l - z->c; /* setlimit, line 37 */
        int m3;
        if (z->c < z->I[0]) return 0;
        z->c = z->I[0]; /* tomark, line 37 */
        m3 = z->lb; z->lb = z->c;
        z->c = z->l - m;
        z->ket = z->c; /* [, line 37 */
        among_var = find_among_b(z, a_0, 29); /* substring, line 37 */
        if (!(among_var)) { z->lb = m3; return 0; }
        z->bra = z->c; /* ], line 37 */
        z->lb = m3;
    }
    switch(among_var) {
        case 0: return 0;
        case 1:
            slice_del(z); /* delete, line 43 */
            break;
        case 2:
            if (!(in_grouping_b(z, g_s_ending, 98, 122))) return 0;
            slice_del(z); /* delete, line 45 */
            break;
        case 3:
            slice_from_s(z, 2, s_0); /* <-, line 47 */
            break;
    }
    return 1;
}

static int r_consonant_pair(struct SN_env * z) {
    {   int m_test = z->l - z->c; /* test, line 52 */
        {   int m = z->l - z->c; /* setlimit, line 53 */
            int m3;
            if (z->c < z->I[0]) return 0;
            z->c = z->I[0]; /* tomark, line 53 */
            m3 = z->lb; z->lb = z->c;
            z->c = z->l - m;
            z->ket = z->c; /* [, line 53 */
            if (!(find_among_b(z, a_1, 2))) { z->lb = m3; return 0; } /* substring, line 53 */
            z->bra = z->c; /* ], line 53 */
            z->lb = m3;
        }
        z->c = z->l - m_test;
    }
    if (z->c <= z->lb) return 0;
    z->c--; /* next, line 58 */
    z->bra = z->c; /* ], line 58 */
    slice_del(z); /* delete, line 58 */
    return 1;
}

static int r_other_suffix(struct SN_env * z) {
    int among_var;
    {   int m = z->l - z->c; /* setlimit, line 62 */
        int m3;
        if (z->c < z->I[0]) return 0;
        z->c = z->I[0]; /* tomark, line 62 */
        m3 = z->lb; z->lb = z->c;
        z->c = z->l - m;
        z->ket = z->c; /* [, line 62 */
        among_var = find_among_b(z, a_2, 11); /* substring, line 62 */
        if (!(among_var)) { z->lb = m3; return 0; }
        z->bra = z->c; /* ], line 62 */
        z->lb = m3;
    }
    switch(among_var) {
        case 0: return 0;
        case 1:
            slice_del(z); /* delete, line 66 */
            break;
    }
    return 1;
}

extern int snowball_norwegian_stem(struct SN_env * z) {
    {   int c = z->c; /* do, line 73 */
        if (!r_mark_regions(z)) goto lab0; /* call mark_regions, line 73 */
    lab0:
        z->c = c;
    }
    z->lb = z->c; z->c = z->l; /* backwards, line 74 */

    {   int m = z->l - z->c; /* do, line 75 */
        if (!r_main_suffix(z)) goto lab1; /* call main_suffix, line 75 */
    lab1:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; /* do, line 76 */
        if (!r_consonant_pair(z)) goto lab2; /* call consonant_pair, line 76 */
    lab2:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; /* do, line 77 */
        if (!r_other_suffix(z)) goto lab3; /* call other_suffix, line 77 */
    lab3:
        z->c = z->l - m;
    }
    z->c = z->lb;
    return 1;
}

extern struct SN_env * snowball_norwegian_create_env(void) { return SN_create_env(0, 1, 0); }

extern void snowball_norwegian_close_env(struct SN_env * z) { SN_close_env(z); }

