#include <config.h>

/* This file was generated automatically by the Snowball to ANSI C compiler */

#include "header.h"

extern int snowball_dutch_stem(struct SN_env * z);
static int r_standard_suffix(struct SN_env * z);
static int r_undouble(struct SN_env * z);
static int r_R2(struct SN_env * z);
static int r_R1(struct SN_env * z);
static int r_mark_regions(struct SN_env * z);
static int r_en_ending(struct SN_env * z);
static int r_e_ending(struct SN_env * z);
static int r_postlude(struct SN_env * z);
static int r_prelude(struct SN_env * z);

static symbol s_0_1[1] = { 225 };
static symbol s_0_2[1] = { 228 };
static symbol s_0_3[1] = { 233 };
static symbol s_0_4[1] = { 235 };
static symbol s_0_5[1] = { 237 };
static symbol s_0_6[1] = { 239 };
static symbol s_0_7[1] = { 243 };
static symbol s_0_8[1] = { 246 };
static symbol s_0_9[1] = { 250 };
static symbol s_0_10[1] = { 252 };

static struct among a_0[11] =
{
/*  0 */ { 0, 0, -1, 6, 0},
/*  1 */ { 1, s_0_1, 0, 1, 0},
/*  2 */ { 1, s_0_2, 0, 1, 0},
/*  3 */ { 1, s_0_3, 0, 2, 0},
/*  4 */ { 1, s_0_4, 0, 2, 0},
/*  5 */ { 1, s_0_5, 0, 3, 0},
/*  6 */ { 1, s_0_6, 0, 3, 0},
/*  7 */ { 1, s_0_7, 0, 4, 0},
/*  8 */ { 1, s_0_8, 0, 4, 0},
/*  9 */ { 1, s_0_9, 0, 5, 0},
/* 10 */ { 1, s_0_10, 0, 5, 0}
};

static symbol s_1_1[1] = { 'I' };
static symbol s_1_2[1] = { 'Y' };

static struct among a_1[3] =
{
/*  0 */ { 0, 0, -1, 3, 0},
/*  1 */ { 1, s_1_1, 0, 2, 0},
/*  2 */ { 1, s_1_2, 0, 1, 0}
};

static symbol s_2_0[2] = { 'd', 'd' };
static symbol s_2_1[2] = { 'k', 'k' };
static symbol s_2_2[2] = { 't', 't' };

static struct among a_2[3] =
{
/*  0 */ { 2, s_2_0, -1, -1, 0},
/*  1 */ { 2, s_2_1, -1, -1, 0},
/*  2 */ { 2, s_2_2, -1, -1, 0}
};

static symbol s_3_0[3] = { 'e', 'n', 'e' };
static symbol s_3_1[2] = { 's', 'e' };
static symbol s_3_2[2] = { 'e', 'n' };
static symbol s_3_3[5] = { 'h', 'e', 'd', 'e', 'n' };
static symbol s_3_4[1] = { 's' };

static struct among a_3[5] =
{
/*  0 */ { 3, s_3_0, -1, 2, 0},
/*  1 */ { 2, s_3_1, -1, 3, 0},
/*  2 */ { 2, s_3_2, -1, 2, 0},
/*  3 */ { 5, s_3_3, 2, 1, 0},
/*  4 */ { 1, s_3_4, -1, 3, 0}
};

static symbol s_4_0[3] = { 'e', 'n', 'd' };
static symbol s_4_1[2] = { 'i', 'g' };
static symbol s_4_2[3] = { 'i', 'n', 'g' };
static symbol s_4_3[4] = { 'l', 'i', 'j', 'k' };
static symbol s_4_4[4] = { 'b', 'a', 'a', 'r' };
static symbol s_4_5[3] = { 'b', 'a', 'r' };

static struct among a_4[6] =
{
/*  0 */ { 3, s_4_0, -1, 1, 0},
/*  1 */ { 2, s_4_1, -1, 2, 0},
/*  2 */ { 3, s_4_2, -1, 1, 0},
/*  3 */ { 4, s_4_3, -1, 3, 0},
/*  4 */ { 4, s_4_4, -1, 4, 0},
/*  5 */ { 3, s_4_5, -1, 5, 0}
};

static symbol s_5_0[2] = { 'a', 'a' };
static symbol s_5_1[2] = { 'e', 'e' };
static symbol s_5_2[2] = { 'o', 'o' };
static symbol s_5_3[2] = { 'u', 'u' };

static struct among a_5[4] =
{
/*  0 */ { 2, s_5_0, -1, -1, 0},
/*  1 */ { 2, s_5_1, -1, -1, 0},
/*  2 */ { 2, s_5_2, -1, -1, 0},
/*  3 */ { 2, s_5_3, -1, -1, 0}
};

