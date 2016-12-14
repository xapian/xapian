/* This file was generated automatically by the Snowball to ISO C++ compiler */
/* http://snowballstem.org/ */

#include <config.h>
#include <limits.h>
#include "french.h"

static const symbol s_pool[] = {
#define s_0_0 0
'c', 'o', 'l',
#define s_0_1 3
'p', 'a', 'r',
#define s_0_2 6
't', 'a', 'p',
#define s_1_1 9
'I',
#define s_1_2 10
'U',
#define s_1_3 11
'Y',
#define s_2_0 12
'i', 'q', 'U',
#define s_2_1 15
'a', 'b', 'l',
#define s_2_2 18
'I', 0xC3, 0xA8, 'r',
#define s_2_3 22
'i', 0xC3, 0xA8, 'r',
#define s_2_4 26
'e', 'u', 's',
#define s_2_5 29
'i', 'v',
#define s_3_0 31
'i', 'c',
#define s_3_1 33
'a', 'b', 'i', 'l',
#define s_3_2 37
'i', 'v',
#define s_4_0 s_4_15
#define s_4_1 s_4_16
#define s_4_2 s_4_17
#define s_4_3 s_4_18
#define s_4_4 s_4_19
#define s_4_5 s_4_20
#define s_4_6 s_4_21
#define s_4_7 s_4_22
#define s_4_8 s_4_23
#define s_4_9 s_4_24
#define s_4_10 s_4_25
#define s_4_11 s_4_26
#define s_4_12 s_4_27
#define s_4_13 s_4_28
#define s_4_14 s_4_29
#define s_4_15 39
'i', 'q', 'U', 'e', 's',
#define s_4_16 44
'a', 't', 'r', 'i', 'c', 'e', 's',
#define s_4_17 51
'a', 'n', 'c', 'e', 's',
#define s_4_18 56
'e', 'n', 'c', 'e', 's',
#define s_4_19 61
'l', 'o', 'g', 'i', 'e', 's',
#define s_4_20 67
'a', 'b', 'l', 'e', 's',
#define s_4_21 72
'i', 's', 'm', 'e', 's',
#define s_4_22 77
'e', 'u', 's', 'e', 's',
#define s_4_23 82
'i', 's', 't', 'e', 's',
#define s_4_24 87
'i', 'v', 'e', 's',
#define s_4_25 91
'i', 'f', 's',
#define s_4_26 94
'u', 's', 'i', 'o', 'n', 's',
#define s_4_27 100
'a', 't', 'i', 'o', 'n', 's',
#define s_4_28 106
'u', 't', 'i', 'o', 'n', 's',
#define s_4_29 112
'a', 't', 'e', 'u', 'r', 's',
#define s_4_30 (s_4_31 + 1)
#define s_4_31 (s_4_32 + 3)
#define s_4_32 118
'i', 's', 's', 'e', 'm', 'e', 'n', 't', 's',
#define s_4_33 127
'i', 't', 0xC3, 0xA9, 's',
#define s_4_34 s_4_30
#define s_4_35 s_4_31
#define s_4_36 s_4_32
#define s_4_37 132
'a', 'm', 'm', 'e', 'n', 't',
#define s_4_38 138
'e', 'm', 'm', 'e', 'n', 't',
#define s_4_39 (s_4_40 + 1)
#define s_4_40 144
'e', 'a', 'u', 'x',
#define s_4_41 148
'e', 'u', 'x',
#define s_4_42 s_4_33
#define s_5_0 s_5_5
#define s_5_1 s_5_8
#define s_5_2 s_5_10
#define s_5_3 s_5_11
#define s_5_4 s_5_0
#define s_5_5 s_5_14
#define s_5_6 s_5_0
#define s_5_7 151
'i', 'r', 'a', 's',
#define s_5_8 155
'i', 'e', 's',
#define s_5_9 158
0xC3, 0xAE, 'm', 'e', 's',
#define s_5_10 163
'i', 's', 's', 'e', 's',
#define s_5_11 168
'i', 's', 's', 'a', 'n', 't', 'e', 's',
#define s_5_12 176
0xC3, 0xAE, 't', 'e', 's',
#define s_5_13 s_5_2
#define s_5_14 181
'i', 'r', 'a', 'i', 's',
#define s_5_15 186
'i', 's', 's', 'a', 'i', 's',
#define s_5_16 192
'i', 'r', 'i', 'o', 'n', 's',
#define s_5_17 198
'i', 's', 's', 'i', 'o', 'n', 's',
#define s_5_18 205
'i', 'r', 'o', 'n', 's',
#define s_5_19 210
'i', 's', 's', 'o', 'n', 's',
#define s_5_20 216
'i', 's', 's', 'a', 'n', 't', 's',
#define s_5_21 (s_5_22 + 3)
#define s_5_22 223
'i', 'r', 'a', 'i', 't',
#define s_5_23 228
'i', 's', 's', 'a', 'i', 't',
#define s_5_24 s_5_3
#define s_5_25 234
'i', 'r', 'a', 'I', 'e', 'n', 't',
#define s_5_26 241
'i', 's', 's', 'a', 'I', 'e', 'n', 't',
#define s_5_27 249
'i', 'r', 'e', 'n', 't',
#define s_5_28 254
'i', 's', 's', 'e', 'n', 't',
#define s_5_29 260
'i', 'r', 'o', 'n', 't',
#define s_5_30 s_5_12
#define s_5_31 265
'i', 'r', 'i', 'e', 'z',
#define s_5_32 270
'i', 's', 's', 'i', 'e', 'z',
#define s_5_33 276
'i', 'r', 'e', 'z',
#define s_5_34 280
'i', 's', 's', 'e', 'z',
#define s_6_0 (s_6_1 + 2)
#define s_6_1 s_6_6
#define s_6_2 s_6_11
#define s_6_3 s_6_12
#define s_6_4 s_6_14
#define s_6_5 (s_6_6 + 2)
#define s_6_6 s_6_16
#define s_6_7 s_6_1
#define s_6_8 s_6_2
#define s_6_9 285
'e', 'r', 'a', 's',
#define s_6_10 289
0xC3, 0xA2, 'm', 'e', 's',
#define s_6_11 294
'a', 's', 's', 'e', 's',
#define s_6_12 299
'a', 'n', 't', 'e', 's',
#define s_6_13 304
0xC3, 0xA2, 't', 'e', 's',
#define s_6_14 309
0xC3, 0xA9, 'e', 's',
#define s_6_15 (s_6_16 + 2)
#define s_6_16 313
'e', 'r', 'a', 'i', 's',
#define s_6_17 (s_6_18 + 2)
#define s_6_18 318
'e', 'r', 'i', 'o', 'n', 's',
#define s_6_19 324
'a', 's', 's', 'i', 'o', 'n', 's',
#define s_6_20 331
'e', 'r', 'o', 'n', 's',
#define s_6_21 336
'a', 'n', 't', 's',
#define s_6_22 340
0xC3, 0xA9, 's',
#define s_6_23 (s_6_24 + 2)
#define s_6_24 343
'e', 'r', 'a', 'i', 't',
#define s_6_25 s_6_3
#define s_6_26 (s_6_27 + 2)
#define s_6_27 348
'e', 'r', 'a', 'I', 'e', 'n', 't',
#define s_6_28 355
0xC3, 0xA8, 'r', 'e', 'n', 't',
#define s_6_29 361
'a', 's', 's', 'e', 'n', 't',
#define s_6_30 367
'e', 'r', 'o', 'n', 't',
#define s_6_31 s_6_13
#define s_6_32 (s_6_33 + 1)
#define s_6_33 (s_6_34 + 2)
#define s_6_34 372
'e', 'r', 'i', 'e', 'z',
#define s_6_35 377
'a', 's', 's', 'i', 'e', 'z',
#define s_6_36 383
'e', 'r', 'e', 'z',
#define s_6_37 s_6_4
#define s_7_0 (s_7_1 + 4)
#define s_7_1 387
'I', 0xC3, 0xA8, 'r', 'e',
#define s_7_2 392
'i', 0xC3, 0xA8, 'r', 'e',
#define s_7_3 397
'i', 'o', 'n',
#define s_7_4 400
'I', 'e', 'r',
#define s_7_5 403
'i', 'e', 'r',
#define s_7_6 406
0xC3, 0xAB,
#define s_8_0 408
'e', 'l', 'l',
#define s_8_1 411
'e', 'i', 'l', 'l',
#define s_8_2 415
'e', 'n', 'n',
#define s_8_3 418
'o', 'n', 'n',
#define s_8_4 421
'e', 't', 't',
};


