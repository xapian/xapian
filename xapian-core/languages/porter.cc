/* This file was generated automatically by the Snowball to ISO C++ compiler */
/* http://snowballstem.org/ */

#include <config.h>
#include <limits.h>
#include "porter.h"

#define s_0_0 (s_0_1 + 2)
static const symbol s_pool[] = {
#define s_0_1 0
'i', 'e', 's',
#define s_0_2 3
's', 's', 'e', 's',
#define s_0_3 s_0_2
#define s_1_1 7
'b', 'b',
#define s_1_2 9
'd', 'd',
#define s_1_3 11
'f', 'f',
#define s_1_4 13
'g', 'g',
#define s_1_5 15
'b', 'l',
#define s_1_6 17
'm', 'm',
#define s_1_7 19
'n', 'n',
#define s_1_8 21
'p', 'p',
#define s_1_9 23
'r', 'r',
#define s_1_10 25
'a', 't',
#define s_1_11 27
't', 't',
#define s_1_12 29
'i', 'z',
#define s_2_0 (s_2_1 + 1)
#define s_2_1 31
'e', 'e', 'd',
#define s_2_2 34
'i', 'n', 'g',
#define s_3_0 37
'a', 'n', 'c', 'i',
#define s_3_1 41
'e', 'n', 'c', 'i',
#define s_3_2 45
'a', 'b', 'l', 'i',
#define s_3_3 49
'e', 'l', 'i',
#define s_3_4 52
'a', 'l', 'l', 'i',
#define s_3_5 56
'o', 'u', 's', 'l', 'i',
#define s_3_6 61
'e', 'n', 't', 'l', 'i',
#define s_3_7 66
'a', 'l', 'i', 't', 'i',
#define s_3_8 71
'b', 'i', 'l', 'i', 't', 'i',
#define s_3_9 77
'i', 'v', 'i', 't', 'i',
#define s_3_10 (s_3_11 + 1)
#define s_3_11 82
'a', 't', 'i', 'o', 'n', 'a', 'l',
#define s_3_12 89
'a', 'l', 'i', 's', 'm',
#define s_3_13 s_3_11
#define s_3_14 94
'i', 'z', 'a', 't', 'i', 'o', 'n',
#define s_3_15 101
'i', 'z', 'e', 'r',
#define s_3_16 105
'a', 't', 'o', 'r',
#define s_3_17 109
'i', 'v', 'e', 'n', 'e', 's', 's',
#define s_3_18 116
'f', 'u', 'l', 'n', 'e', 's', 's',
#define s_3_19 123
'o', 'u', 's', 'n', 'e', 's', 's',
#define s_4_0 130
'i', 'c', 'a', 't', 'e',
#define s_4_1 135
'a', 't', 'i', 'v', 'e',
#define s_4_2 140
'a', 'l', 'i', 'z', 'e',
#define s_4_3 145
'i', 'c', 'i', 't', 'i',
#define s_4_4 150
'i', 'c', 'a', 'l',
#define s_4_5 154
'f', 'u', 'l',
#define s_4_6 157
'n', 'e', 's', 's',
#define s_5_0 161
'i', 'c',
#define s_5_1 163
'a', 'n', 'c', 'e',
#define s_5_2 167
'e', 'n', 'c', 'e',
#define s_5_3 171
'a', 'b', 'l', 'e',
#define s_5_4 175
'i', 'b', 'l', 'e',
#define s_5_5 179
'a', 't', 'e',
#define s_5_6 182
'i', 'v', 'e',
#define s_5_7 185
'i', 'z', 'e',
#define s_5_8 188
'i', 't', 'i',
#define s_5_9 191
'a', 'l',
#define s_5_10 193
'i', 's', 'm',
#define s_5_11 196
'i', 'o', 'n',
#define s_5_12 199
'e', 'r',
#define s_5_13 201
'o', 'u', 's',
#define s_5_14 204
'a', 'n', 't',
#define s_5_15 (s_5_16 + 1)
#define s_5_16 (s_5_17 + 1)
#define s_5_17 207
'e', 'm', 'e', 'n', 't',
#define s_5_18 s_5_13
};


