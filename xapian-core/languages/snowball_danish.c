
/* This file was generated automatically by the Snowball to ANSI C compiler */

#include "header.h"

extern int snowball_danish_stem(struct SN_env * z);
static int r_undouble(struct SN_env * z);
static int r_other_suffix(struct SN_env * z);
static int r_consonant_pair(struct SN_env * z);
static int r_main_suffix(struct SN_env * z);
static int r_mark_regions(struct SN_env * z);

static symbol s_0_0[3] = { 'h', 'e', 'd' };
static symbol s_0_1[5] = { 'e', 't', 'h', 'e', 'd' };
static symbol s_0_2[4] = { 'e', 'r', 'e', 'd' };
static symbol s_0_3[1] = { 'e' };
static symbol s_0_4[5] = { 'e', 'r', 'e', 'd', 'e' };
static symbol s_0_5[4] = { 'e', 'n', 'd', 'e' };
static symbol s_0_6[6] = { 'e', 'r', 'e', 'n', 'd', 'e' };
static symbol s_0_7[3] = { 'e', 'n', 'e' };
static symbol s_0_8[4] = { 'e', 'r', 'n', 'e' };
static symbol s_0_9[3] = { 'e', 'r', 'e' };
static symbol s_0_10[2] = { 'e', 'n' };
static symbol s_0_11[5] = { 'h', 'e', 'd', 'e', 'n' };
static symbol s_0_12[4] = { 'e', 'r', 'e', 'n' };
static symbol s_0_13[2] = { 'e', 'r' };
static symbol s_0_14[5] = { 'h', 'e', 'd', 'e', 'r' };
static symbol s_0_15[4] = { 'e', 'r', 'e', 'r' };
static symbol s_0_16[1] = { 's' };
static symbol s_0_17[4] = { 'h', 'e', 'd', 's' };
static symbol s_0_18[2] = { 'e', 's' };
static symbol s_0_19[5] = { 'e', 'n', 'd', 'e', 's' };
static symbol s_0_20[7] = { 'e', 'r', 'e', 'n', 'd', 'e', 's' };
static symbol s_0_21[4] = { 'e', 'n', 'e', 's' };
static symbol s_0_22[5] = { 'e', 'r', 'n', 'e', 's' };
static symbol s_0_23[4] = { 'e', 'r', 'e', 's' };
static symbol s_0_24[3] = { 'e', 'n', 's' };
static symbol s_0_25[6] = { 'h', 'e', 'd', 'e', 'n', 's' };
static symbol s_0_26[5] = { 'e', 'r', 'e', 'n', 's' };
static symbol s_0_27[3] = { 'e', 'r', 's' };
static symbol s_0_28[3] = { 'e', 't', 's' };
static symbol s_0_29[5] = { 'e', 'r', 'e', 't', 's' };
static symbol s_0_30[2] = { 'e', 't' };
static symbol s_0_31[4] = { 'e', 'r', 'e', 't' };

static struct among a_0[32] =
{
/*  0 */ { 3, s_0_0, -1, 1, 0},
/*  1 */ { 5, s_0_1, 0, 1, 0},
/*  2 */ { 4, s_0_2, -1, 1, 0},
/*  3 */ { 1, s_0_3, -1, 1, 0},
/*  4 */ { 5, s_0_4, 3, 1, 0},
/*  5 */ { 4, s_0_5, 3, 1, 0},
/*  6 */ { 6, s_0_6, 5, 1, 0},
/*  7 */ { 3, s_0_7, 3, 1, 0},
/*  8 */ { 4, s_0_8, 3, 1, 0},
/*  9 */ { 3, s_0_9, 3, 1, 0},
/* 10 */ { 2, s_0_10, -1, 1, 0},
/* 11 */ { 5, s_0_11, 10, 1, 0},
/* 12 */ { 4, s_0_12, 10, 1, 0},
/* 13 */ { 2, s_0_13, -1, 1, 0},
/* 14 */ { 5, s_0_14, 13, 1, 0},
/* 15 */ { 4, s_0_15, 13, 1, 0},
/* 16 */ { 1, s_0_16, -1, 2, 0},
/* 17 */ { 4, s_0_17, 16, 1, 0},
/* 18 */ { 2, s_0_18, 16, 1, 0},
/* 19 */ { 5, s_0_19, 18, 1, 0},
/* 20 */ { 7, s_0_20, 19, 1, 0},
/* 21 */ { 4, s_0_21, 18, 1, 0},
/* 22 */ { 5, s_0_22, 18, 1, 0},
/* 23 */ { 4, s_0_23, 18, 1, 0},
/* 24 */ { 3, s_0_24, 16, 1, 0},
/* 25 */ { 6, s_0_25, 24, 1, 0},
/* 26 */ { 5, s_0_26, 24, 1, 0},
/* 27 */ { 3, s_0_27, 16, 1, 0},
/* 28 */ { 3, s_0_28, 16, 1, 0},
/* 29 */ { 5, s_0_29, 28, 1, 0},
/* 30 */ { 2, s_0_30, -1, 1, 0},
/* 31 */ { 4, s_0_31, 30, 1, 0}
};