static const struct among a_0[3] =
{
/*  0 */ { 3, s_0_0, -1, -1},
/*  1 */ { 3, s_0_1, -1, -1},
/*  2 */ { 3, s_0_2, -1, -1}
};


static const struct among a_1[4] =
{
/*  0 */ { 0, 0, -1, 4},
/*  1 */ { 1, s_1_1, 0, 1},
/*  2 */ { 1, s_1_2, 0, 2},
/*  3 */ { 1, s_1_3, 0, 3}
};


static const struct among a_2[6] =
{
/*  0 */ { 3, s_2_0, -1, 3},
/*  1 */ { 3, s_2_1, -1, 3},
/*  2 */ { 4, s_2_2, -1, 4},
/*  3 */ { 4, s_2_3, -1, 4},
/*  4 */ { 3, s_2_4, -1, 2},
/*  5 */ { 2, s_2_5, -1, 1}
};


static const struct among a_3[3] =
{
/*  0 */ { 2, s_3_0, -1, 2},
/*  1 */ { 4, s_3_1, -1, 1},
/*  2 */ { 2, s_3_2, -1, 3}
};


static const struct among a_4[43] =
{
/*  0 */ { 4, s_4_0, -1, 1},
/*  1 */ { 6, s_4_1, -1, 2},
/*  2 */ { 4, s_4_2, -1, 1},
/*  3 */ { 4, s_4_3, -1, 5},
/*  4 */ { 5, s_4_4, -1, 3},
/*  5 */ { 4, s_4_5, -1, 1},
/*  6 */ { 4, s_4_6, -1, 1},
/*  7 */ { 4, s_4_7, -1, 11},
/*  8 */ { 4, s_4_8, -1, 1},
/*  9 */ { 3, s_4_9, -1, 8},
/* 10 */ { 2, s_4_10, -1, 8},
/* 11 */ { 5, s_4_11, -1, 4},
/* 12 */ { 5, s_4_12, -1, 2},
/* 13 */ { 5, s_4_13, -1, 4},
/* 14 */ { 5, s_4_14, -1, 2},
/* 15 */ { 5, s_4_15, -1, 1},
/* 16 */ { 7, s_4_16, -1, 2},
/* 17 */ { 5, s_4_17, -1, 1},
/* 18 */ { 5, s_4_18, -1, 5},
/* 19 */ { 6, s_4_19, -1, 3},
/* 20 */ { 5, s_4_20, -1, 1},
/* 21 */ { 5, s_4_21, -1, 1},
/* 22 */ { 5, s_4_22, -1, 11},
/* 23 */ { 5, s_4_23, -1, 1},
/* 24 */ { 4, s_4_24, -1, 8},
/* 25 */ { 3, s_4_25, -1, 8},
/* 26 */ { 6, s_4_26, -1, 4},
/* 27 */ { 6, s_4_27, -1, 2},
/* 28 */ { 6, s_4_28, -1, 4},
/* 29 */ { 6, s_4_29, -1, 2},
/* 30 */ { 5, s_4_30, -1, 15},
/* 31 */ { 6, s_4_31, 30, 6},
/* 32 */ { 9, s_4_32, 31, 12},
/* 33 */ { 5, s_4_33, -1, 7},
/* 34 */ { 4, s_4_34, -1, 15},
/* 35 */ { 5, s_4_35, 34, 6},
/* 36 */ { 8, s_4_36, 35, 12},
/* 37 */ { 6, s_4_37, 34, 13},
/* 38 */ { 6, s_4_38, 34, 14},
/* 39 */ { 3, s_4_39, -1, 10},
/* 40 */ { 4, s_4_40, 39, 9},
/* 41 */ { 3, s_4_41, -1, 1},
/* 42 */ { 4, s_4_42, -1, 7}
};