static const struct among a_0[4] =
{
/*  0 */ { 1, s_0_0, -1, 3},
/*  1 */ { 3, s_0_1, 0, 2},
/*  2 */ { 4, s_0_2, 0, 1},
/*  3 */ { 2, s_0_3, 0, -1}
};


static const struct among a_1[13] =
{
/*  0 */ { 0, 0, -1, 3},
/*  1 */ { 2, s_1_1, 0, 2},
/*  2 */ { 2, s_1_2, 0, 2},
/*  3 */ { 2, s_1_3, 0, 2},
/*  4 */ { 2, s_1_4, 0, 2},
/*  5 */ { 2, s_1_5, 0, 1},
/*  6 */ { 2, s_1_6, 0, 2},
/*  7 */ { 2, s_1_7, 0, 2},
/*  8 */ { 2, s_1_8, 0, 2},
/*  9 */ { 2, s_1_9, 0, 2},
/* 10 */ { 2, s_1_10, 0, 1},
/* 11 */ { 2, s_1_11, 0, 2},
/* 12 */ { 2, s_1_12, 0, 1}
};


static const struct among a_2[3] =
{
/*  0 */ { 2, s_2_0, -1, 2},
/*  1 */ { 3, s_2_1, 0, 1},
/*  2 */ { 3, s_2_2, -1, 2}
};


static const struct among a_3[20] =
{
/*  0 */ { 4, s_3_0, -1, 3},
/*  1 */ { 4, s_3_1, -1, 2},
/*  2 */ { 4, s_3_2, -1, 4},
/*  3 */ { 3, s_3_3, -1, 6},
/*  4 */ { 4, s_3_4, -1, 9},
/*  5 */ { 5, s_3_5, -1, 12},
/*  6 */ { 5, s_3_6, -1, 5},
/*  7 */ { 5, s_3_7, -1, 10},
/*  8 */ { 6, s_3_8, -1, 14},
/*  9 */ { 5, s_3_9, -1, 13},
/* 10 */ { 6, s_3_10, -1, 1},
/* 11 */ { 7, s_3_11, 10, 8},
/* 12 */ { 5, s_3_12, -1, 10},
/* 13 */ { 5, s_3_13, -1, 8},
/* 14 */ { 7, s_3_14, 13, 7},
/* 15 */ { 4, s_3_15, -1, 7},
/* 16 */ { 4, s_3_16, -1, 8},
/* 17 */ { 7, s_3_17, -1, 13},
/* 18 */ { 7, s_3_18, -1, 11},
/* 19 */ { 7, s_3_19, -1, 12}
};


static const struct among a_4[7] =
{
/*  0 */ { 5, s_4_0, -1, 2},
/*  1 */ { 5, s_4_1, -1, 3},
/*  2 */ { 5, s_4_2, -1, 1},
/*  3 */ { 5, s_4_3, -1, 2},
/*  4 */ { 4, s_4_4, -1, 2},
/*  5 */ { 3, s_4_5, -1, 3},
/*  6 */ { 4, s_4_6, -1, 3}
};


static const struct among a_5[19] =
{
/*  0 */ { 2, s_5_0, -1, 1},
/*  1 */ { 4, s_5_1, -1, 1},
/*  2 */ { 4, s_5_2, -1, 1},
/*  3 */ { 4, s_5_3, -1, 1},
/*  4 */ { 4, s_5_4, -1, 1},
/*  5 */ { 3, s_5_5, -1, 1},
/*  6 */ { 3, s_5_6, -1, 1},
/*  7 */ { 3, s_5_7, -1, 1},
/*  8 */ { 3, s_5_8, -1, 1},
/*  9 */ { 2, s_5_9, -1, 1},
/* 10 */ { 3, s_5_10, -1, 1},
/* 11 */ { 3, s_5_11, -1, 2},
/* 12 */ { 2, s_5_12, -1, 1},
/* 13 */ { 3, s_5_13, -1, 1},
/* 14 */ { 3, s_5_14, -1, 1},
/* 15 */ { 3, s_5_15, -1, 1},
/* 16 */ { 4, s_5_16, 15, 1},
/* 17 */ { 5, s_5_17, 16, 1},
/* 18 */ { 2, s_5_18, -1, 1}
};

