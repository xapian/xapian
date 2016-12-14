/* This file was generated automatically by the Snowball to ISO C++ compiler */
/* http://snowballstem.org/ */

#include <config.h>
#include <limits.h>
#include "german2.h"

static const symbol s_pool[] = {
#define s_0_1 0
'a', 'e',
#define s_0_2 2
'o', 'e',
#define s_0_3 4
'q', 'u',
#define s_0_4 6
'u', 'e',
#define s_0_5 8
0xC3, 0x9F,
#define s_1_1 10
'U',
#define s_1_2 11
'Y',
#define s_1_3 12
0xC3, 0xA4,
#define s_1_4 14
0xC3, 0xB6,
#define s_1_5 16
0xC3, 0xBC,
#define s_2_0 s_2_1
#define s_2_1 18
'e', 'm',
#define s_2_2 20
'e', 'n',
#define s_2_3 22
'e', 'r', 'n',
#define s_2_4 s_2_3
#define s_2_5 (s_2_6 + 1)
#define s_2_6 25
'e', 's',
#define s_3_0 27
'e', 'n',
#define s_3_1 29
'e', 'r',
#define s_3_2 (s_3_3 + 1)
#define s_3_3 31
'e', 's', 't',
#define s_4_0 34
'i', 'g',
#define s_4_1 36
'l', 'i', 'c', 'h',
#define s_5_0 40
'e', 'n', 'd',
#define s_5_1 43
'i', 'g',
#define s_5_2 45
'u', 'n', 'g',
#define s_5_3 48
'l', 'i', 'c', 'h',
#define s_5_4 52
'i', 's', 'c', 'h',
#define s_5_5 56
'i', 'k',
#define s_5_6 58
'h', 'e', 'i', 't',
#define s_5_7 62
'k', 'e', 'i', 't',
};


static const struct among a_0[6] =
{
/*  0 */ { 0, 0, -1, 6},
/*  1 */ { 2, s_0_1, 0, 2},
/*  2 */ { 2, s_0_2, 0, 3},
/*  3 */ { 2, s_0_3, 0, 5},
/*  4 */ { 2, s_0_4, 0, 4},
/*  5 */ { 2, s_0_5, 0, 1}
};


static const struct among a_1[6] =
{
/*  0 */ { 0, 0, -1, 6},
/*  1 */ { 1, s_1_1, 0, 2},
/*  2 */ { 1, s_1_2, 0, 1},
/*  3 */ { 2, s_1_3, 0, 3},
/*  4 */ { 2, s_1_4, 0, 4},
/*  5 */ { 2, s_1_5, 0, 5}
};


static const struct among a_2[7] =
{
/*  0 */ { 1, s_2_0, -1, 2},
/*  1 */ { 2, s_2_1, -1, 1},
/*  2 */ { 2, s_2_2, -1, 2},
/*  3 */ { 3, s_2_3, -1, 1},
/*  4 */ { 2, s_2_4, -1, 1},
/*  5 */ { 1, s_2_5, -1, 3},
/*  6 */ { 2, s_2_6, 5, 2}
};


static const struct among a_3[4] =
{
/*  0 */ { 2, s_3_0, -1, 1},
/*  1 */ { 2, s_3_1, -1, 1},
/*  2 */ { 2, s_3_2, -1, 2},
/*  3 */ { 3, s_3_3, 2, 1}
};


static const struct among a_4[2] =
{
/*  0 */ { 2, s_4_0, -1, 1},
/*  1 */ { 4, s_4_1, -1, 1}
};


static const struct among a_5[8] =
{
/*  0 */ { 3, s_5_0, -1, 1},
/*  1 */ { 2, s_5_1, -1, 2},
/*  2 */ { 3, s_5_2, -1, 1},
/*  3 */ { 4, s_5_3, -1, 3},
/*  4 */ { 4, s_5_4, -1, 2},
/*  5 */ { 2, s_5_5, -1, 2},
/*  6 */ { 4, s_5_6, -1, 3},
/*  7 */ { 4, s_5_7, -1, 4}
};