static const struct among a_5[35] =
{
/*  0 */ { 3, s_5_0, -1, 1},
/*  1 */ { 2, s_5_1, -1, 1},
/*  2 */ { 4, s_5_2, -1, 1},
/*  3 */ { 7, s_5_3, -1, 1},
/*  4 */ { 1, s_5_4, -1, 1},
/*  5 */ { 4, s_5_5, 4, 1},
/*  6 */ { 2, s_5_6, -1, 1},
/*  7 */ { 4, s_5_7, -1, 1},
/*  8 */ { 3, s_5_8, -1, 1},
/*  9 */ { 5, s_5_9, -1, 1},
/* 10 */ { 5, s_5_10, -1, 1},
/* 11 */ { 8, s_5_11, -1, 1},
/* 12 */ { 5, s_5_12, -1, 1},
/* 13 */ { 2, s_5_13, -1, 1},
/* 14 */ { 5, s_5_14, 13, 1},
/* 15 */ { 6, s_5_15, 13, 1},
/* 16 */ { 6, s_5_16, -1, 1},
/* 17 */ { 7, s_5_17, -1, 1},
/* 18 */ { 5, s_5_18, -1, 1},
/* 19 */ { 6, s_5_19, -1, 1},
/* 20 */ { 7, s_5_20, -1, 1},
/* 21 */ { 2, s_5_21, -1, 1},
/* 22 */ { 5, s_5_22, 21, 1},
/* 23 */ { 6, s_5_23, 21, 1},
/* 24 */ { 6, s_5_24, -1, 1},
/* 25 */ { 7, s_5_25, -1, 1},
/* 26 */ { 8, s_5_26, -1, 1},
/* 27 */ { 5, s_5_27, -1, 1},
/* 28 */ { 6, s_5_28, -1, 1},
/* 29 */ { 5, s_5_29, -1, 1},
/* 30 */ { 3, s_5_30, -1, 1},
/* 31 */ { 5, s_5_31, -1, 1},
/* 32 */ { 6, s_5_32, -1, 1},
/* 33 */ { 4, s_5_33, -1, 1},
/* 34 */ { 5, s_5_34, -1, 1}
};


static const struct among a_6[38] =
{
/*  0 */ { 1, s_6_0, -1, 3},
/*  1 */ { 3, s_6_1, 0, 2},
/*  2 */ { 4, s_6_2, -1, 3},
/*  3 */ { 4, s_6_3, -1, 3},
/*  4 */ { 3, s_6_4, -1, 2},
/*  5 */ { 2, s_6_5, -1, 3},
/*  6 */ { 4, s_6_6, 5, 2},
/*  7 */ { 2, s_6_7, -1, 2},
/*  8 */ { 2, s_6_8, -1, 3},
/*  9 */ { 4, s_6_9, 8, 2},
/* 10 */ { 5, s_6_10, -1, 3},
/* 11 */ { 5, s_6_11, -1, 3},
/* 12 */ { 5, s_6_12, -1, 3},
/* 13 */ { 5, s_6_13, -1, 3},
/* 14 */ { 4, s_6_14, -1, 2},
/* 15 */ { 3, s_6_15, -1, 3},
/* 16 */ { 5, s_6_16, 15, 2},
/* 17 */ { 4, s_6_17, -1, 1},
/* 18 */ { 6, s_6_18, 17, 2},
/* 19 */ { 7, s_6_19, 17, 3},
/* 20 */ { 5, s_6_20, -1, 2},
/* 21 */ { 4, s_6_21, -1, 3},
/* 22 */ { 3, s_6_22, -1, 2},
/* 23 */ { 3, s_6_23, -1, 3},
/* 24 */ { 5, s_6_24, 23, 2},
/* 25 */ { 3, s_6_25, -1, 3},
/* 26 */ { 5, s_6_26, -1, 3},
/* 27 */ { 7, s_6_27, 26, 2},
/* 28 */ { 6, s_6_28, -1, 2},
/* 29 */ { 6, s_6_29, -1, 3},
/* 30 */ { 5, s_6_30, -1, 2},
/* 31 */ { 3, s_6_31, -1, 3},
/* 32 */ { 2, s_6_32, -1, 2},
/* 33 */ { 3, s_6_33, 32, 2},
/* 34 */ { 5, s_6_34, 33, 2},
/* 35 */ { 6, s_6_35, 33, 3},
/* 36 */ { 4, s_6_36, 32, 2},
/* 37 */ { 2, s_6_37, -1, 2}
};


static const struct among a_7[7] =
{
/*  0 */ { 1, s_7_0, -1, 3},
/*  1 */ { 5, s_7_1, 0, 2},
/*  2 */ { 5, s_7_2, 0, 2},
/*  3 */ { 3, s_7_3, -1, 1},
/*  4 */ { 3, s_7_4, -1, 2},
/*  5 */ { 3, s_7_5, -1, 2},
/*  6 */ { 2, s_7_6, -1, 4}
};


static const struct among a_8[5] =
{
/*  0 */ { 3, s_8_0, -1, -1},
/*  1 */ { 4, s_8_1, -1, -1},
/*  2 */ { 3, s_8_2, -1, -1},
/*  3 */ { 3, s_8_3, -1, -1},
/*  4 */ { 3, s_8_4, -1, -1}
};

static const unsigned char g_v[] = { 17, 65, 16, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 130, 103, 8, 5 };

static const unsigned char g_keep_with_s[] = { 1, 65, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128 };