static const unsigned char g_v[] = { 17, 65, 16, 1 };

static const unsigned char g_v_WXY[] = { 1, 17, 65, 208, 1 };

static const symbol s_0[] = { 's', 's' };
static const symbol s_1[] = { 'i' };
static const symbol s_2[] = { 'e', 'e' };
static const symbol s_3[] = { 'e' };
static const symbol s_4[] = { 'e' };
static const symbol s_5[] = { 'i' };
static const symbol s_6[] = { 't', 'i', 'o', 'n' };
static const symbol s_7[] = { 'e', 'n', 'c', 'e' };
static const symbol s_8[] = { 'a', 'n', 'c', 'e' };
static const symbol s_9[] = { 'a', 'b', 'l', 'e' };
static const symbol s_10[] = { 'e', 'n', 't' };
static const symbol s_11[] = { 'e' };
static const symbol s_12[] = { 'i', 'z', 'e' };
static const symbol s_13[] = { 'a', 't', 'e' };
static const symbol s_14[] = { 'a', 'l' };
static const symbol s_15[] = { 'a', 'l' };
static const symbol s_16[] = { 'f', 'u', 'l' };
static const symbol s_17[] = { 'o', 'u', 's' };
static const symbol s_18[] = { 'i', 'v', 'e' };
static const symbol s_19[] = { 'b', 'l', 'e' };
static const symbol s_20[] = { 'a', 'l' };
static const symbol s_21[] = { 'i', 'c' };
static const symbol s_22[] = { 'Y' };
static const symbol s_23[] = { 'Y' };
static const symbol s_24[] = { 'y' };

int Xapian::InternalStemPorter::r_shortv() { /* backwardmode */
    if (out_grouping_b_U(g_v_WXY, 89, 121, 0)) return 0; /* non v_WXY, line 21 */
    if (in_grouping_b_U(g_v, 97, 121, 0)) return 0; /* grouping v, line 21 */
    if (out_grouping_b_U(g_v, 97, 121, 0)) return 0; /* non v, line 21 */
    return 1;
}

int Xapian::InternalStemPorter::r_R1() { /* backwardmode */
    if (!(I_p1 <= c)) return 0; /* $p1 <= <integer expression>, line 23 */
    return 1;
}

int Xapian::InternalStemPorter::r_R2() { /* backwardmode */
    if (!(I_p2 <= c)) return 0; /* $p2 <= <integer expression>, line 24 */
    return 1;
}

int Xapian::InternalStemPorter::r_Step_1a() { /* backwardmode */
    int among_var;
    ket = c; /* [, line 27 */
    if (c <= lb || p[c - 1] != 115) return 0; /* substring, line 27 */
    among_var = find_among_b(s_pool, a_0, 4, 0, 0);
    if (!(among_var)) return 0;
    bra = c; /* ], line 27 */
    switch (among_var) { /* among, line 27 */
        case 0: return 0;
        case 1:
            {   int ret = slice_from_s(2, s_0); /* <-, line 28 */
                if (ret < 0) return ret;
            }
            break;
        case 2:
            {   int ret = slice_from_s(1, s_1); /* <-, line 29 */
                if (ret < 0) return ret;
            }
            break;
        case 3:
            if (slice_del() == -1) return -1; /* delete, line 31 */
            break;
    }
    return 1;
}

