
/* This file was generated automatically by the Snowball to ANSI C compiler */

#include "header.h"

extern int snowball_german_stem(struct SN_env * z);
static int r_standard_suffix(struct SN_env * z);
static int r_R2(struct SN_env * z);
static int r_R1(struct SN_env * z);
static int r_mark_regions(struct SN_env * z);
static int r_postlude(struct SN_env * z);
static int r_prelude(struct SN_env * z);

static symbol s_0_1[1] = { 'U' };
static symbol s_0_2[1] = { 'Y' };
static symbol s_0_3[1] = { 228 };
static symbol s_0_4[1] = { 246 };
static symbol s_0_5[1] = { 252 };

static struct among a_0[6] =
{
/*  0 */ { 0, 0, -1, 6, 0},
/*  1 */ { 1, s_0_1, 0, 2, 0},
/*  2 */ { 1, s_0_2, 0, 1, 0},
/*  3 */ { 1, s_0_3, 0, 3, 0},
/*  4 */ { 1, s_0_4, 0, 4, 0},
/*  5 */ { 1, s_0_5, 0, 5, 0}
};

static symbol s_1_0[1] = { 'e' };
static symbol s_1_1[2] = { 'e', 'm' };
static symbol s_1_2[2] = { 'e', 'n' };
static symbol s_1_3[3] = { 'e', 'r', 'n' };
static symbol s_1_4[2] = { 'e', 'r' };
static symbol s_1_5[1] = { 's' };
static symbol s_1_6[2] = { 'e', 's' };

static struct among a_1[7] =
{
/*  0 */ { 1, s_1_0, -1, 1, 0},
/*  1 */ { 2, s_1_1, -1, 1, 0},
/*  2 */ { 2, s_1_2, -1, 1, 0},
/*  3 */ { 3, s_1_3, -1, 1, 0},
/*  4 */ { 2, s_1_4, -1, 1, 0},
/*  5 */ { 1, s_1_5, -1, 2, 0},
/*  6 */ { 2, s_1_6, 5, 1, 0}
};

static symbol s_2_0[2] = { 'e', 'n' };
static symbol s_2_1[2] = { 'e', 'r' };
static symbol s_2_2[2] = { 's', 't' };
static symbol s_2_3[3] = { 'e', 's', 't' };

static struct among a_2[4] =
{
/*  0 */ { 2, s_2_0, -1, 1, 0},
/*  1 */ { 2, s_2_1, -1, 1, 0},
/*  2 */ { 2, s_2_2, -1, 2, 0},
/*  3 */ { 3, s_2_3, 2, 1, 0}
};

static symbol s_3_0[2] = { 'i', 'g' };
static symbol s_3_1[4] = { 'l', 'i', 'c', 'h' };

static struct among a_3[2] =
{
/*  0 */ { 2, s_3_0, -1, 1, 0},
/*  1 */ { 4, s_3_1, -1, 1, 0}
};

static symbol s_4_0[3] = { 'e', 'n', 'd' };
static symbol s_4_1[2] = { 'i', 'g' };
static symbol s_4_2[3] = { 'u', 'n', 'g' };
static symbol s_4_3[4] = { 'l', 'i', 'c', 'h' };
static symbol s_4_4[4] = { 'i', 's', 'c', 'h' };
static symbol s_4_5[2] = { 'i', 'k' };
static symbol s_4_6[4] = { 'h', 'e', 'i', 't' };
static symbol s_4_7[4] = { 'k', 'e', 'i', 't' };

static struct among a_4[8] =
{
/*  0 */ { 3, s_4_0, -1, 1, 0},
/*  1 */ { 2, s_4_1, -1, 2, 0},
/*  2 */ { 3, s_4_2, -1, 1, 0},
/*  3 */ { 4, s_4_3, -1, 3, 0},
/*  4 */ { 4, s_4_4, -1, 2, 0},
/*  5 */ { 2, s_4_5, -1, 2, 0},
/*  6 */ { 4, s_4_6, -1, 3, 0},
/*  7 */ { 4, s_4_7, -1, 4, 0}
};

static unsigned char g_v[] = { 17, 65, 16, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 32, 8 };

static unsigned char g_s_ending[] = { 117, 30, 5 };

static unsigned char g_st_ending[] = { 117, 30, 4 };

static symbol s_0[] = { 223 };
static symbol s_1[] = { 's', 's' };
static symbol s_2[] = { 'u' };
static symbol s_3[] = { 'U' };
static symbol s_4[] = { 'y' };
static symbol s_5[] = { 'Y' };
static symbol s_6[] = { 'y' };
static symbol s_7[] = { 'u' };
static symbol s_8[] = { 'a' };
static symbol s_9[] = { 'o' };
static symbol s_10[] = { 'u' };
static symbol s_11[] = { 'i', 'g' };
static symbol s_12[] = { 'e' };
static symbol s_13[] = { 'e' };
static symbol s_14[] = { 'e', 'r' };
static symbol s_15[] = { 'e', 'n' };