static unsigned char g_v[] = { 17, 65, 16, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128 };

static unsigned char g_v_I[] = { 1, 0, 0, 17, 65, 16, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128 };

static unsigned char g_v_j[] = { 17, 67, 16, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128 };

static symbol s_0[] = { 'a' };
static symbol s_1[] = { 'e' };
static symbol s_2[] = { 'i' };
static symbol s_3[] = { 'o' };
static symbol s_4[] = { 'u' };
static symbol s_5[] = { 'y' };
static symbol s_6[] = { 'Y' };
static symbol s_7[] = { 'i' };
static symbol s_8[] = { 'I' };
static symbol s_9[] = { 'y' };
static symbol s_10[] = { 'Y' };
static symbol s_11[] = { 'y' };
static symbol s_12[] = { 'i' };
static symbol s_13[] = { 'e' };
static symbol s_14[] = { 'g', 'e', 'm' };
static symbol s_15[] = { 'h', 'e', 'i', 'd' };
static symbol s_16[] = { 'h', 'e', 'i', 'd' };
static symbol s_17[] = { 'c' };
static symbol s_18[] = { 'e', 'n' };
static symbol s_19[] = { 'i', 'g' };
static symbol s_20[] = { 'e' };
static symbol s_21[] = { 'e' };

static int r_prelude(struct SN_env * z) {
    int among_var;
    {   int c_test = z->c; /* test, line 42 */
        while(1) { /* repeat, line 42 */
            int c = z->c;
            z->bra = z->c; /* [, line 43 */
            among_var = find_among(z, a_0, 11); /* substring, line 43 */
            if (!(among_var)) goto lab0;
            z->ket = z->c; /* ], line 43 */
            switch(among_var) {
                case 0: goto lab0;
                case 1:
                    slice_from_s(z, 1, s_0); /* <-, line 45 */
                    break;
                case 2:
                    slice_from_s(z, 1, s_1); /* <-, line 47 */
                    break;
                case 3:
                    slice_from_s(z, 1, s_2); /* <-, line 49 */
                    break;
                case 4:
                    slice_from_s(z, 1, s_3); /* <-, line 51 */
                    break;
                case 5:
                    slice_from_s(z, 1, s_4); /* <-, line 53 */
                    break;
                case 6:
                    if (z->c >= z->l) goto lab0;
                    z->c++; /* next, line 54 */
                    break;
            }
            continue;
        lab0:
            z->c = c;
            break;
        }
        z->c = c_test;
    }
    {   int c = z->c; /* try, line 57 */
        z->bra = z->c; /* [, line 57 */
        if (!(eq_s(z, 1, s_5))) { z->c = c; goto lab1; }
        z->ket = z->c; /* ], line 57 */
        slice_from_s(z, 1, s_6); /* <-, line 57 */
    lab1:
        ;
    }
    while(1) { /* repeat, line 58 */
        int c = z->c;
        while(1) { /* goto, line 58 */
            int c = z->c;
            if (!(in_grouping(z, g_v, 97, 232))) goto lab3;
            z->bra = z->c; /* [, line 59 */
            {   int c = z->c; /* or, line 59 */
                if (!(eq_s(z, 1, s_7))) goto lab5;
                z->ket = z->c; /* ], line 59 */
                if (!(in_grouping(z, g_v, 97, 232))) goto lab5;
                slice_from_s(z, 1, s_8); /* <-, line 59 */
                goto lab4;
            lab5:
                z->c = c;
                if (!(eq_s(z, 1, s_9))) goto lab3;
                z->ket = z->c; /* ], line 60 */
                slice_from_s(z, 1, s_10); /* <-, line 60 */
            }
        lab4:
            z->c = c;
            break;
        lab3:
            z->c = c;
            if (z->c >= z->l) goto lab2;
            z->c++;
        }
        continue;
    lab2:
        z->c = c;
        break;
    }
    return 1;
}

