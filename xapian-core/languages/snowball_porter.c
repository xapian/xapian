
/* This file was generated automatically by the Snowball to ANSI C compiler */

#include "header.h"

extern int snowball_porter_stem(struct SN_env * z);
static int r_Step_5b(struct SN_env * z);
static int r_Step_5a(struct SN_env * z);
static int r_Step_4(struct SN_env * z);
static int r_Step_3(struct SN_env * z);
static int r_Step_2(struct SN_env * z);
static int r_Step_1c(struct SN_env * z);
static int r_Step_1b(struct SN_env * z);
static int r_Step_1a(struct SN_env * z);
static int r_R2(struct SN_env * z);
static int r_R1(struct SN_env * z);
static int r_shortv(struct SN_env * z);

static symbol s_0_0[1] = { 's' };
static symbol s_0_1[3] = { 'i', 'e', 's' };
static symbol s_0_2[4] = { 's', 's', 'e', 's' };
static symbol s_0_3[2] = { 's', 's' };

static struct among a_0[4] =
{
/*  0 */ { 1, s_0_0, -1, 3, 0},
/*  1 */ { 3, s_0_1, 0, 2, 0},
/*  2 */ { 4, s_0_2, 0, 1, 0},
/*  3 */ { 2, s_0_3, 0, -1, 0}
};

static symbol s_1_1[2] = { 'b', 'b' };
static symbol s_1_2[2] = { 'd', 'd' };
static symbol s_1_3[2] = { 'f', 'f' };
static symbol s_1_4[2] = { 'g', 'g' };
static symbol s_1_5[2] = { 'b', 'l' };
static symbol s_1_6[2] = { 'm', 'm' };
static symbol s_1_7[2] = { 'n', 'n' };
static symbol s_1_8[2] = { 'p', 'p' };
static symbol s_1_9[2] = { 'r', 'r' };
static symbol s_1_10[2] = { 'a', 't' };
static symbol s_1_11[2] = { 't', 't' };
static symbol s_1_12[2] = { 'i', 'z' };

static struct among a_1[13] =
{
/*  0 */ { 0, 0, -1, 3, 0},
/*  1 */ { 2, s_1_1, 0, 2, 0},
/*  2 */ { 2, s_1_2, 0, 2, 0},
/*  3 */ { 2, s_1_3, 0, 2, 0},
/*  4 */ { 2, s_1_4, 0, 2, 0},
/*  5 */ { 2, s_1_5, 0, 1, 0},
/*  6 */ { 2, s_1_6, 0, 2, 0},
/*  7 */ { 2, s_1_7, 0, 2, 0},
/*  8 */ { 2, s_1_8, 0, 2, 0},
/*  9 */ { 2, s_1_9, 0, 2, 0},
/* 10 */ { 2, s_1_10, 0, 1, 0},
/* 11 */ { 2, s_1_11, 0, 2, 0},
/* 12 */ { 2, s_1_12, 0, 1, 0}
};

static symbol s_2_0[2] = { 'e', 'd' };
static symbol s_2_1[3] = { 'e', 'e', 'd' };
static symbol s_2_2[3] = { 'i', 'n', 'g' };

static struct among a_2[3] =
{
/*  0 */ { 2, s_2_0, -1, 2, 0},
/*  1 */ { 3, s_2_1, 0, 1, 0},
/*  2 */ { 3, s_2_2, -1, 2, 0}
};