static const unsigned char g_v[] = { 17, 65, 16, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 32, 8 };

static const unsigned char g_s_ending[] = { 117, 30, 5 };

static const unsigned char g_st_ending[] = { 117, 30, 4 };

static const symbol s_0[] = { 'U' };
static const symbol s_1[] = { 'Y' };
static const symbol s_2[] = { 's', 's' };
static const symbol s_3[] = { 0xC3, 0xA4 };
static const symbol s_4[] = { 0xC3, 0xB6 };
static const symbol s_5[] = { 0xC3, 0xBC };
static const symbol s_6[] = { 'y' };
static const symbol s_7[] = { 'u' };
static const symbol s_8[] = { 'a' };
static const symbol s_9[] = { 'o' };
static const symbol s_10[] = { 'u' };
static const symbol s_11[] = { 'n', 'i', 's' };
static const symbol s_12[] = { 'i', 'g' };
static const symbol s_13[] = { 'e', 'r' };
static const symbol s_14[] = { 'e', 'n' };

int Xapian::InternalStemGerman2::r_prelude() { /* forwardmode */
    int among_var;
    {   int c_test1 = c; /* test, line 36 */
        while(1) { /* repeat, line 36 */
            int c2 = c;
            while(1) { /* goto, line 36 */
                int c3 = c;
                if (in_grouping_U(g_v, 97, 252, 0)) goto lab1; /* grouping v, line 37 */
                bra = c; /* [, line 37 */
                {   int c4 = c; /* or, line 37 */
                    if (c == l || p[c] != 'u') goto lab3; /* literal, line 37 */
                    c++;
                    ket = c; /* ], line 37 */
                    if (in_grouping_U(g_v, 97, 252, 0)) goto lab3; /* grouping v, line 37 */
                    {   int ret = slice_from_s(1, s_0); /* <-, line 37 */
                        if (ret < 0) return ret;
                    }
                    goto lab2;
                lab3:
                    c = c4;
                    if (c == l || p[c] != 'y') goto lab1; /* literal, line 38 */
                    c++;
                    ket = c; /* ], line 38 */
                    if (in_grouping_U(g_v, 97, 252, 0)) goto lab1; /* grouping v, line 38 */
                    {   int ret = slice_from_s(1, s_1); /* <-, line 38 */
                        if (ret < 0) return ret;
                    }
                }
            lab2:
                c = c3;
                break;
            lab1:
                c = c3;
                {   int ret = skip_utf8(p, c, 0, l, 1);
                    if (ret < 0) goto lab0;
                    c = ret; /* goto, line 36 */
                }
            }
            continue;
        lab0:
            c = c2;
            break;
        }
        c = c_test1;
    }
    while(1) { /* repeat, line 41 */
        int c5 = c;
        bra = c; /* [, line 42 */
        among_var = find_among(s_pool, a_0, 6, 0, 0); /* substring, line 42 */
        if (!(among_var)) goto lab4;
        ket = c; /* ], line 42 */
        switch (among_var) { /* among, line 42 */
            case 0: goto lab4;
            case 1:
                {   int ret = slice_from_s(2, s_2); /* <-, line 43 */
                    if (ret < 0) return ret;
                }
                break;
            case 2:
                {   int ret = slice_from_s(2, s_3); /* <-, line 44 */
                    if (ret < 0) return ret;
                }
                break;
            case 3:
                {   int ret = slice_from_s(2, s_4); /* <-, line 45 */
                    if (ret < 0) return ret;
                }
                break;
            case 4:
                {   int ret = slice_from_s(2, s_5); /* <-, line 46 */
                    if (ret < 0) return ret;
                }
                break;
            case 5:
                {   int ret = skip_utf8(p, c, 0, l, + 2); /* hop, line 47 */
                    if (ret < 0) goto lab4;
                    c = ret;
                }
                break;
            case 6:
                {   int ret = skip_utf8(p, c, 0, l, 1);
                    if (ret < 0) goto lab4;
                    c = ret; /* next, line 48 */
                }
                break;
        }
        continue;
    lab4:
        c = c5;
        break;
    }
    return 1;
}