static const symbol s_0[] = { 'U' };
static const symbol s_1[] = { 'I' };
static const symbol s_2[] = { 'Y' };
static const symbol s_3[] = { 'Y' };
static const symbol s_4[] = { 'U' };
static const symbol s_5[] = { 'i' };
static const symbol s_6[] = { 'u' };
static const symbol s_7[] = { 'y' };
static const symbol s_8[] = { 'i', 'c' };
static const symbol s_9[] = { 'i', 'q', 'U' };
static const symbol s_10[] = { 'l', 'o', 'g' };
static const symbol s_11[] = { 'u' };
static const symbol s_12[] = { 'e', 'n', 't' };
static const symbol s_13[] = { 'a', 't' };
static const symbol s_14[] = { 'e', 'u', 'x' };
static const symbol s_15[] = { 'i' };
static const symbol s_16[] = { 'a', 'b', 'l' };
static const symbol s_17[] = { 'i', 'q', 'U' };
static const symbol s_18[] = { 'a', 't' };
static const symbol s_19[] = { 'i', 'c' };
static const symbol s_20[] = { 'i', 'q', 'U' };
static const symbol s_21[] = { 'e', 'a', 'u' };
static const symbol s_22[] = { 'a', 'l' };
static const symbol s_23[] = { 'e', 'u', 'x' };
static const symbol s_24[] = { 'a', 'n', 't' };
static const symbol s_25[] = { 'e', 'n', 't' };
static const symbol s_26[] = { 'i' };
static const symbol s_27[] = { 'g', 'u' };
static const symbol s_28[] = { 0xC3, 0xA9 };
static const symbol s_29[] = { 0xC3, 0xA8 };
static const symbol s_30[] = { 'e' };
static const symbol s_31[] = { 'i' };
static const symbol s_32[] = { 0xC3, 0xA7 };
static const symbol s_33[] = { 'c' };

int Xapian::InternalStemFrench::r_prelude() { /* forwardmode */
    while(1) { /* repeat, line 40 */
        int c1 = c;
        while(1) { /* goto, line 40 */
            int c2 = c;
            {   int c3 = c; /* or, line 46 */
                if (in_grouping_U(g_v, 97, 251, 0)) goto lab3; /* grouping v, line 42 */
                bra = c; /* [, line 42 */
                {   int c4 = c; /* or, line 42 */
                    if (c == l || p[c] != 'u') goto lab5; /* literal, line 42 */
                    c++;
                    ket = c; /* ], line 42 */
                    if (in_grouping_U(g_v, 97, 251, 0)) goto lab5; /* grouping v, line 42 */
                    {   int ret = slice_from_s(1, s_0); /* <-, line 42 */
                        if (ret < 0) return ret;
                    }
                    goto lab4;
                lab5:
                    c = c4;
                    if (c == l || p[c] != 'i') goto lab6; /* literal, line 43 */
                    c++;
                    ket = c; /* ], line 43 */
                    if (in_grouping_U(g_v, 97, 251, 0)) goto lab6; /* grouping v, line 43 */
                    {   int ret = slice_from_s(1, s_1); /* <-, line 43 */
                        if (ret < 0) return ret;
                    }
                    goto lab4;
                lab6:
                    c = c4;
                    if (c == l || p[c] != 'y') goto lab3; /* literal, line 44 */
                    c++;
                    ket = c; /* ], line 44 */
                    {   int ret = slice_from_s(1, s_2); /* <-, line 44 */
                        if (ret < 0) return ret;
                    }
                }
            lab4:
                goto lab2;
            lab3:
                c = c3;
                bra = c; /* [, line 47 */
                if (c == l || p[c] != 'y') goto lab7; /* literal, line 47 */
                c++;
                ket = c; /* ], line 47 */
                if (in_grouping_U(g_v, 97, 251, 0)) goto lab7; /* grouping v, line 47 */
                {   int ret = slice_from_s(1, s_3); /* <-, line 47 */
                    if (ret < 0) return ret;
                }
                goto lab2;
            lab7:
                c = c3;
                if (c == l || p[c] != 'q') goto lab1; /* literal, line 49 */
                c++;
                bra = c; /* [, line 49 */
                if (c == l || p[c] != 'u') goto lab1; /* literal, line 49 */
                c++;
                ket = c; /* ], line 49 */
                {   int ret = slice_from_s(1, s_4); /* <-, line 49 */
                    if (ret < 0) return ret;
                }
            }
        lab2:
            c = c2;
            break;
        lab1:
            c = c2;
            {   int ret = skip_utf8(p, c, 0, l, 1);
                if (ret < 0) goto lab0;
                c = ret; /* goto, line 40 */
            }
        }
        continue;
    lab0:
        c = c1;
        break;
    }
    return 1;
}

int Xapian::InternalStemFrench::r_mark_regions() { /* forwardmode */
    I_pV = l; /* $pV = <integer expression>, line 54 */
    I_p1 = l; /* $p1 = <integer expression>, line 55 */
    I_p2 = l; /* $p2 = <integer expression>, line 56 */
    {   int c1 = c; /* do, line 58 */
        {   int c2 = c; /* or, line 60 */
            if (in_grouping_U(g_v, 97, 251, 0)) goto lab2; /* grouping v, line 59 */
            if (in_grouping_U(g_v, 97, 251, 0)) goto lab2; /* grouping v, line 59 */
            {   int ret = skip_utf8(p, c, 0, l, 1);
                if (ret < 0) goto lab2;
                c = ret; /* next, line 59 */
            }
            goto lab1;
        lab2:
            c = c2;
            if (c + 2 >= l || p[c + 2] >> 5 != 3 || !((331776 >> (p[c + 2] & 0x1f)) & 1)) goto lab3; /* among, line 61 */
            if (!(find_among(s_pool, a_0, 3, 0, 0))) goto lab3;
            goto lab1;
        lab3:
            c = c2;
            {   int ret = skip_utf8(p, c, 0, l, 1);
                if (ret < 0) goto lab0;
                c = ret; /* next, line 68 */
            }
            {    /* gopast */ /* grouping v, line 68 */
                int ret = out_grouping_U(g_v, 97, 251, 1);
                if (ret < 0) goto lab0;
                c += ret;
            }
        }
    lab1:
        I_pV = c; /* setmark pV, line 69 */
    lab0:
        c = c1;
    }
    {   int c3 = c; /* do, line 71 */
        {    /* gopast */ /* grouping v, line 72 */
            int ret = out_grouping_U(g_v, 97, 251, 1);
            if (ret < 0) goto lab4;
            c += ret;
        }
        {    /* gopast */ /* non v, line 72 */
            int ret = in_grouping_U(g_v, 97, 251, 1);
            if (ret < 0) goto lab4;
            c += ret;
        }
        I_p1 = c; /* setmark p1, line 72 */
        {    /* gopast */ /* grouping v, line 73 */
            int ret = out_grouping_U(g_v, 97, 251, 1);
            if (ret < 0) goto lab4;
            c += ret;
        }
        {    /* gopast */ /* non v, line 73 */
            int ret = in_grouping_U(g_v, 97, 251, 1);
            if (ret < 0) goto lab4;
            c += ret;
        }
        I_p2 = c; /* setmark p2, line 73 */
    lab4:
        c = c3;
    }
    return 1;
}