static symbol s_3_0[4] = { 'a', 'n', 'c', 'i' };
static symbol s_3_1[4] = { 'e', 'n', 'c', 'i' };
static symbol s_3_2[4] = { 'a', 'b', 'l', 'i' };
static symbol s_3_3[3] = { 'e', 'l', 'i' };
static symbol s_3_4[4] = { 'a', 'l', 'l', 'i' };
static symbol s_3_5[5] = { 'o', 'u', 's', 'l', 'i' };
static symbol s_3_6[5] = { 'e', 'n', 't', 'l', 'i' };
static symbol s_3_7[5] = { 'a', 'l', 'i', 't', 'i' };
static symbol s_3_8[6] = { 'b', 'i', 'l', 'i', 't', 'i' };
static symbol s_3_9[5] = { 'i', 'v', 'i', 't', 'i' };
static symbol s_3_10[6] = { 't', 'i', 'o', 'n', 'a', 'l' };
static symbol s_3_11[7] = { 'a', 't', 'i', 'o', 'n', 'a', 'l' };
static symbol s_3_12[5] = { 'a', 'l', 'i', 's', 'm' };
static symbol s_3_13[5] = { 'a', 't', 'i', 'o', 'n' };
static symbol s_3_14[7] = { 'i', 'z', 'a', 't', 'i', 'o', 'n' };
static symbol s_3_15[4] = { 'i', 'z', 'e', 'r' };
static symbol s_3_16[4] = { 'a', 't', 'o', 'r' };
static symbol s_3_17[7] = { 'i', 'v', 'e', 'n', 'e', 's', 's' };
static symbol s_3_18[7] = { 'f', 'u', 'l', 'n', 'e', 's', 's' };
static symbol s_3_19[7] = { 'o', 'u', 's', 'n', 'e', 's', 's' };

static struct among a_3[20] =
{
/*  0 */ { 4, s_3_0, -1, 3, 0},
/*  1 */ { 4, s_3_1, -1, 2, 0},
/*  2 */ { 4, s_3_2, -1, 4, 0},
/*  3 */ { 3, s_3_3, -1, 6, 0},
/*  4 */ { 4, s_3_4, -1, 9, 0},
/*  5 */ { 5, s_3_5, -1, 12, 0},
/*  6 */ { 5, s_3_6, -1, 5, 0},
/*  7 */ { 5, s_3_7, -1, 10, 0},
/*  8 */ { 6, s_3_8, -1, 14, 0},
/*  9 */ { 5, s_3_9, -1, 13, 0},
/* 10 */ { 6, s_3_10, -1, 1, 0},
/* 11 */ { 7, s_3_11, 10, 8, 0},
/* 12 */ { 5, s_3_12, -1, 10, 0},
/* 13 */ { 5, s_3_13, -1, 8, 0},
/* 14 */ { 7, s_3_14, 13, 7, 0},
/* 15 */ { 4, s_3_15, -1, 7, 0},
/* 16 */ { 4, s_3_16, -1, 8, 0},
/* 17 */ { 7, s_3_17, -1, 13, 0},
/* 18 */ { 7, s_3_18, -1, 11, 0},
/* 19 */ { 7, s_3_19, -1, 12, 0}
};

static symbol s_4_0[5] = { 'i', 'c', 'a', 't', 'e' };
static symbol s_4_1[5] = { 'a', 't', 'i', 'v', 'e' };
static symbol s_4_2[5] = { 'a', 'l', 'i', 'z', 'e' };
static symbol s_4_3[5] = { 'i', 'c', 'i', 't', 'i' };
static symbol s_4_4[4] = { 'i', 'c', 'a', 'l' };
static symbol s_4_5[3] = { 'f', 'u', 'l' };
static symbol s_4_6[4] = { 'n', 'e', 's', 's' };

static struct among a_4[7] =
{
/*  0 */ { 5, s_4_0, -1, 2, 0},
/*  1 */ { 5, s_4_1, -1, 3, 0},
/*  2 */ { 5, s_4_2, -1, 1, 0},
/*  3 */ { 5, s_4_3, -1, 2, 0},
/*  4 */ { 4, s_4_4, -1, 2, 0},
/*  5 */ { 3, s_4_5, -1, 3, 0},
/*  6 */ { 4, s_4_6, -1, 3, 0}
};

static symbol s_5_0[2] = { 'i', 'c' };
static symbol s_5_1[4] = { 'a', 'n', 'c', 'e' };
static symbol s_5_2[4] = { 'e', 'n', 'c', 'e' };
static symbol s_5_3[4] = { 'a', 'b', 'l', 'e' };
static symbol s_5_4[4] = { 'i', 'b', 'l', 'e' };
static symbol s_5_5[3] = { 'a', 't', 'e' };
static symbol s_5_6[3] = { 'i', 'v', 'e' };
static symbol s_5_7[3] = { 'i', 'z', 'e' };
static symbol s_5_8[3] = { 'i', 't', 'i' };
static symbol s_5_9[2] = { 'a', 'l' };
static symbol s_5_10[3] = { 'i', 's', 'm' };
static symbol s_5_11[3] = { 'i', 'o', 'n' };
static symbol s_5_12[2] = { 'e', 'r' };
static symbol s_5_13[3] = { 'o', 'u', 's' };
static symbol s_5_14[3] = { 'a', 'n', 't' };
static symbol s_5_15[3] = { 'e', 'n', 't' };
static symbol s_5_16[4] = { 'm', 'e', 'n', 't' };
static symbol s_5_17[5] = { 'e', 'm', 'e', 'n', 't' };
static symbol s_5_18[2] = { 'o', 'u' };