int Xapian::InternalStemGerman2::r_mark_regions() { /* forwardmode */
    I_p1 = l; /* $p1 = <integer expression>, line 56 */
    I_p2 = l; /* $p2 = <integer expression>, line 57 */
    {   int c_test1 = c; /* test, line 59 */
        {   int ret = skip_utf8(p, c, 0, l, + 3); /* hop, line 59 */
            if (ret < 0) return 0;
            c = ret;
        }
        I_x = c; /* setmark x, line 59 */
        c = c_test1;
    }
    {    /* gopast */ /* grouping v, line 61 */
        int ret = out_grouping_U(g_v, 97, 252, 1);
        if (ret < 0) return 0;
        c += ret;
    }
    {    /* gopast */ /* non v, line 61 */
        int ret = in_grouping_U(g_v, 97, 252, 1);
        if (ret < 0) return 0;
        c += ret;
    }
    I_p1 = c; /* setmark p1, line 61 */
    /* try, line 62 */
    if (!(I_p1 < I_x)) goto lab0; /* $p1 < <integer expression>, line 62 */
    I_p1 = I_x; /* $p1 = <integer expression>, line 62 */
lab0:
    {    /* gopast */ /* grouping v, line 63 */
        int ret = out_grouping_U(g_v, 97, 252, 1);
        if (ret < 0) return 0;
        c += ret;
    }
    {    /* gopast */ /* non v, line 63 */
        int ret = in_grouping_U(g_v, 97, 252, 1);
        if (ret < 0) return 0;
        c += ret;
    }
    I_p2 = c; /* setmark p2, line 63 */
    return 1;
}