int Xapian::InternalStemFrench::r_postlude() { /* forwardmode */
    int among_var;
    while(1) { /* repeat, line 77 */
        int c1 = c;
        bra = c; /* [, line 79 */
        if (c >= l || p[c + 0] >> 5 != 2 || !((35652096 >> (p[c + 0] & 0x1f)) & 1)) among_var = 4; else /* substring, line 79 */
        among_var = find_among(s_pool, a_1, 4, 0, 0);
        if (!(among_var)) goto lab0;
        ket = c; /* ], line 79 */
        switch (among_var) { /* among, line 79 */
            case 0: goto lab0;
            case 1:
                {   int ret = slice_from_s(1, s_5); /* <-, line 80 */
                    if (ret < 0) return ret;
                }
                break;
            case 2:
                {   int ret = slice_from_s(1, s_6); /* <-, line 81 */
                    if (ret < 0) return ret;
                }
                break;
            case 3:
                {   int ret = slice_from_s(1, s_7); /* <-, line 82 */
                    if (ret < 0) return ret;
                }
                break;
            case 4:
                {   int ret = skip_utf8(p, c, 0, l, 1);
                    if (ret < 0) goto lab0;
                    c = ret; /* next, line 83 */
                }
                break;
        }
        continue;
    lab0:
        c = c1;
        break;
    }
    return 1;
}

int Xapian::InternalStemFrench::r_RV() { /* backwardmode */
    if (!(I_pV <= c)) return 0; /* $pV <= <integer expression>, line 89 */
    return 1;
}

int Xapian::InternalStemFrench::r_R1() { /* backwardmode */
    if (!(I_p1 <= c)) return 0; /* $p1 <= <integer expression>, line 90 */
    return 1;
}

int Xapian::InternalStemFrench::r_R2() { /* backwardmode */
    if (!(I_p2 <= c)) return 0; /* $p2 <= <integer expression>, line 91 */
    return 1;
}