static int r_prelude(struct SN_env * z) {
    {   int c_test = z->c; /* test, line 30 */
        while(1) { /* repeat, line 30 */
            int c = z->c;
            {   int c = z->c; /* or, line 33 */
                z->bra = z->c; /* [, line 32 */
                if (!(eq_s(z, 1, s_0))) goto lab2;
                z->ket = z->c; /* ], line 32 */
                slice_from_s(z, 2, s_1); /* <-, line 32 */
                goto lab1;
            lab2:
                z->c = c;
                if (z->c >= z->l) goto lab0;
                z->c++; /* next, line 33 */
            }
        lab1:
            continue;
        lab0:
            z->c = c;
            break;
        }
        z->c = c_test;
    }
    while(1) { /* repeat, line 36 */
        int c = z->c;
        while(1) { /* goto, line 36 */
            int c = z->c;
            if (!(in_grouping(z, g_v, 97, 252))) goto lab4;
            z->bra = z->c; /* [, line 37 */
            {   int c = z->c; /* or, line 37 */
                if (!(eq_s(z, 1, s_2))) goto lab6;
                z->ket = z->c; /* ], line 37 */
                if (!(in_grouping(z, g_v, 97, 252))) goto lab6;
                slice_from_s(z, 1, s_3); /* <-, line 37 */
                goto lab5;
            lab6:
                z->c = c;
                if (!(eq_s(z, 1, s_4))) goto lab4;
                z->ket = z->c; /* ], line 38 */
                if (!(in_grouping(z, g_v, 97, 252))) goto lab4;
                slice_from_s(z, 1, s_5); /* <-, line 38 */
            }
        lab5:
            z->c = c;
            break;
        lab4:
            z->c = c;
            if (z->c >= z->l) goto lab3;
            z->c++;
        }
        continue;
    lab3:
        z->c = c;
        break;
    }
    return 1;
}

static int r_mark_regions(struct SN_env * z) {
    z->I[0] = z->l;
    z->I[1] = z->l;
    while(1) { /* gopast, line 47 */
        if (!(in_grouping(z, g_v, 97, 252))) goto lab0;
        break;
    lab0:
        if (z->c >= z->l) return 0;
        z->c++;
    }
    while(1) { /* gopast, line 47 */
        if (!(out_grouping(z, g_v, 97, 252))) goto lab1;
        break;
    lab1:
        if (z->c >= z->l) return 0;
        z->c++;
    }
    z->I[0] = z->c; /* setmark p1, line 47 */
     /* try, line 48 */
    if (!(z->I[0] < 3)) goto lab2;
    z->I[0] = 3;
lab2:
    while(1) { /* gopast, line 49 */
        if (!(in_grouping(z, g_v, 97, 252))) goto lab3;
        break;
    lab3:
        if (z->c >= z->l) return 0;
        z->c++;
    }
    while(1) { /* gopast, line 49 */
        if (!(out_grouping(z, g_v, 97, 252))) goto lab4;
        break;
    lab4:
        if (z->c >= z->l) return 0;
        z->c++;
    }
    z->I[1] = z->c; /* setmark p2, line 49 */
    return 1;
}

static int r_postlude(struct SN_env * z) {
    int among_var;
    while(1) { /* repeat, line 53 */
        int c = z->c;
        z->bra = z->c; /* [, line 55 */
        among_var = find_among(z, a_0, 6); /* substring, line 55 */
        if (!(among_var)) goto lab0;
        z->ket = z->c; /* ], line 55 */
        switch(among_var) {
            case 0: goto lab0;
            case 1:
                slice_from_s(z, 1, s_6); /* <-, line 56 */
                break;
            case 2:
                slice_from_s(z, 1, s_7); /* <-, line 57 */
                break;
            case 3:
                slice_from_s(z, 1, s_8); /* <-, line 58 */
                break;
            case 4:
                slice_from_s(z, 1, s_9); /* <-, line 59 */
                break;
            case 5:
                slice_from_s(z, 1, s_10); /* <-, line 60 */
                break;
            case 6:
                if (z->c >= z->l) goto lab0;
                z->c++; /* next, line 61 */
                break;
        }
        continue;
    lab0:
        z->c = c;
        break;
    }
    return 1;
}

static int r_R1(struct SN_env * z) {
    if (!(z->I[0] <= z->c)) return 0;
    return 1;
}

static int r_R2(struct SN_env * z) {
    if (!(z->I[1] <= z->c)) return 0;
    return 1;
}