static struct among a_5[19] =
{
/*  0 */ { 2, s_5_0, -1, 1, 0},
/*  1 */ { 4, s_5_1, -1, 1, 0},
/*  2 */ { 4, s_5_2, -1, 1, 0},
/*  3 */ { 4, s_5_3, -1, 1, 0},
/*  4 */ { 4, s_5_4, -1, 1, 0},
/*  5 */ { 3, s_5_5, -1, 1, 0},
/*  6 */ { 3, s_5_6, -1, 1, 0},
/*  7 */ { 3, s_5_7, -1, 1, 0},
/*  8 */ { 3, s_5_8, -1, 1, 0},
/*  9 */ { 2, s_5_9, -1, 1, 0},
/* 10 */ { 3, s_5_10, -1, 1, 0},
/* 11 */ { 3, s_5_11, -1, 2, 0},
/* 12 */ { 2, s_5_12, -1, 1, 0},
/* 13 */ { 3, s_5_13, -1, 1, 0},
/* 14 */ { 3, s_5_14, -1, 1, 0},
/* 15 */ { 3, s_5_15, -1, 1, 0},
/* 16 */ { 4, s_5_16, 15, 1, 0},
/* 17 */ { 5, s_5_17, 16, 1, 0},
/* 18 */ { 2, s_5_18, -1, 1, 0}
};

static unsigned char g_v[] = { 17, 65, 16, 1 };

static unsigned char g_v_WXY[] = { 1, 17, 65, 208, 1 };

static symbol s_0[] = { 's', 's' };
static symbol s_1[] = { 'i' };
static symbol s_2[] = { 'e', 'e' };
static symbol s_3[] = { 'e' };
static symbol s_4[] = { 'e' };
static symbol s_5[] = { 'y' };
static symbol s_6[] = { 'Y' };
static symbol s_7[] = { 'i' };
static symbol s_8[] = { 't', 'i', 'o', 'n' };
static symbol s_9[] = { 'e', 'n', 'c', 'e' };
static symbol s_10[] = { 'a', 'n', 'c', 'e' };
static symbol s_11[] = { 'a', 'b', 'l', 'e' };
static symbol s_12[] = { 'e', 'n', 't' };
static symbol s_13[] = { 'e' };
static symbol s_14[] = { 'i', 'z', 'e' };
static symbol s_15[] = { 'a', 't', 'e' };
static symbol s_16[] = { 'a', 'l' };
static symbol s_17[] = { 'a', 'l' };
static symbol s_18[] = { 'f', 'u', 'l' };
static symbol s_19[] = { 'o', 'u', 's' };
static symbol s_20[] = { 'i', 'v', 'e' };
static symbol s_21[] = { 'b', 'l', 'e' };
static symbol s_22[] = { 'a', 'l' };
static symbol s_23[] = { 'i', 'c' };
static symbol s_24[] = { 's' };
static symbol s_25[] = { 't' };
static symbol s_26[] = { 'e' };
static symbol s_27[] = { 'l' };
static symbol s_28[] = { 'l' };
static symbol s_29[] = { 'y' };
static symbol s_30[] = { 'Y' };
static symbol s_31[] = { 'y' };
static symbol s_32[] = { 'Y' };
static symbol s_33[] = { 'Y' };
static symbol s_34[] = { 'y' };