int Xapian::InternalStemPorter::r_Step_1b() { /* backwardmode */
    int among_var;
    ket = c; /* [, line 36 */
    if (c - 1 <= lb || (p[c - 1] != 100 && p[c - 1] != 103)) return 0; /* substring, line 36 */
    among_var = find_among_b(s_pool, a_2, 3, 0, 0);
    if (!(among_var)) return 0;
    bra = c; /* ], line 36 */
    switch (among_var) { /* among, line 36 */
        case 0: return 0;
        case 1:
            {   int ret = r_R1(); /* call R1, line 37 */
                if (ret <= 0) return ret;
            }
            {   int ret = slice_from_s(2, s_2); /* <-, line 37 */
                if (ret < 0) return ret;
            }
            break;
        case 2:
            {   int m_test1 = l - c; /* test, line 40 */
                {    /* gopast */ /* grouping v, line 40 */
                    int ret = out_grouping_b_U(g_v, 97, 121, 1);
                    if (ret < 0) return 0;
                    c -= ret;
                }
                c = l - m_test1;
            }
            if (slice_del() == -1) return -1; /* delete, line 40 */
            {   int m_test2 = l - c; /* test, line 41 */
                if (c - 1 <= lb || p[c - 1] >> 5 != 3 || !((68514004 >> (p[c - 1] & 0x1f)) & 1)) among_var = 3; else /* substring, line 41 */
                among_var = find_among_b(s_pool, a_1, 13, 0, 0);
                if (!(among_var)) return 0;
                c = l - m_test2;
            }
            switch (among_var) { /* among, line 41 */
                case 0: return 0;
                case 1:
                    {   int saved_c = c;
                        insert_s(c, c, 1, s_3); /* <+, line 43 */
                        c = saved_c;
                    }
                    break;
                case 2:
                    ket = c; /* [, line 46 */
                    {   int ret = skip_utf8(p, c, lb, 0, -1);
                        if (ret < 0) return 0;
                        c = ret; /* next, line 46 */
                    }
                    bra = c; /* ], line 46 */
                    if (slice_del() == -1) return -1; /* delete, line 46 */
                    break;
                case 3:
                    if (c != I_p1) return 0; /* atmark, line 47 */
                    {   int m_test3 = l - c; /* test, line 47 */
                        {   int ret = r_shortv(); /* call shortv, line 47 */
                            if (ret <= 0) return ret;
                        }
                        c = l - m_test3;
                    }
                    {   int saved_c = c;
                        insert_s(c, c, 1, s_4); /* <+, line 47 */
                        c = saved_c;
                    }
                    break;
            }
            break;
    }
    return 1;
}

int Xapian::InternalStemPorter::r_Step_1c() { /* backwardmode */
    ket = c; /* [, line 54 */
    {   int m1 = l - c; /*(void)m1*/; /* or, line 54 */
        if (c <= lb || p[c - 1] != 'y') goto lab1; /* literal, line 54 */
        c--;
        goto lab0;
    lab1:
        c = l - m1;
        if (c <= lb || p[c - 1] != 'Y') return 0; /* literal, line 54 */
        c--;
    }
lab0:
    bra = c; /* ], line 54 */
    {    /* gopast */ /* grouping v, line 55 */
        int ret = out_grouping_b_U(g_v, 97, 121, 1);
        if (ret < 0) return 0;
        c -= ret;
    }
    {   int ret = slice_from_s(1, s_5); /* <-, line 56 */
        if (ret < 0) return ret;
    }
    return 1;
}