int Xapian::InternalStemFrench::r_standard_suffix() { /* backwardmode */
    int among_var;
    ket = c; /* [, line 94 */
    among_var = find_among_b(s_pool, a_4, 43, 0, 0); /* substring, line 94 */
    if (!(among_var)) return 0;
    bra = c; /* ], line 94 */
    switch (among_var) { /* among, line 94 */
        case 0: return 0;
        case 1:
            {   int ret = r_R2(); /* call R2, line 98 */
                if (ret <= 0) return ret;
            }
            if (slice_del() == -1) return -1; /* delete, line 98 */
            break;
        case 2:
            {   int ret = r_R2(); /* call R2, line 101 */
                if (ret <= 0) return ret;
            }
            if (slice_del() == -1) return -1; /* delete, line 101 */
            {   int m1 = l - c; /*(void)m1*/; /* try, line 102 */
                ket = c; /* [, line 102 */
                if (!(eq_s_b(2, s_8))) { c = l - m1; goto lab0; } /* literal, line 102 */
                bra = c; /* ], line 102 */
                {   int m2 = l - c; /*(void)m2*/; /* or, line 102 */
                    {   int ret = r_R2(); /* call R2, line 102 */
                        if (ret == 0) goto lab2;
                        if (ret < 0) return ret;
                    }
                    if (slice_del() == -1) return -1; /* delete, line 102 */
                    goto lab1;
                lab2:
                    c = l - m2;
                    {   int ret = slice_from_s(3, s_9); /* <-, line 102 */
                        if (ret < 0) return ret;
                    }
                }
            lab1:
            lab0:
                ;
            }
            break;
        case 3:
            {   int ret = r_R2(); /* call R2, line 106 */
                if (ret <= 0) return ret;
            }
            {   int ret = slice_from_s(3, s_10); /* <-, line 106 */
                if (ret < 0) return ret;
            }
            break;
        case 4:
            {   int ret = r_R2(); /* call R2, line 109 */
                if (ret <= 0) return ret;
            }
            {   int ret = slice_from_s(1, s_11); /* <-, line 109 */
                if (ret < 0) return ret;
            }
            break;
        case 5:
            {   int ret = r_R2(); /* call R2, line 112 */
                if (ret <= 0) return ret;
            }
            {   int ret = slice_from_s(3, s_12); /* <-, line 112 */
                if (ret < 0) return ret;
            }
            break;
        case 6:
            {   int ret = r_RV(); /* call RV, line 116 */
                if (ret <= 0) return ret;
            }
            if (slice_del() == -1) return -1; /* delete, line 116 */
            {   int m3 = l - c; /*(void)m3*/; /* try, line 117 */
                ket = c; /* [, line 118 */
                among_var = find_among_b(s_pool, a_2, 6, 0, 0); /* substring, line 118 */
                if (!(among_var)) { c = l - m3; goto lab3; }
                bra = c; /* ], line 118 */
                switch (among_var) { /* among, line 118 */
                    case 0: { c = l - m3; goto lab3; }
                    case 1:
                        {   int ret = r_R2(); /* call R2, line 119 */
                            if (ret == 0) { c = l - m3; goto lab3; }
                            if (ret < 0) return ret;
                        }
                        if (slice_del() == -1) return -1; /* delete, line 119 */
                        ket = c; /* [, line 119 */
                        if (!(eq_s_b(2, s_13))) { c = l - m3; goto lab3; } /* literal, line 119 */
                        bra = c; /* ], line 119 */
                        {   int ret = r_R2(); /* call R2, line 119 */
                            if (ret == 0) { c = l - m3; goto lab3; }
                            if (ret < 0) return ret;
                        }
                        if (slice_del() == -1) return -1; /* delete, line 119 */
                        break;
                    case 2:
                        {   int m4 = l - c; /*(void)m4*/; /* or, line 120 */
                            {   int ret = r_R2(); /* call R2, line 120 */
                                if (ret == 0) goto lab5;
                                if (ret < 0) return ret;
                            }
                            if (slice_del() == -1) return -1; /* delete, line 120 */
                            goto lab4;
                        lab5:
                            c = l - m4;
                            {   int ret = r_R1(); /* call R1, line 120 */
                                if (ret == 0) { c = l - m3; goto lab3; }
                                if (ret < 0) return ret;
                            }
                            {   int ret = slice_from_s(3, s_14); /* <-, line 120 */
                                if (ret < 0) return ret;
                            }
                        }
                    lab4:
                        break;
                    case 3:
                        {   int ret = r_R2(); /* call R2, line 122 */
                            if (ret == 0) { c = l - m3; goto lab3; }
                            if (ret < 0) return ret;
                        }
                        if (slice_del() == -1) return -1; /* delete, line 122 */
                        break;
                    case 4:
                        {   int ret = r_RV(); /* call RV, line 124 */
                            if (ret == 0) { c = l - m3; goto lab3; }
                            if (ret < 0) return ret;
                        }
                        {   int ret = slice_from_s(1, s_15); /* <-, line 124 */
                            if (ret < 0) return ret;
                        }
                        break;
                }
            lab3:
                ;
            }
            break;
        case 7:
            {   int ret = r_R2(); /* call R2, line 131 */
                if (ret <= 0) return ret;
            }
            if (slice_del() == -1) return -1; /* delete, line 131 */
            {   int m5 = l - c; /*(void)m5*/; /* try, line 132 */
                ket = c; /* [, line 133 */
                if (c - 1 <= lb || p[c - 1] >> 5 != 3 || !((4198408 >> (p[c - 1] & 0x1f)) & 1)) { c = l - m5; goto lab6; } /* substring, line 133 */
                among_var = find_among_b(s_pool, a_3, 3, 0, 0);
                if (!(among_var)) { c = l - m5; goto lab6; }
                bra = c; /* ], line 133 */
                switch (among_var) { /* among, line 133 */
                    case 0: { c = l - m5; goto lab6; }
                    case 1:
                        {   int m6 = l - c; /*(void)m6*/; /* or, line 134 */
                            {   int ret = r_R2(); /* call R2, line 134 */
                                if (ret == 0) goto lab8;
                                if (ret < 0) return ret;
                            }
                            if (slice_del() == -1) return -1; /* delete, line 134 */
                            goto lab7;
                        lab8:
                            c = l - m6;
                            {   int ret = slice_from_s(3, s_16); /* <-, line 134 */
                                if (ret < 0) return ret;
                            }
                        }
                    lab7:
                        break;
                    case 2:
                        {   int m7 = l - c; /*(void)m7*/; /* or, line 135 */
                            {   int ret = r_R2(); /* call R2, line 135 */
                                if (ret == 0) goto lab10;
                                if (ret < 0) return ret;
                            }
                            if (slice_del() == -1) return -1; /* delete, line 135 */
                            goto lab9;
                        lab10:
                            c = l - m7;
                            {   int ret = slice_from_s(3, s_17); /* <-, line 135 */
                                if (ret < 0) return ret;
                            }
                        }
                    lab9:
                        break;
                    case 3:
                        {   int ret = r_R2(); /* call R2, line 136 */
                            if (ret == 0) { c = l - m5; goto lab6; }
                            if (ret < 0) return ret;
                        }
                        if (slice_del() == -1) return -1; /* delete, line 136 */
                        break;
                }
            lab6:
                ;
            }
            break;
        case 8:
            {   int ret = r_R2(); /* call R2, line 143 */
                if (ret <= 0) return ret;
            }
            if (slice_del() == -1) return -1; /* delete, line 143 */
            {   int m8 = l - c; /*(void)m8*/; /* try, line 144 */
                ket = c; /* [, line 144 */
                if (!(eq_s_b(2, s_18))) { c = l - m8; goto lab11; } /* literal, line 144 */
                bra = c; /* ], line 144 */
                {   int ret = r_R2(); /* call R2, line 144 */
                    if (ret == 0) { c = l - m8; goto lab11; }
                    if (ret < 0) return ret;
                }
                if (slice_del() == -1) return -1; /* delete, line 144 */
                ket = c; /* [, line 144 */
                if (!(eq_s_b(2, s_19))) { c = l - m8; goto lab11; } /* literal, line 144 */
                bra = c; /* ], line 144 */
                {   int m9 = l - c; /*(void)m9*/; /* or, line 144 */
                    {   int ret = r_R2(); /* call R2, line 144 */
                        if (ret == 0) goto lab13;
                        if (ret < 0) return ret;
                    }
                    if (slice_del() == -1) return -1; /* delete, line 144 */
                    goto lab12;
                lab13:
                    c = l - m9;
                    {   int ret = slice_from_s(3, s_20); /* <-, line 144 */
                        if (ret < 0) return ret;
                    }
                }
            lab12:
            lab11:
                ;
            }
            break;
        case 9:
            {   int ret = slice_from_s(3, s_21); /* <-, line 146 */
                if (ret < 0) return ret;
            }
            break;
        case 10:
            {   int ret = r_R1(); /* call R1, line 147 */
                if (ret <= 0) return ret;
            }
            {   int ret = slice_from_s(2, s_22); /* <-, line 147 */
                if (ret < 0) return ret;
            }
            break;
        case 11:
            {   int m10 = l - c; /*(void)m10*/; /* or, line 149 */
                {   int ret = r_R2(); /* call R2, line 149 */
                    if (ret == 0) goto lab15;
                    if (ret < 0) return ret;
                }
                if (slice_del() == -1) return -1; /* delete, line 149 */
                goto lab14;
            lab15:
                c = l - m10;
                {   int ret = r_R1(); /* call R1, line 149 */
                    if (ret <= 0) return ret;
                }
                {   int ret = slice_from_s(3, s_23); /* <-, line 149 */
                    if (ret < 0) return ret;
                }
            }
        lab14:
            break;
        case 12:
            {   int ret = r_R1(); /* call R1, line 152 */
                if (ret <= 0) return ret;
            }
            if (out_grouping_b_U(g_v, 97, 251, 0)) return 0; /* non v, line 152 */
            if (slice_del() == -1) return -1; /* delete, line 152 */
            break;
        case 13:
            {   int ret = r_RV(); /* call RV, line 157 */
                if (ret <= 0) return ret;
            }
            {   int ret = slice_from_s(3, s_24); /* <-, line 157 */
                if (ret < 0) return ret;
            }
            return 0; /* fail, line 157 */
            break;
        case 14:
            {   int ret = r_RV(); /* call RV, line 158 */
                if (ret <= 0) return ret;
            }
            {   int ret = slice_from_s(3, s_25); /* <-, line 158 */
                if (ret < 0) return ret;
            }
            return 0; /* fail, line 158 */
            break;
        case 15:
            {   int m_test11 = l - c; /* test, line 160 */
                if (in_grouping_b_U(g_v, 97, 251, 0)) return 0; /* grouping v, line 160 */
                {   int ret = r_RV(); /* call RV, line 160 */
                    if (ret <= 0) return ret;
                }
                c = l - m_test11;
            }
            if (slice_del() == -1) return -1; /* delete, line 160 */
            return 0; /* fail, line 160 */
            break;
    }
    return 1;
}