static symbol s_1_0[2] = { 'g', 'd' };
static symbol s_1_1[2] = { 'd', 't' };
static symbol s_1_2[2] = { 'g', 't' };
static symbol s_1_3[2] = { 'k', 't' };

static struct among a_1[4] =
{
/*  0 */ { 2, s_1_0, -1, -1, 0},
/*  1 */ { 2, s_1_1, -1, -1, 0},
/*  2 */ { 2, s_1_2, -1, -1, 0},
/*  3 */ { 2, s_1_3, -1, -1, 0}
};

static symbol s_2_0[2] = { 'i', 'g' };
static symbol s_2_1[3] = { 'l', 'i', 'g' };
static symbol s_2_2[4] = { 'e', 'l', 'i', 'g' };
static symbol s_2_3[3] = { 'e', 'l', 's' };
static symbol s_2_4[4] = { 'l', 248, 's', 't' };

static struct among a_2[5] =
{
/*  0 */ { 2, s_2_0, -1, 1, 0},
/*  1 */ { 3, s_2_1, 0, 1, 0},
/*  2 */ { 4, s_2_2, 1, 1, 0},
/*  3 */ { 3, s_2_3, -1, 1, 0},
/*  4 */ { 4, s_2_4, -1, 2, 0}
};

static unsigned char g_v[] = { 17, 65, 16, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 48, 0, 128 };

static unsigned char g_s_ending[] = { 239, 254, 42, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16 };

static symbol s_0[] = { 's', 't' };
static symbol s_1[] = { 'i', 'g' };
static symbol s_2[] = { 'l', 248, 's' };

static int r_mark_regions(struct SN_env * z) {
    z->I[0] = z->l;
    while(1) { /* goto, line 33 */
        int c = z->c;
        if (!(in_grouping(z, g_v, 97, 248))) goto lab0;
        z->c = c;
        break;
    lab0:
        z->c = c;
        if (z->c >= z->l) return 0;
        z->c++;
    }
    while(1) { /* gopast, line 33 */
        if (!(out_grouping(z, g_v, 97, 248))) goto lab1;
        break;
    lab1:
        if (z->c >= z->l) return 0;
        z->c++;
    }
    z->I[0] = z->c; /* setmark p1, line 33 */
     /* try, line 34 */
    if (!(z->I[0] < 3)) goto lab2;
    z->I[0] = 3;
lab2:
    return 1;
}

static int r_main_suffix(struct SN_env * z) {
    int among_var;
    {   int m = z->l - z->c; /* setlimit, line 40 */
        int m3;
        if (z->c < z->I[0]) return 0;
        z->c = z->I[0]; /* tomark, line 40 */
        m3 = z->lb; z->lb = z->c;
        z->c = z->l - m;
        z->ket = z->c; /* [, line 40 */
        among_var = find_among_b(z, a_0, 32); /* substring, line 40 */
        if (!(among_var)) { z->lb = m3; return 0; }
        z->bra = z->c; /* ], line 40 */
        z->lb = m3;
    }
    switch(among_var) {
        case 0: return 0;
        case 1:
            slice_del(z); /* delete, line 47 */
            break;
        case 2:
            if (!(in_grouping_b(z, g_s_ending, 97, 229))) return 0;
            slice_del(z); /* delete, line 49 */
            break;
    }
    return 1;
}