static int r_shortv(struct SN_env * z) {
    if (!(out_grouping_b(z, g_v_WXY, 89, 121))) return 0;
    if (!(in_grouping_b(z, g_v, 97, 121))) return 0;
    if (!(out_grouping_b(z, g_v, 97, 121))) return 0;
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

static int r_Step_1a(struct SN_env * z) {
    int among_var;
    z->ket = z->c; /* [, line 25 */
    among_var = find_among_b(z, a_0, 4); /* substring, line 25 */
    if (!(among_var)) return 0;
    z->bra = z->c; /* ], line 25 */
    switch(among_var) {
        case 0: return 0;
        case 1:
            slice_from_s(z, 2, s_0); /* <-, line 26 */
            break;
        case 2:
            slice_from_s(z, 1, s_1); /* <-, line 27 */
            break;
        case 3:
            slice_del(z); /* delete, line 29 */
            break;
    }
    return 1;
}

static int r_Step_1b(struct SN_env * z) {
    int among_var;
    z->ket = z->c; /* [, line 34 */
    among_var = find_among_b(z, a_2, 3); /* substring, line 34 */
    if (!(among_var)) return 0;
    z->bra = z->c; /* ], line 34 */
    switch(among_var) {
        case 0: return 0;
        case 1:
            if (!r_R1(z)) return 0; /* call R1, line 35 */
            slice_from_s(z, 2, s_2); /* <-, line 35 */
            break;
        case 2:
            {   int m_test = z->l - z->c; /* test, line 38 */
                while(1) { /* gopast, line 38 */
                    if (!(in_grouping_b(z, g_v, 97, 121))) goto lab0;
                    break;
                lab0:
                    if (z->c <= z->lb) return 0;
                    z->c--;
                }
                z->c = z->l - m_test;
            }
            slice_del(z); /* delete, line 38 */
            {   int m_test = z->l - z->c; /* test, line 39 */
                among_var = find_among_b(z, a_1, 13); /* substring, line 39 */
                if (!(among_var)) return 0;
                z->c = z->l - m_test;
            }
            switch(among_var) {
                case 0: return 0;
                case 1:
                    {   int c = z->c;
                        insert_s(z, z->c, z->c, 1, s_3); /* <+, line 41 */
                        z->c = c;
                    }
                    break;
                case 2:
                    z->ket = z->c; /* [, line 44 */
                    if (z->c <= z->lb) return 0;
                    z->c--; /* next, line 44 */
                    z->bra = z->c; /* ], line 44 */
                    slice_del(z); /* delete, line 44 */
                    break;
                case 3:
                    if (z->c != z->I[0]) return 0; /* atmark, line 45 */
                    {   int m_test = z->l - z->c; /* test, line 45 */
                        if (!r_shortv(z)) return 0; /* call shortv, line 45 */
                        z->c = z->l - m_test;
                    }
                    {   int c = z->c;
                        insert_s(z, z->c, z->c, 1, s_4); /* <+, line 45 */
                        z->c = c;
                    }
                    break;
            }
            break;
    }
    return 1;
}

static int r_Step_1c(struct SN_env * z) {
    z->ket = z->c; /* [, line 52 */
    {   int m = z->l - z->c; /* or, line 52 */
        if (!(eq_s_b(z, 1, s_5))) goto lab1;
        goto lab0;
    lab1:
        z->c = z->l - m;
        if (!(eq_s_b(z, 1, s_6))) return 0;
    }
lab0:
    z->bra = z->c; /* ], line 52 */
    while(1) { /* gopast, line 53 */
        if (!(in_grouping_b(z, g_v, 97, 121))) goto lab2;
        break;
    lab2:
        if (z->c <= z->lb) return 0;
        z->c--;
    }
    slice_from_s(z, 1, s_7); /* <-, line 54 */
    return 1;
}

static int r_Step_2(struct SN_env * z) {
    int among_var;
    z->ket = z->c; /* [, line 58 */
    among_var = find_among_b(z, a_3, 20); /* substring, line 58 */
    if (!(among_var)) return 0;
    z->bra = z->c; /* ], line 58 */
    if (!r_R1(z)) return 0; /* call R1, line 58 */
    switch(among_var) {
        case 0: return 0;
        case 1:
            slice_from_s(z, 4, s_8); /* <-, line 59 */
            break;
        case 2:
            slice_from_s(z, 4, s_9); /* <-, line 60 */
            break;
        case 3:
            slice_from_s(z, 4, s_10); /* <-, line 61 */
            break;
        case 4:
            slice_from_s(z, 4, s_11); /* <-, line 62 */
            break;
        case 5:
            slice_from_s(z, 3, s_12); /* <-, line 63 */
            break;
        case 6:
            slice_from_s(z, 1, s_13); /* <-, line 64 */
            break;
        case 7:
            slice_from_s(z, 3, s_14); /* <-, line 66 */
            break;
        case 8:
            slice_from_s(z, 3, s_15); /* <-, line 68 */
            break;
        case 9:
            slice_from_s(z, 2, s_16); /* <-, line 69 */
            break;
        case 10:
            slice_from_s(z, 2, s_17); /* <-, line 71 */
            break;
        case 11:
            slice_from_s(z, 3, s_18); /* <-, line 72 */
            break;
        case 12:
            slice_from_s(z, 3, s_19); /* <-, line 74 */
            break;
        case 13:
            slice_from_s(z, 3, s_20); /* <-, line 76 */
            break;
        case 14:
            slice_from_s(z, 3, s_21); /* <-, line 77 */
            break;
    }
    return 1;
}

static int r_Step_3(struct SN_env * z) {
    int among_var;
    z->ket = z->c; /* [, line 82 */
    among_var = find_among_b(z, a_4, 7); /* substring, line 82 */
    if (!(among_var)) return 0;
    z->bra = z->c; /* ], line 82 */
    if (!r_R1(z)) return 0; /* call R1, line 82 */
    switch(among_var) {
        case 0: return 0;
        case 1:
            slice_from_s(z, 2, s_22); /* <-, line 83 */
            break;
        case 2:
            slice_from_s(z, 2, s_23); /* <-, line 85 */
            break;
        case 3:
            slice_del(z); /* delete, line 87 */
            break;
    }
    return 1;
}

static int r_Step_4(struct SN_env * z) {
    int among_var;
    z->ket = z->c; /* [, line 92 */
    among_var = find_among_b(z, a_5, 19); /* substring, line 92 */
    if (!(among_var)) return 0;
    z->bra = z->c; /* ], line 92 */
    if (!r_R2(z)) return 0; /* call R2, line 92 */
    switch(among_var) {
        case 0: return 0;
        case 1:
            slice_del(z); /* delete, line 95 */
            break;
        case 2:
            {   int m = z->l - z->c; /* or, line 96 */
                if (!(eq_s_b(z, 1, s_24))) goto lab1;
                goto lab0;
            lab1:
                z->c = z->l - m;
                if (!(eq_s_b(z, 1, s_25))) return 0;
            }
        lab0:
            slice_del(z); /* delete, line 96 */
            break;
    }
    return 1;
}

static int r_Step_5a(struct SN_env * z) {
    z->ket = z->c; /* [, line 101 */
    if (!(eq_s_b(z, 1, s_26))) return 0;
    z->bra = z->c; /* ], line 101 */
    {   int m = z->l - z->c; /* or, line 102 */
        if (!r_R2(z)) goto lab1; /* call R2, line 102 */
        goto lab0;
    lab1:
        z->c = z->l - m;
        if (!r_R1(z)) return 0; /* call R1, line 102 */
        {   int m = z->l - z->c; /* not, line 102 */
            if (!r_shortv(z)) goto lab2; /* call shortv, line 102 */
            return 0;
        lab2:
            z->c = z->l - m;
        }
    }
lab0:
    slice_del(z); /* delete, line 103 */
    return 1;
}

static int r_Step_5b(struct SN_env * z) {
    z->ket = z->c; /* [, line 107 */
    if (!(eq_s_b(z, 1, s_27))) return 0;
    z->bra = z->c; /* ], line 107 */
    if (!r_R2(z)) return 0; /* call R2, line 108 */
    if (!(eq_s_b(z, 1, s_28))) return 0;
    slice_del(z); /* delete, line 109 */
    return 1;
}

extern int snowball_porter_stem(struct SN_env * z) {
    z->B[0] = 0; /* unset Y_found, line 115 */
    {   int c = z->c; /* do, line 116 */
        z->bra = z->c; /* [, line 116 */
        if (!(eq_s(z, 1, s_29))) goto lab0;
        z->ket = z->c; /* ], line 116 */
        slice_from_s(z, 1, s_30); /* <-, line 116 */
        z->B[0] = 1; /* set Y_found, line 116 */
    lab0:
        z->c = c;
    }
    {   int c = z->c; /* do, line 117 */
        while(1) { /* repeat, line 117 */
            int c = z->c;
            while(1) { /* goto, line 117 */
                int c = z->c;
                if (!(in_grouping(z, g_v, 97, 121))) goto lab3;
                z->bra = z->c; /* [, line 117 */
                if (!(eq_s(z, 1, s_31))) goto lab3;
                z->ket = z->c; /* ], line 117 */
                z->c = c;
                break;
            lab3:
                z->c = c;
                if (z->c >= z->l) goto lab2;
                z->c++;
            }
            slice_from_s(z, 1, s_32); /* <-, line 117 */
            z->B[0] = 1; /* set Y_found, line 117 */
            continue;
        lab2:
            z->c = c;
            break;
        }
    lab1:
        z->c = c;
    }
    z->I[0] = z->l;
    z->I[1] = z->l;
    {   int c = z->c; /* do, line 121 */
        while(1) { /* gopast, line 122 */
            if (!(in_grouping(z, g_v, 97, 121))) goto lab5;
            break;
        lab5:
            if (z->c >= z->l) goto lab4;
            z->c++;
        }
        while(1) { /* gopast, line 122 */
            if (!(out_grouping(z, g_v, 97, 121))) goto lab6;
            break;
        lab6:
            if (z->c >= z->l) goto lab4;
            z->c++;
        }
        z->I[0] = z->c; /* setmark p1, line 122 */
        while(1) { /* gopast, line 123 */
            if (!(in_grouping(z, g_v, 97, 121))) goto lab7;
            break;
        lab7:
            if (z->c >= z->l) goto lab4;
            z->c++;
        }
        while(1) { /* gopast, line 123 */
            if (!(out_grouping(z, g_v, 97, 121))) goto lab8;
            break;
        lab8:
            if (z->c >= z->l) goto lab4;
            z->c++;
        }
        z->I[1] = z->c; /* setmark p2, line 123 */
    lab4:
        z->c = c;
    }
    z->lb = z->c; z->c = z->l; /* backwards, line 126 */

    {   int m = z->l - z->c; /* do, line 127 */
        if (!r_Step_1a(z)) goto lab9; /* call Step_1a, line 127 */
    lab9:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; /* do, line 128 */
        if (!r_Step_1b(z)) goto lab10; /* call Step_1b, line 128 */
    lab10:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; /* do, line 129 */
        if (!r_Step_1c(z)) goto lab11; /* call Step_1c, line 129 */
    lab11:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; /* do, line 130 */
        if (!r_Step_2(z)) goto lab12; /* call Step_2, line 130 */
    lab12:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; /* do, line 131 */
        if (!r_Step_3(z)) goto lab13; /* call Step_3, line 131 */
    lab13:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; /* do, line 132 */
        if (!r_Step_4(z)) goto lab14; /* call Step_4, line 132 */
    lab14:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; /* do, line 133 */
        if (!r_Step_5a(z)) goto lab15; /* call Step_5a, line 133 */
    lab15:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; /* do, line 134 */
        if (!r_Step_5b(z)) goto lab16; /* call Step_5b, line 134 */
    lab16:
        z->c = z->l - m;
    }
    z->c = z->lb;
    {   int c = z->c; /* do, line 137 */
        if (!(z->B[0])) goto lab17; /* Boolean test Y_found, line 137 */
        while(1) { /* repeat, line 137 */
            int c = z->c;
            while(1) { /* goto, line 137 */
                int c = z->c;
                z->bra = z->c; /* [, line 137 */
                if (!(eq_s(z, 1, s_33))) goto lab19;
                z->ket = z->c; /* ], line 137 */
                z->c = c;
                break;
            lab19:
                z->c = c;
                if (z->c >= z->l) goto lab18;
                z->c++;
            }
            slice_from_s(z, 1, s_34); /* <-, line 137 */
            continue;
        lab18:
            z->c = c;
            break;
        }
    lab17:
        z->c = c;
    }
    return 1;
}

extern struct SN_env * snowball_porter_create_env(void) { return SN_create_env(0, 2, 1); }

extern void snowball_porter_close_env(struct SN_env * z) { SN_close_env(z); }