int Xapian::InternalStemFrench::r_i_verb_suffix() { /* backwardmode */
    int among_var;
    {   int m1 = l - c; /*(void)m1*/; /* setlimit, line 165 */
        int mlimit1;
        if (c < I_pV) return 0;
        c = I_pV; /* tomark, line 165 */
        mlimit1 = lb; lb = c;
        c = l - m1;
        ket = c; /* [, line 166 */
        if (c <= lb || p[c - 1] >> 5 != 3 || !((68944418 >> (p[c - 1] & 0x1f)) & 1)) { lb = mlimit1; return 0; } /* substring, line 166 */
        among_var = find_among_b(s_pool, a_5, 35, 0, 0);
        if (!(among_var)) { lb = mlimit1; return 0; }
        bra = c; /* ], line 166 */
        switch (among_var) { /* among, line 166 */
            case 0: { lb = mlimit1; return 0; }
            case 1:
                if (out_grouping_b_U(g_v, 97, 251, 0)) { lb = mlimit1; return 0; } /* non v, line 172 */
                if (slice_del() == -1) return -1; /* delete, line 172 */
                break;
        }
        lb = mlimit1;
    }
    return 1;
}

int Xapian::InternalStemFrench::r_verb_suffix() { /* backwardmode */
    int among_var;
    {   int m1 = l - c; /*(void)m1*/; /* setlimit, line 176 */
        int mlimit1;
        if (c < I_pV) return 0;
        c = I_pV; /* tomark, line 176 */
        mlimit1 = lb; lb = c;
        c = l - m1;
        ket = c; /* [, line 177 */
        among_var = find_among_b(s_pool, a_6, 38, 0, 0); /* substring, line 177 */
        if (!(among_var)) { lb = mlimit1; return 0; }
        bra = c; /* ], line 177 */
        switch (among_var) { /* among, line 177 */
            case 0: { lb = mlimit1; return 0; }
            case 1:
                {   int ret = r_R2(); /* call R2, line 179 */
                    if (ret == 0) { lb = mlimit1; return 0; }
                    if (ret < 0) return ret;
                }
                if (slice_del() == -1) return -1; /* delete, line 179 */
                break;
            case 2:
                if (slice_del() == -1) return -1; /* delete, line 187 */
                break;
            case 3:
                if (slice_del() == -1) return -1; /* delete, line 192 */
                {   int m2 = l - c; /*(void)m2*/; /* try, line 193 */
                    ket = c; /* [, line 193 */
                    if (c <= lb || p[c - 1] != 'e') { c = l - m2; goto lab0; } /* literal, line 193 */
                    c--;
                    bra = c; /* ], line 193 */
                    if (slice_del() == -1) return -1; /* delete, line 193 */
                lab0:
                    ;
                }
                break;
        }
        lb = mlimit1;
    }
    return 1;
}

int Xapian::InternalStemFrench::r_residual_suffix() { /* backwardmode */
    int among_var;
    {   int m1 = l - c; /*(void)m1*/; /* try, line 201 */
        ket = c; /* [, line 201 */
        if (c <= lb || p[c - 1] != 's') { c = l - m1; goto lab0; } /* literal, line 201 */
        c--;
        bra = c; /* ], line 201 */
        {   int m_test2 = l - c; /* test, line 201 */
            if (out_grouping_b_U(g_keep_with_s, 97, 232, 0)) { c = l - m1; goto lab0; } /* non keep_with_s, line 201 */
            c = l - m_test2;
        }
        if (slice_del() == -1) return -1; /* delete, line 201 */
    lab0:
        ;
    }
    {   int m3 = l - c; /*(void)m3*/; /* setlimit, line 202 */
        int mlimit3;
        if (c < I_pV) return 0;
        c = I_pV; /* tomark, line 202 */
        mlimit3 = lb; lb = c;
        c = l - m3;
        ket = c; /* [, line 203 */
        among_var = find_among_b(s_pool, a_7, 7, 0, 0); /* substring, line 203 */
        if (!(among_var)) { lb = mlimit3; return 0; }
        bra = c; /* ], line 203 */
        switch (among_var) { /* among, line 203 */
            case 0: { lb = mlimit3; return 0; }
            case 1:
                {   int ret = r_R2(); /* call R2, line 204 */
                    if (ret == 0) { lb = mlimit3; return 0; }
                    if (ret < 0) return ret;
                }
                {   int m4 = l - c; /*(void)m4*/; /* or, line 204 */
                    if (c <= lb || p[c - 1] != 's') goto lab2; /* literal, line 204 */
                    c--;
                    goto lab1;
                lab2:
                    c = l - m4;
                    if (c <= lb || p[c - 1] != 't') { lb = mlimit3; return 0; } /* literal, line 204 */
                    c--;
                }
            lab1:
                if (slice_del() == -1) return -1; /* delete, line 204 */
                break;
            case 2:
                {   int ret = slice_from_s(1, s_26); /* <-, line 206 */
                    if (ret < 0) return ret;
                }
                break;
            case 3:
                if (slice_del() == -1) return -1; /* delete, line 207 */
                break;
            case 4:
                if (!(eq_s_b(2, s_27))) { lb = mlimit3; return 0; } /* literal, line 208 */
                if (slice_del() == -1) return -1; /* delete, line 208 */
                break;
        }
        lb = mlimit3;
    }
    return 1;
}