int Xapian::InternalStemPorter::r_Step_2() { /* backwardmode */
    int among_var;
    ket = c; /* [, line 60 */
    if (c - 2 <= lb || p[c - 1] >> 5 != 3 || !((815616 >> (p[c - 1] & 0x1f)) & 1)) return 0; /* substring, line 60 */
    among_var = find_among_b(s_pool, a_3, 20, 0, 0);
    if (!(among_var)) return 0;
    bra = c; /* ], line 60 */
    {   int ret = r_R1(); /* call R1, line 60 */
        if (ret <= 0) return ret;
    }
    switch (among_var) { /* among, line 60 */
        case 0: return 0;
        case 1:
            {   int ret = slice_from_s(4, s_6); /* <-, line 61 */
                if (ret < 0) return ret;
            }
            break;
        case 2:
            {   int ret = slice_from_s(4, s_7); /* <-, line 62 */
                if (ret < 0) return ret;
            }
            break;
        case 3:
            {   int ret = slice_from_s(4, s_8); /* <-, line 63 */
                if (ret < 0) return ret;
            }
            break;
        case 4:
            {   int ret = slice_from_s(4, s_9); /* <-, line 64 */
                if (ret < 0) return ret;
            }
            break;
        case 5:
            {   int ret = slice_from_s(3, s_10); /* <-, line 65 */
                if (ret < 0) return ret;
            }
            break;
        case 6:
            {   int ret = slice_from_s(1, s_11); /* <-, line 66 */
                if (ret < 0) return ret;
            }
            break;
        case 7:
            {   int ret = slice_from_s(3, s_12); /* <-, line 68 */
                if (ret < 0) return ret;
            }
            break;
        case 8:
            {   int ret = slice_from_s(3, s_13); /* <-, line 70 */
                if (ret < 0) return ret;
            }
            break;
        case 9:
            {   int ret = slice_from_s(2, s_14); /* <-, line 71 */
                if (ret < 0) return ret;
            }
            break;
        case 10:
            {   int ret = slice_from_s(2, s_15); /* <-, line 73 */
                if (ret < 0) return ret;
            }
            break;
        case 11:
            {   int ret = slice_from_s(3, s_16); /* <-, line 74 */
                if (ret < 0) return ret;
            }
            break;
        case 12:
            {   int ret = slice_from_s(3, s_17); /* <-, line 76 */
                if (ret < 0) return ret;
            }
            break;
        case 13:
            {   int ret = slice_from_s(3, s_18); /* <-, line 78 */
                if (ret < 0) return ret;
            }
            break;
        case 14:
            {   int ret = slice_from_s(3, s_19); /* <-, line 79 */
                if (ret < 0) return ret;
            }
            break;
    }
    return 1;
}

int Xapian::InternalStemPorter::r_Step_3() { /* backwardmode */
    int among_var;
    ket = c; /* [, line 84 */
    if (c - 2 <= lb || p[c - 1] >> 5 != 3 || !((528928 >> (p[c - 1] & 0x1f)) & 1)) return 0; /* substring, line 84 */
    among_var = find_among_b(s_pool, a_4, 7, 0, 0);
    if (!(among_var)) return 0;
    bra = c; /* ], line 84 */
    {   int ret = r_R1(); /* call R1, line 84 */
        if (ret <= 0) return ret;
    }
    switch (among_var) { /* among, line 84 */
        case 0: return 0;
        case 1:
            {   int ret = slice_from_s(2, s_20); /* <-, line 85 */
                if (ret < 0) return ret;
            }
            break;
        case 2:
            {   int ret = slice_from_s(2, s_21); /* <-, line 87 */
                if (ret < 0) return ret;
            }
            break;
        case 3:
            if (slice_del() == -1) return -1; /* delete, line 89 */
            break;
    }
    return 1;
}

int Xapian::InternalStemPorter::r_Step_4() { /* backwardmode */
    int among_var;
    ket = c; /* [, line 94 */
    if (c - 1 <= lb || p[c - 1] >> 5 != 3 || !((3961384 >> (p[c - 1] & 0x1f)) & 1)) return 0; /* substring, line 94 */
    among_var = find_among_b(s_pool, a_5, 19, 0, 0);
    if (!(among_var)) return 0;
    bra = c; /* ], line 94 */
    {   int ret = r_R2(); /* call R2, line 94 */
        if (ret <= 0) return ret;
    }
    switch (among_var) { /* among, line 94 */
        case 0: return 0;
        case 1:
            if (slice_del() == -1) return -1; /* delete, line 97 */
            break;
        case 2:
            {   int m1 = l - c; /*(void)m1*/; /* or, line 98 */
                if (c <= lb || p[c - 1] != 's') goto lab1; /* literal, line 98 */
                c--;
                goto lab0;
            lab1:
                c = l - m1;
                if (c <= lb || p[c - 1] != 't') return 0; /* literal, line 98 */
                c--;
            }
        lab0:
            if (slice_del() == -1) return -1; /* delete, line 98 */
            break;
    }
    return 1;
}