int Xapian::InternalStemGerman2::r_postlude() { /* forwardmode */
    int among_var;
    while(1) { /* repeat, line 67 */
        int c1 = c;
        bra = c; /* [, line 69 */
        among_var = find_among(s_pool, a_1, 6, 0, 0); /* substring, line 69 */
        if (!(among_var)) goto lab0;
        ket = c; /* ], line 69 */
        switch (among_var) { /* among, line 69 */
            case 0: goto lab0;
            case 1:
                {   int ret = slice_from_s(1, s_6); /* <-, line 70 */
                    if (ret < 0) return ret;
                }
                break;
            case 2:
                {   int ret = slice_from_s(1, s_7); /* <-, line 71 */
                    if (ret < 0) return ret;
                }
                break;
            case 3:
                {   int ret = slice_from_s(1, s_8); /* <-, line 72 */
                    if (ret < 0) return ret;
                }
                break;
            case 4:
                {   int ret = slice_from_s(1, s_9); /* <-, line 73 */
                    if (ret < 0) return ret;
                }
                break;
            case 5:
                {   int ret = slice_from_s(1, s_10); /* <-, line 74 */
                    if (ret < 0) return ret;
                }
                break;
            case 6:
                {   int ret = skip_utf8(p, c, 0, l, 1);
                    if (ret < 0) goto lab0;
                    c = ret; /* next, line 75 */
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

int Xapian::InternalStemGerman2::r_R1() { /* backwardmode */
    if (!(I_p1 <= c)) return 0; /* $p1 <= <integer expression>, line 82 */
    return 1;
}

int Xapian::InternalStemGerman2::r_R2() { /* backwardmode */
    if (!(I_p2 <= c)) return 0; /* $p2 <= <integer expression>, line 83 */
    return 1;
}

int Xapian::InternalStemGerman2::r_standard_suffix() { /* backwardmode */
    int among_var;
    {   int m1 = l - c; /*(void)m1*/; /* do, line 86 */
        ket = c; /* [, line 87 */
        if (c <= lb || p[c - 1] >> 5 != 3 || !((811040 >> (p[c - 1] & 0x1f)) & 1)) goto lab0; /* substring, line 87 */
        among_var = find_among_b(s_pool, a_2, 7, 0, 0);
        if (!(among_var)) goto lab0;
        bra = c; /* ], line 87 */
        {   int ret = r_R1(); /* call R1, line 87 */
            if (ret == 0) goto lab0;
            if (ret < 0) return ret;
        }
        switch (among_var) { /* among, line 87 */
            case 0: goto lab0;
            case 1:
                if (slice_del() == -1) return -1; /* delete, line 89 */
                break;
            case 2:
                if (slice_del() == -1) return -1; /* delete, line 92 */
                {   int m2 = l - c; /*(void)m2*/; /* try, line 93 */
                    ket = c; /* [, line 93 */
                    if (c <= lb || p[c - 1] != 's') { c = l - m2; goto lab1; } /* literal, line 93 */
                    c--;
                    bra = c; /* ], line 93 */
                    if (!(eq_s_b(3, s_11))) { c = l - m2; goto lab1; } /* literal, line 93 */
                    if (slice_del() == -1) return -1; /* delete, line 93 */
                lab1:
                    ;
                }
                break;
            case 3:
                if (in_grouping_b_U(g_s_ending, 98, 116, 0)) goto lab0; /* grouping s_ending, line 96 */
                if (slice_del() == -1) return -1; /* delete, line 96 */
                break;
        }
    lab0:
        c = l - m1;
    }
    {   int m3 = l - c; /*(void)m3*/; /* do, line 100 */
        ket = c; /* [, line 101 */
        if (c - 1 <= lb || p[c - 1] >> 5 != 3 || !((1327104 >> (p[c - 1] & 0x1f)) & 1)) goto lab2; /* substring, line 101 */
        among_var = find_among_b(s_pool, a_3, 4, 0, 0);
        if (!(among_var)) goto lab2;
        bra = c; /* ], line 101 */
        {   int ret = r_R1(); /* call R1, line 101 */
            if (ret == 0) goto lab2;
            if (ret < 0) return ret;
        }
        switch (among_var) { /* among, line 101 */
            case 0: goto lab2;
            case 1:
                if (slice_del() == -1) return -1; /* delete, line 103 */
                break;
            case 2:
                if (in_grouping_b_U(g_st_ending, 98, 116, 0)) goto lab2; /* grouping st_ending, line 106 */
                {   int ret = skip_utf8(p, c, lb, l, - 3); /* hop, line 106 */
                    if (ret < 0) goto lab2;
                    c = ret;
                }
                if (slice_del() == -1) return -1; /* delete, line 106 */
                break;
        }
    lab2:
        c = l - m3;
    }
    {   int m4 = l - c; /*(void)m4*/; /* do, line 110 */
        ket = c; /* [, line 111 */
        if (c - 1 <= lb || p[c - 1] >> 5 != 3 || !((1051024 >> (p[c - 1] & 0x1f)) & 1)) goto lab3; /* substring, line 111 */
        among_var = find_among_b(s_pool, a_5, 8, 0, 0);
        if (!(among_var)) goto lab3;
        bra = c; /* ], line 111 */
        {   int ret = r_R2(); /* call R2, line 111 */
            if (ret == 0) goto lab3;
            if (ret < 0) return ret;
        }
        switch (among_var) { /* among, line 111 */
            case 0: goto lab3;
            case 1:
                if (slice_del() == -1) return -1; /* delete, line 113 */
                {   int m5 = l - c; /*(void)m5*/; /* try, line 114 */
                    ket = c; /* [, line 114 */
                    if (!(eq_s_b(2, s_12))) { c = l - m5; goto lab4; } /* literal, line 114 */
                    bra = c; /* ], line 114 */
                    {   int m6 = l - c; /*(void)m6*/; /* not, line 114 */
                        if (c <= lb || p[c - 1] != 'e') goto lab5; /* literal, line 114 */
                        c--;
                        { c = l - m5; goto lab4; }
                    lab5:
                        c = l - m6;
                    }
                    {   int ret = r_R2(); /* call R2, line 114 */
                        if (ret == 0) { c = l - m5; goto lab4; }
                        if (ret < 0) return ret;
                    }
                    if (slice_del() == -1) return -1; /* delete, line 114 */
                lab4:
                    ;
                }
                break;
            case 2:
                {   int m7 = l - c; /*(void)m7*/; /* not, line 117 */
                    if (c <= lb || p[c - 1] != 'e') goto lab6; /* literal, line 117 */
                    c--;
                    goto lab3;
                lab6:
                    c = l - m7;
                }
                if (slice_del() == -1) return -1; /* delete, line 117 */
                break;
            case 3:
                if (slice_del() == -1) return -1; /* delete, line 120 */
                {   int m8 = l - c; /*(void)m8*/; /* try, line 121 */
                    ket = c; /* [, line 122 */
                    {   int m9 = l - c; /*(void)m9*/; /* or, line 122 */
                        if (!(eq_s_b(2, s_13))) goto lab9; /* literal, line 122 */
                        goto lab8;
                    lab9:
                        c = l - m9;
                        if (!(eq_s_b(2, s_14))) { c = l - m8; goto lab7; } /* literal, line 122 */
                    }
                lab8:
                    bra = c; /* ], line 122 */
                    {   int ret = r_R1(); /* call R1, line 122 */
                        if (ret == 0) { c = l - m8; goto lab7; }
                        if (ret < 0) return ret;
                    }
                    if (slice_del() == -1) return -1; /* delete, line 122 */
                lab7:
                    ;
                }
                break;
            case 4:
                if (slice_del() == -1) return -1; /* delete, line 126 */
                {   int m10 = l - c; /*(void)m10*/; /* try, line 127 */
                    ket = c; /* [, line 128 */
                    if (c - 1 <= lb || (p[c - 1] != 103 && p[c - 1] != 104)) { c = l - m10; goto lab10; } /* substring, line 128 */
                    among_var = find_among_b(s_pool, a_4, 2, 0, 0);
                    if (!(among_var)) { c = l - m10; goto lab10; }
                    bra = c; /* ], line 128 */
                    {   int ret = r_R2(); /* call R2, line 128 */
                        if (ret == 0) { c = l - m10; goto lab10; }
                        if (ret < 0) return ret;
                    }
                    switch (among_var) { /* among, line 128 */
                        case 0: { c = l - m10; goto lab10; }
                        case 1:
                            if (slice_del() == -1) return -1; /* delete, line 130 */
                            break;
                    }
                lab10:
                    ;
                }
                break;
        }
    lab3:
        c = l - m4;
    }
    return 1;
}

int Xapian::InternalStemGerman2::stem() { /* forwardmode */
    {   int c1 = c; /* do, line 141 */
        {   int ret = r_prelude(); /* call prelude, line 141 */
            if (ret == 0) goto lab0;
            if (ret < 0) return ret;
        }
    lab0:
        c = c1;
    }
    {   int c2 = c; /* do, line 142 */
        {   int ret = r_mark_regions(); /* call mark_regions, line 142 */
            if (ret == 0) goto lab1;
            if (ret < 0) return ret;
        }
    lab1:
        c = c2;
    }
    lb = c; c = l; /* backwards, line 143 */

    {   int m3 = l - c; /*(void)m3*/; /* do, line 144 */
        {   int ret = r_standard_suffix(); /* call standard_suffix, line 144 */
            if (ret == 0) goto lab2;
            if (ret < 0) return ret;
        }
    lab2:
        c = l - m3;
    }
    c = lb;
    {   int c4 = c; /* do, line 145 */
        {   int ret = r_postlude(); /* call postlude, line 145 */
            if (ret == 0) goto lab3;
            if (ret < 0) return ret;
        }
    lab3:
        c = c4;
    }
    return 1;
}

Xapian::InternalStemGerman2::InternalStemGerman2()
    : I_x(0), I_p2(0), I_p1(0)
{
}

Xapian::InternalStemGerman2::~InternalStemGerman2()
{
}

std::string
Xapian::InternalStemGerman2::get_description() const
{
    return "german2";
}