int Xapian::InternalStemFrench::r_un_double() { /* backwardmode */
    {   int m_test1 = l - c; /* test, line 214 */
        if (c - 2 <= lb || p[c - 1] >> 5 != 3 || !((1069056 >> (p[c - 1] & 0x1f)) & 1)) return 0; /* among, line 214 */
        if (!(find_among_b(s_pool, a_8, 5, 0, 0))) return 0;
        c = l - m_test1;
    }
    ket = c; /* [, line 214 */
    {   int ret = skip_utf8(p, c, lb, 0, -1);
        if (ret < 0) return 0;
        c = ret; /* next, line 214 */
    }
    bra = c; /* ], line 214 */
    if (slice_del() == -1) return -1; /* delete, line 214 */
    return 1;
}

int Xapian::InternalStemFrench::r_un_accent() { /* backwardmode */
    {   int i = 1;
        while(1) { /* atleast, line 218 */
            if (out_grouping_b_U(g_v, 97, 251, 0)) goto lab0; /* non v, line 218 */
            i--;
            continue;
        lab0:
            break;
        }
        if (i > 0) return 0;
    }
    ket = c; /* [, line 219 */
    {   int m1 = l - c; /*(void)m1*/; /* or, line 219 */
        if (!(eq_s_b(2, s_28))) goto lab2; /* literal, line 219 */
        goto lab1;
    lab2:
        c = l - m1;
        if (!(eq_s_b(2, s_29))) return 0; /* literal, line 219 */
    }
lab1:
    bra = c; /* ], line 219 */
    {   int ret = slice_from_s(1, s_30); /* <-, line 219 */
        if (ret < 0) return ret;
    }
    return 1;
}

int Xapian::InternalStemFrench::stem() { /* forwardmode */
    {   int c1 = c; /* do, line 225 */
        {   int ret = r_prelude(); /* call prelude, line 225 */
            if (ret == 0) goto lab0;
            if (ret < 0) return ret;
        }
    lab0:
        c = c1;
    }
    {   int c2 = c; /* do, line 226 */
        {   int ret = r_mark_regions(); /* call mark_regions, line 226 */
            if (ret == 0) goto lab1;
            if (ret < 0) return ret;
        }
    lab1:
        c = c2;
    }
    lb = c; c = l; /* backwards, line 227 */

    {   int m3 = l - c; /*(void)m3*/; /* do, line 229 */
        {   int m4 = l - c; /*(void)m4*/; /* or, line 239 */
            {   int m5 = l - c; /*(void)m5*/; /* and, line 235 */
                {   int m6 = l - c; /*(void)m6*/; /* or, line 231 */
                    {   int ret = r_standard_suffix(); /* call standard_suffix, line 231 */
                        if (ret == 0) goto lab6;
                        if (ret < 0) return ret;
                    }
                    goto lab5;
                lab6:
                    c = l - m6;
                    {   int ret = r_i_verb_suffix(); /* call i_verb_suffix, line 232 */
                        if (ret == 0) goto lab7;
                        if (ret < 0) return ret;
                    }
                    goto lab5;
                lab7:
                    c = l - m6;
                    {   int ret = r_verb_suffix(); /* call verb_suffix, line 233 */
                        if (ret == 0) goto lab4;
                        if (ret < 0) return ret;
                    }
                }
            lab5:
                c = l - m5;
                {   int m7 = l - c; /*(void)m7*/; /* try, line 236 */
                    ket = c; /* [, line 236 */
                    {   int m8 = l - c; /*(void)m8*/; /* or, line 236 */
                        if (c <= lb || p[c - 1] != 'Y') goto lab10; /* literal, line 236 */
                        c--;
                        bra = c; /* ], line 236 */
                        {   int ret = slice_from_s(1, s_31); /* <-, line 236 */
                            if (ret < 0) return ret;
                        }
                        goto lab9;
                    lab10:
                        c = l - m8;
                        if (!(eq_s_b(2, s_32))) { c = l - m7; goto lab8; } /* literal, line 237 */
                        bra = c; /* ], line 237 */
                        {   int ret = slice_from_s(1, s_33); /* <-, line 237 */
                            if (ret < 0) return ret;
                        }
                    }
                lab9:
                lab8:
                    ;
                }
            }
            goto lab3;
        lab4:
            c = l - m4;
            {   int ret = r_residual_suffix(); /* call residual_suffix, line 240 */
                if (ret == 0) goto lab2;
                if (ret < 0) return ret;
            }
        }
    lab3:
    lab2:
        c = l - m3;
    }
    {   int m9 = l - c; /*(void)m9*/; /* do, line 245 */
        {   int ret = r_un_double(); /* call un_double, line 245 */
            if (ret == 0) goto lab11;
            if (ret < 0) return ret;
        }
    lab11:
        c = l - m9;
    }
    {   int m10 = l - c; /*(void)m10*/; /* do, line 246 */
        {   int ret = r_un_accent(); /* call un_accent, line 246 */
            if (ret == 0) goto lab12;
            if (ret < 0) return ret;
        }
    lab12:
        c = l - m10;
    }
    c = lb;
    {   int c11 = c; /* do, line 248 */
        {   int ret = r_postlude(); /* call postlude, line 248 */
            if (ret == 0) goto lab13;
            if (ret < 0) return ret;
        }
    lab13:
        c = c11;
    }
    return 1;
}

Xapian::InternalStemFrench::InternalStemFrench()
    : I_p2(0), I_p1(0), I_pV(0)
{
}

Xapian::InternalStemFrench::~InternalStemFrench()
{
}

std::string
Xapian::InternalStemFrench::get_description() const
{
    return "french";
}