int Xapian::InternalStemPorter::r_Step_5a() { /* backwardmode */
    ket = c; /* [, line 103 */
    if (c <= lb || p[c - 1] != 'e') return 0; /* literal, line 103 */
    c--;
    bra = c; /* ], line 103 */
    {   int m1 = l - c; /*(void)m1*/; /* or, line 104 */
        {   int ret = r_R2(); /* call R2, line 104 */
            if (ret == 0) goto lab1;
            if (ret < 0) return ret;
        }
        goto lab0;
    lab1:
        c = l - m1;
        {   int ret = r_R1(); /* call R1, line 104 */
            if (ret <= 0) return ret;
        }
        {   int m2 = l - c; /*(void)m2*/; /* not, line 104 */
            {   int ret = r_shortv(); /* call shortv, line 104 */
                if (ret == 0) goto lab2;
                if (ret < 0) return ret;
            }
            return 0;
        lab2:
            c = l - m2;
        }
    }
lab0:
    if (slice_del() == -1) return -1; /* delete, line 105 */
    return 1;
}

int Xapian::InternalStemPorter::r_Step_5b() { /* backwardmode */
    ket = c; /* [, line 109 */
    if (c <= lb || p[c - 1] != 'l') return 0; /* literal, line 109 */
    c--;
    bra = c; /* ], line 109 */
    {   int ret = r_R2(); /* call R2, line 110 */
        if (ret <= 0) return ret;
    }
    if (c <= lb || p[c - 1] != 'l') return 0; /* literal, line 110 */
    c--;
    if (slice_del() == -1) return -1; /* delete, line 111 */
    return 1;
}