static int r_mark_regions(struct SN_env * z) {
    z->I[0] = z->l;
    z->I[1] = z->l;
    while(1) { /* gopast, line 69 */
        if (!(in_grouping(z, g_v, 97, 232))) goto lab0;
        break;
    lab0:
        if (z->c >= z->l) return 0;
        z->c++;
    }
    while(1) { /* gopast, line 69 */
        if (!(out_grouping(z, g_v, 97, 232))) goto lab1;
        break;
    lab1:
        if (z->c >= z->l) return 0;
        z->c++;
    }
    z->I[0] = z->c; /* setmark p1, line 69 */
     /* try, line 70 */
    if (!(z->I[0] < 3)) goto lab2;
    z->I[0] = 3;
lab2:
    while(1) { /* gopast, line 71 */
        if (!(in_grouping(z, g_v, 97, 232))) goto lab3;
        break;
    lab3:
        if (z->c >= z->l) return 0;
        z->c++;
    }
    while(1) { /* gopast, line 71 */
        if (!(out_grouping(z, g_v, 97, 232))) goto lab4;
        break;
    lab4:
        if (z->c >= z->l) return 0;
        z->c++;
    }
    z->I[1] = z->c; /* setmark p2, line 71 */
    return 1;
}

static int r_postlude(struct SN_env * z) {
    int among_var;
    while(1) { /* repeat, line 75 */
        int c = z->c;
        z->bra = z->c; /* [, line 77 */
        among_var = find_among(z, a_1, 3); /* substring, line 77 */
        if (!(among_var)) goto lab0;
        z->ket = z->c; /* ], line 77 */
        switch(among_var) {
            case 0: goto lab0;
            case 1:
                slice_from_s(z, 1, s_11); /* <-, line 78 */
                break;
            case 2:
                slice_from_s(z, 1, s_12); /* <-, line 79 */
                break;
            case 3:
                if (z->c >= z->l) goto lab0;
                z->c++; /* next, line 80 */
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

static int r_undouble(struct SN_env * z) {
    {   int m_test = z->l - z->c; /* test, line 91 */
        if (!(find_among_b(z, a_2, 3))) return 0; /* among, line 91 */
        z->c = z->l - m_test;
    }
    z->ket = z->c; /* [, line 91 */
    if (z->c <= z->lb) return 0;
    z->c--; /* next, line 91 */
    z->bra = z->c; /* ], line 91 */
    slice_del(z); /* delete, line 91 */
    return 1;
}

static int r_e_ending(struct SN_env * z) {
    z->B[0] = 0; /* unset e_found, line 95 */
    z->ket = z->c; /* [, line 96 */
    if (!(eq_s_b(z, 1, s_13))) return 0;
    z->bra = z->c; /* ], line 96 */
    if (!r_R1(z)) return 0; /* call R1, line 96 */
    {   int m_test = z->l - z->c; /* test, line 96 */
        if (!(out_grouping_b(z, g_v, 97, 232))) return 0;
        z->c = z->l - m_test;
    }
    slice_del(z); /* delete, line 96 */
    z->B[0] = 1; /* set e_found, line 97 */
    if (!r_undouble(z)) return 0; /* call undouble, line 98 */
    return 1;
}

static int r_en_ending(struct SN_env * z) {
    if (!r_R1(z)) return 0; /* call R1, line 102 */
    {   int m = z->l - z->c; /* and, line 102 */
        if (!(out_grouping_b(z, g_v, 97, 232))) return 0;
        z->c = z->l - m;
        {   int m = z->l - z->c; /* not, line 102 */
            if (!(eq_s_b(z, 3, s_14))) goto lab0;
            return 0;
        lab0:
            z->c = z->l - m;
        }
    }
    slice_del(z); /* delete, line 102 */
    if (!r_undouble(z)) return 0; /* call undouble, line 103 */
    return 1;
}

static int r_standard_suffix(struct SN_env * z) {
    int among_var;
    {   int m = z->l - z->c; /* do, line 107 */
        z->ket = z->c; /* [, line 108 */
        among_var = find_among_b(z, a_3, 5); /* substring, line 108 */
        if (!(among_var)) goto lab0;
        z->bra = z->c; /* ], line 108 */
        switch(among_var) {
            case 0: goto lab0;
            case 1:
                if (!r_R1(z)) goto lab0; /* call R1, line 110 */
                slice_from_s(z, 4, s_15); /* <-, line 110 */
                break;
            case 2:
                if (!r_en_ending(z)) goto lab0; /* call en_ending, line 113 */
                break;
            case 3:
                if (!r_R1(z)) goto lab0; /* call R1, line 116 */
                if (!(out_grouping_b(z, g_v_j, 97, 232))) goto lab0;
                slice_del(z); /* delete, line 116 */
                break;
        }
    lab0:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; /* do, line 120 */
        if (!r_e_ending(z)) goto lab1; /* call e_ending, line 120 */
    lab1:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; /* do, line 122 */
        z->ket = z->c; /* [, line 122 */
        if (!(eq_s_b(z, 4, s_16))) goto lab2;
        z->bra = z->c; /* ], line 122 */
        if (!r_R2(z)) goto lab2; /* call R2, line 122 */
        {   int m = z->l - z->c; /* not, line 122 */
            if (!(eq_s_b(z, 1, s_17))) goto lab3;
            goto lab2;
        lab3:
            z->c = z->l - m;
        }
        slice_del(z); /* delete, line 122 */
        z->ket = z->c; /* [, line 123 */
        if (!(eq_s_b(z, 2, s_18))) goto lab2;
        z->bra = z->c; /* ], line 123 */
        if (!r_en_ending(z)) goto lab2; /* call en_ending, line 123 */
    lab2:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; /* do, line 126 */
        z->ket = z->c; /* [, line 127 */
        among_var = find_among_b(z, a_4, 6); /* substring, line 127 */
        if (!(among_var)) goto lab4;
        z->bra = z->c; /* ], line 127 */
        switch(among_var) {
            case 0: goto lab4;
            case 1:
                if (!r_R2(z)) goto lab4; /* call R2, line 129 */
                slice_del(z); /* delete, line 129 */
                {   int m = z->l - z->c; /* or, line 130 */
                    z->ket = z->c; /* [, line 130 */
                    if (!(eq_s_b(z, 2, s_19))) goto lab6;
                    z->bra = z->c; /* ], line 130 */
                    if (!r_R2(z)) goto lab6; /* call R2, line 130 */
                    {   int m = z->l - z->c; /* not, line 130 */
                        if (!(eq_s_b(z, 1, s_20))) goto lab7;
                        goto lab6;
                    lab7:
                        z->c = z->l - m;
                    }
                    slice_del(z); /* delete, line 130 */
                    goto lab5;
                lab6:
                    z->c = z->l - m;
                    if (!r_undouble(z)) goto lab4; /* call undouble, line 130 */
                }
            lab5:
                break;
            case 2:
                if (!r_R2(z)) goto lab4; /* call R2, line 133 */
                {   int m = z->l - z->c; /* not, line 133 */
                    if (!(eq_s_b(z, 1, s_21))) goto lab8;
                    goto lab4;
                lab8:
                    z->c = z->l - m;
                }
                slice_del(z); /* delete, line 133 */
                break;
            case 3:
                if (!r_R2(z)) goto lab4; /* call R2, line 136 */
                slice_del(z); /* delete, line 136 */
                if (!r_e_ending(z)) goto lab4; /* call e_ending, line 136 */
                break;
            case 4:
                if (!r_R2(z)) goto lab4; /* call R2, line 139 */
                slice_del(z); /* delete, line 139 */
                break;
            case 5:
                if (!r_R2(z)) goto lab4; /* call R2, line 142 */
                if (!(z->B[0])) goto lab4; /* Boolean test e_found, line 142 */
                slice_del(z); /* delete, line 142 */
                break;
        }
    lab4:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; /* do, line 146 */
        if (!(out_grouping_b(z, g_v_I, 73, 232))) goto lab9;
        {   int m_test = z->l - z->c; /* test, line 148 */
            if (!(find_among_b(z, a_5, 4))) goto lab9; /* among, line 149 */
            if (!(out_grouping_b(z, g_v, 97, 232))) goto lab9;
            z->c = z->l - m_test;
        }
        z->ket = z->c; /* [, line 152 */
        if (z->c <= z->lb) goto lab9;
        z->c--; /* next, line 152 */
        z->bra = z->c; /* ], line 152 */
        slice_del(z); /* delete, line 152 */
    lab9:
        z->c = z->l - m;
    }
    return 1;
}

extern int snowball_dutch_stem(struct SN_env * z) {
    {   int c = z->c; /* do, line 159 */
        if (!r_prelude(z)) goto lab0; /* call prelude, line 159 */
    lab0:
        z->c = c;
    }
    {   int c = z->c; /* do, line 160 */
        if (!r_mark_regions(z)) goto lab1; /* call mark_regions, line 160 */
    lab1:
        z->c = c;
    }
    z->lb = z->c; z->c = z->l; /* backwards, line 161 */

    {   int m = z->l - z->c; /* do, line 162 */
        if (!r_standard_suffix(z)) goto lab2; /* call standard_suffix, line 162 */
    lab2:
        z->c = z->l - m;
    }
    z->c = z->lb;
    {   int c = z->c; /* do, line 163 */
        if (!r_postlude(z)) goto lab3; /* call postlude, line 163 */
    lab3:
        z->c = c;
    }
    return 1;
}

extern struct SN_env * snowball_dutch_create_env(void) { return SN_create_env(0, 2, 1); }

extern void snowball_dutch_close_env(struct SN_env * z) { SN_close_env(z); }