static int r_standard_suffix(struct SN_env * z) {
    int among_var;
    {   int m = z->l - z->c; /* do, line 72 */
        z->ket = z->c; /* [, line 73 */
        among_var = find_among_b(z, a_1, 7); /* substring, line 73 */
        if (!(among_var)) goto lab0;
        z->bra = z->c; /* ], line 73 */
        if (!r_R1(z)) goto lab0; /* call R1, line 73 */
        switch(among_var) {
            case 0: goto lab0;
            case 1:
                slice_del(z); /* delete, line 75 */
                break;
            case 2:
                if (!(in_grouping_b(z, g_s_ending, 98, 116))) goto lab0;
                slice_del(z); /* delete, line 78 */
                break;
        }
    lab0:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; /* do, line 82 */
        z->ket = z->c; /* [, line 83 */
        among_var = find_among_b(z, a_2, 4); /* substring, line 83 */
        if (!(among_var)) goto lab1;
        z->bra = z->c; /* ], line 83 */
        if (!r_R1(z)) goto lab1; /* call R1, line 83 */
        switch(among_var) {
            case 0: goto lab1;
            case 1:
                slice_del(z); /* delete, line 85 */
                break;
            case 2:
                if (!(in_grouping_b(z, g_st_ending, 98, 116))) goto lab1;
                {   int c = z->c - 3;
                    if (z->lb > c || c > z->l) goto lab1;
                    z->c = c; /* hop, line 88 */
                }
                slice_del(z); /* delete, line 88 */
                break;
        }
    lab1:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; /* do, line 92 */
        z->ket = z->c; /* [, line 93 */
        among_var = find_among_b(z, a_4, 8); /* substring, line 93 */
        if (!(among_var)) goto lab2;
        z->bra = z->c; /* ], line 93 */
        if (!r_R2(z)) goto lab2; /* call R2, line 93 */
        switch(among_var) {
            case 0: goto lab2;
            case 1:
                slice_del(z); /* delete, line 95 */
                {   int m = z->l - z->c; /* try, line 96 */
                    z->ket = z->c; /* [, line 96 */
                    if (!(eq_s_b(z, 2, s_11))) { z->c = z->l - m; goto lab3; }
                    z->bra = z->c; /* ], line 96 */
                    {   int m = z->l - z->c; /* not, line 96 */
                        if (!(eq_s_b(z, 1, s_12))) goto lab4;
                        { z->c = z->l - m; goto lab3; }
                    lab4:
                        z->c = z->l - m;
                    }
                    if (!r_R2(z)) { z->c = z->l - m; goto lab3; } /* call R2, line 96 */
                    slice_del(z); /* delete, line 96 */
                lab3:
                    ;
                }
                break;
            case 2:
                {   int m = z->l - z->c; /* not, line 99 */
                    if (!(eq_s_b(z, 1, s_13))) goto lab5;
                    goto lab2;
                lab5:
                    z->c = z->l - m;
                }
                slice_del(z); /* delete, line 99 */
                break;
            case 3:
                slice_del(z); /* delete, line 102 */
                {   int m = z->l - z->c; /* try, line 103 */
                    z->ket = z->c; /* [, line 104 */
                    {   int m = z->l - z->c; /* or, line 104 */
                        if (!(eq_s_b(z, 2, s_14))) goto lab8;
                        goto lab7;
                    lab8:
                        z->c = z->l - m;
                        if (!(eq_s_b(z, 2, s_15))) { z->c = z->l - m; goto lab6; }
                    }
                lab7:
                    z->bra = z->c; /* ], line 104 */
                    if (!r_R1(z)) { z->c = z->l - m; goto lab6; } /* call R1, line 104 */
                    slice_del(z); /* delete, line 104 */
                lab6:
                    ;
                }
                break;
            case 4:
                slice_del(z); /* delete, line 108 */
                {   int m = z->l - z->c; /* try, line 109 */
                    z->ket = z->c; /* [, line 110 */
                    among_var = find_among_b(z, a_3, 2); /* substring, line 110 */
                    if (!(among_var)) { z->c = z->l - m; goto lab9; }
                    z->bra = z->c; /* ], line 110 */
                    if (!r_R2(z)) { z->c = z->l - m; goto lab9; } /* call R2, line 110 */
                    switch(among_var) {
                        case 0: { z->c = z->l - m; goto lab9; }
                        case 1:
                            slice_del(z); /* delete, line 112 */
                            break;
                    }
                lab9:
                    ;
                }
                break;
        }
    lab2:
        z->c = z->l - m;
    }
    return 1;
}

extern int snowball_german_stem(struct SN_env * z) {
    {   int c = z->c; /* do, line 123 */
        if (!r_prelude(z)) goto lab0; /* call prelude, line 123 */
    lab0:
        z->c = c;
    }
    {   int c = z->c; /* do, line 124 */
        if (!r_mark_regions(z)) goto lab1; /* call mark_regions, line 124 */
    lab1:
        z->c = c;
    }
    z->lb = z->c; z->c = z->l; /* backwards, line 125 */

    {   int m = z->l - z->c; /* do, line 126 */
        if (!r_standard_suffix(z)) goto lab2; /* call standard_suffix, line 126 */
    lab2:
        z->c = z->l - m;
    }
    z->c = z->lb;
    {   int c = z->c; /* do, line 127 */
        if (!r_postlude(z)) goto lab3; /* call postlude, line 127 */
    lab3:
        z->c = c;
    }
    return 1;
}

extern struct SN_env * snowball_german_create_env(void) { return SN_create_env(0, 2, 0); }

extern void snowball_german_close_env(struct SN_env * z) { SN_close_env(z); }