int Xapian::InternalStemPorter::stem() { /* forwardmode */
    B_Y_found = 0; /* unset Y_found, line 117 */
    {   int c1 = c; /* do, line 118 */
        bra = c; /* [, line 118 */
        if (c == l || p[c] != 'y') goto lab0; /* literal, line 118 */
        c++;
        ket = c; /* ], line 118 */
        {   int ret = slice_from_s(1, s_22); /* <-, line 118 */
            if (ret < 0) return ret;
        }
        B_Y_found = 1; /* set Y_found, line 118 */
    lab0:
        c = c1;
    }
    {   int c2 = c; /* do, line 119 */
        while(1) { /* repeat, line 119 */
            int c3 = c;
            while(1) { /* goto, line 119 */
                int c4 = c;
                if (in_grouping_U(g_v, 97, 121, 0)) goto lab3; /* grouping v, line 119 */
                bra = c; /* [, line 119 */
                if (c == l || p[c] != 'y') goto lab3; /* literal, line 119 */
                c++;
                ket = c; /* ], line 119 */
                c = c4;
                break;
            lab3:
                c = c4;
                {   int ret = skip_utf8(p, c, 0, l, 1);
                    if (ret < 0) goto lab2;
                    c = ret; /* goto, line 119 */
                }
            }
            {   int ret = slice_from_s(1, s_23); /* <-, line 119 */
                if (ret < 0) return ret;
            }
            B_Y_found = 1; /* set Y_found, line 119 */
            continue;
        lab2:
            c = c3;
            break;
        }
        c = c2;
    }
    I_p1 = l; /* $p1 = <integer expression>, line 121 */
    I_p2 = l; /* $p2 = <integer expression>, line 122 */
    {   int c5 = c; /* do, line 123 */
        {    /* gopast */ /* grouping v, line 124 */
            int ret = out_grouping_U(g_v, 97, 121, 1);
            if (ret < 0) goto lab4;
            c += ret;
        }
        {    /* gopast */ /* non v, line 124 */
            int ret = in_grouping_U(g_v, 97, 121, 1);
            if (ret < 0) goto lab4;
            c += ret;
        }
        I_p1 = c; /* setmark p1, line 124 */
        {    /* gopast */ /* grouping v, line 125 */
            int ret = out_grouping_U(g_v, 97, 121, 1);
            if (ret < 0) goto lab4;
            c += ret;
        }
        {    /* gopast */ /* non v, line 125 */
            int ret = in_grouping_U(g_v, 97, 121, 1);
            if (ret < 0) goto lab4;
            c += ret;
        }
        I_p2 = c; /* setmark p2, line 125 */
    lab4:
        c = c5;
    }
    lb = c; c = l; /* backwards, line 128 */

    {   int m6 = l - c; /*(void)m6*/; /* do, line 129 */
        {   int ret = r_Step_1a(); /* call Step_1a, line 129 */
            if (ret == 0) goto lab5;
            if (ret < 0) return ret;
        }
    lab5:
        c = l - m6;
    }
    {   int m7 = l - c; /*(void)m7*/; /* do, line 130 */
        {   int ret = r_Step_1b(); /* call Step_1b, line 130 */
            if (ret == 0) goto lab6;
            if (ret < 0) return ret;
        }
    lab6:
        c = l - m7;
    }
    {   int m8 = l - c; /*(void)m8*/; /* do, line 131 */
        {   int ret = r_Step_1c(); /* call Step_1c, line 131 */
            if (ret == 0) goto lab7;
            if (ret < 0) return ret;
        }
    lab7:
        c = l - m8;
    }
    {   int m9 = l - c; /*(void)m9*/; /* do, line 132 */
        {   int ret = r_Step_2(); /* call Step_2, line 132 */
            if (ret == 0) goto lab8;
            if (ret < 0) return ret;
        }
    lab8:
        c = l - m9;
    }
    {   int m10 = l - c; /*(void)m10*/; /* do, line 133 */
        {   int ret = r_Step_3(); /* call Step_3, line 133 */
            if (ret == 0) goto lab9;
            if (ret < 0) return ret;
        }
    lab9:
        c = l - m10;
    }
    {   int m11 = l - c; /*(void)m11*/; /* do, line 134 */
        {   int ret = r_Step_4(); /* call Step_4, line 134 */
            if (ret == 0) goto lab10;
            if (ret < 0) return ret;
        }
    lab10:
        c = l - m11;
    }
    {   int m12 = l - c; /*(void)m12*/; /* do, line 135 */
        {   int ret = r_Step_5a(); /* call Step_5a, line 135 */
            if (ret == 0) goto lab11;
            if (ret < 0) return ret;
        }
    lab11:
        c = l - m12;
    }
    {   int m13 = l - c; /*(void)m13*/; /* do, line 136 */
        {   int ret = r_Step_5b(); /* call Step_5b, line 136 */
            if (ret == 0) goto lab12;
            if (ret < 0) return ret;
        }
    lab12:
        c = l - m13;
    }
    c = lb;
    {   int c14 = c; /* do, line 139 */
        if (!(B_Y_found)) goto lab13; /* Boolean test Y_found, line 139 */
        while(1) { /* repeat, line 139 */
            int c15 = c;
            while(1) { /* goto, line 139 */
                int c16 = c;
                bra = c; /* [, line 139 */
                if (c == l || p[c] != 'Y') goto lab15; /* literal, line 139 */
                c++;
                ket = c; /* ], line 139 */
                c = c16;
                break;
            lab15:
                c = c16;
                {   int ret = skip_utf8(p, c, 0, l, 1);
                    if (ret < 0) goto lab14;
                    c = ret; /* goto, line 139 */
                }
            }
            {   int ret = slice_from_s(1, s_24); /* <-, line 139 */
                if (ret < 0) return ret;
            }
            continue;
        lab14:
            c = c15;
            break;
        }
    lab13:
        c = c14;
    }
    return 1;
}

Xapian::InternalStemPorter::InternalStemPorter()
    : B_Y_found(0), I_p2(0), I_p1(0)
{
}

Xapian::InternalStemPorter::~InternalStemPorter()
{
}

std::string
Xapian::InternalStemPorter::get_description() const
{
    return "porter";
}