static int r_consonant_pair(struct SN_env * z) {
    {   int m_test = z->l - z->c; /* test, line 54 */
        {   int m = z->l - z->c; /* setlimit, line 55 */
            int m3;
            if (z->c < z->I[0]) return 0;
            z->c = z->I[0]; /* tomark, line 55 */
            m3 = z->lb; z->lb = z->c;
            z->c = z->l - m;
            z->ket = z->c; /* [, line 55 */
            if (!(find_among_b(z, a_1, 4))) { z->lb = m3; return 0; } /* substring, line 55 */
            z->bra = z->c; /* ], line 55 */
            z->lb = m3;
        }
        z->c = z->l - m_test;
    }
    if (z->c <= z->lb) return 0;
    z->c--; /* next, line 61 */
    z->bra = z->c; /* ], line 61 */
    slice_del(z); /* delete, line 61 */
    return 1;
}

static int r_other_suffix(struct SN_env * z) {
    int among_var;
    {   int m = z->l - z->c; /* do, line 65 */
        z->ket = z->c; /* [, line 65 */
        if (!(eq_s_b(z, 2, s_0))) goto lab0;
        z->bra = z->c; /* ], line 65 */
        if (!(eq_s_b(z, 2, s_1))) goto lab0;
        slice_del(z); /* delete, line 65 */
    lab0:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; /* setlimit, line 66 */
        int m3;
        if (z->c < z->I[0]) return 0;
        z->c = z->I[0]; /* tomark, line 66 */
        m3 = z->lb; z->lb = z->c;
        z->c = z->l - m;
        z->ket = z->c; /* [, line 66 */
        among_var = find_among_b(z, a_2, 5); /* substring, line 66 */
        if (!(among_var)) { z->lb = m3; return 0; }
        z->bra = z->c; /* ], line 66 */
        z->lb = m3;
    }
    switch(among_var) {
        case 0: return 0;
        case 1:
            slice_del(z); /* delete, line 69 */
            {   int m = z->l - z->c; /* do, line 69 */
                if (!r_consonant_pair(z)) goto lab1; /* call consonant_pair, line 69 */
            lab1:
                z->c = z->l - m;
            }
            break;
        case 2:
            slice_from_s(z, 3, s_2); /* <-, line 71 */
            break;
    }
    return 1;
}

static int r_undouble(struct SN_env * z) {
    {   int m = z->l - z->c; /* setlimit, line 75 */
        int m3;
        if (z->c < z->I[0]) return 0;
        z->c = z->I[0]; /* tomark, line 75 */
        m3 = z->lb; z->lb = z->c;
        z->c = z->l - m;
        z->ket = z->c; /* [, line 75 */
        if (!(out_grouping_b(z, g_v, 97, 248))) { z->lb = m3; return 0; }
        z->bra = z->c; /* ], line 75 */
        z->S[0] = slice_to(z, z->S[0]); /* -> ch, line 75 */
        z->lb = m3;
    }
    if (!(eq_v_b(z, z->S[0]))) return 0; /* name ch, line 76 */
    slice_del(z); /* delete, line 77 */
    return 1;
}

extern int snowball_danish_stem(struct SN_env * z) {
    {   int c = z->c; /* do, line 83 */
        if (!r_mark_regions(z)) goto lab0; /* call mark_regions, line 83 */
    lab0:
        z->c = c;
    }
    z->lb = z->c; z->c = z->l; /* backwards, line 84 */

    {   int m = z->l - z->c; /* do, line 85 */
        if (!r_main_suffix(z)) goto lab1; /* call main_suffix, line 85 */
    lab1:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; /* do, line 86 */
        if (!r_consonant_pair(z)) goto lab2; /* call consonant_pair, line 86 */
    lab2:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; /* do, line 87 */
        if (!r_other_suffix(z)) goto lab3; /* call other_suffix, line 87 */
    lab3:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; /* do, line 88 */
        if (!r_undouble(z)) goto lab4; /* call undouble, line 88 */
    lab4:
        z->c = z->l - m;
    }
    z->c = z->lb;
    return 1;
}

extern struct SN_env * snowball_danish_create_env(void) { return SN_create_env(1, 1, 0); }

extern void snowball_danish_close_env(struct SN_env * z) { SN_close_env(z); }

