/* This file was generated automatically by the Snowball to ISO C++ compiler */
/* http://snowballstem.org/ */

#include <config.h>
#include <limits.h>
#include "italian.h"

static const symbol s_pool[] = {
#define s_0_1 0
'q', 'u',
#define s_0_2 2
0xC3, 0xA1,
#define s_0_3 4
0xC3, 0xA9,
#define s_0_4 6
0xC3, 0xAD,
#define s_0_5 8
0xC3, 0xB3,
#define s_0_6 10
0xC3, 0xBA,
#define s_1_1 12
'I',
#define s_1_2 13
'U',
#define s_2_0 (s_2_1 + 2)
#define s_2_1 14
'c', 'e', 'l', 'a',
#define s_2_2 18
'g', 'l', 'i', 'e', 'l', 'a',
#define s_2_3 24
'm', 'e', 'l', 'a',
#define s_2_4 28
't', 'e', 'l', 'a',
#define s_2_5 32
'v', 'e', 'l', 'a',
#define s_2_6 (s_2_7 + 2)
#define s_2_7 36
'c', 'e', 'l', 'e',
#define s_2_8 40
'g', 'l', 'i', 'e', 'l', 'e',
#define s_2_9 46
'm', 'e', 'l', 'e',
#define s_2_10 50
't', 'e', 'l', 'e',
#define s_2_11 54
'v', 'e', 'l', 'e',
#define s_2_12 (s_2_13 + 2)
#define s_2_13 58
'c', 'e', 'n', 'e',
#define s_2_14 62
'g', 'l', 'i', 'e', 'n', 'e',
#define s_2_15 68
'm', 'e', 'n', 'e',
#define s_2_16 72
's', 'e', 'n', 'e',
#define s_2_17 76
't', 'e', 'n', 'e',
#define s_2_18 80
'v', 'e', 'n', 'e',
#define s_2_19 84
'c', 'i',
#define s_2_20 (s_2_2 + 1)
#define s_2_21 86
'c', 'e', 'l', 'i',
#define s_2_22 90
'g', 'l', 'i', 'e', 'l', 'i',
#define s_2_23 96
'm', 'e', 'l', 'i',
#define s_2_24 100
't', 'e', 'l', 'i',
#define s_2_25 104
'v', 'e', 'l', 'i',
#define s_2_26 s_2_2
#define s_2_27 108
'm', 'i',
#define s_2_28 110
's', 'i',
#define s_2_29 112
't', 'i',
#define s_2_30 114
'v', 'i',
#define s_2_31 (s_2_32 + 2)
#define s_2_32 116
'c', 'e', 'l', 'o',
#define s_2_33 120
'g', 'l', 'i', 'e', 'l', 'o',
#define s_2_34 126
'm', 'e', 'l', 'o',
#define s_2_35 130
't', 'e', 'l', 'o',
#define s_2_36 134
'v', 'e', 'l', 'o',
#define s_3_0 138
'a', 'n', 'd', 'o',
#define s_3_1 142
'e', 'n', 'd', 'o',
#define s_3_2 146
'a', 'r',
#define s_3_3 148
'e', 'r',
#define s_3_4 150
'i', 'r',
#define s_4_0 152
'i', 'c',
#define s_4_1 154
'a', 'b', 'i', 'l',
#define s_4_2 158
'o', 's',
#define s_4_3 160
'i', 'v',
#define s_5_0 162
'i', 'c',
#define s_5_1 164
'a', 'b', 'i', 'l',
#define s_5_2 168
'i', 'v',
#define s_6_0 170
'i', 'c', 'a',
#define s_6_1 173
'l', 'o', 'g', 'i', 'a',
#define s_6_2 178
'o', 's', 'a',
#define s_6_3 181
'i', 's', 't', 'a',
#define s_6_4 185
'i', 'v', 'a',
#define s_6_5 188
'a', 'n', 'z', 'a',
#define s_6_6 192
'e', 'n', 'z', 'a',
#define s_6_7 (s_6_8 + 3)
#define s_6_8 196
'a', 't', 'r', 'i', 'c', 'e',
#define s_6_9 202
'i', 'c', 'h', 'e',
#define s_6_10 206
'l', 'o', 'g', 'i', 'e',
#define s_6_11 211
'a', 'b', 'i', 'l', 'e',
#define s_6_12 216
'i', 'b', 'i', 'l', 'e',
#define s_6_13 221
'u', 's', 'i', 'o', 'n', 'e',
#define s_6_14 227
'a', 'z', 'i', 'o', 'n', 'e',
#define s_6_15 233
'u', 'z', 'i', 'o', 'n', 'e',
#define s_6_16 239
'a', 't', 'o', 'r', 'e',
#define s_6_17 244
'o', 's', 'e',
#define s_6_18 247
'a', 'n', 't', 'e',
#define s_6_19 (s_6_20 + 1)
#define s_6_20 251
'a', 'm', 'e', 'n', 't', 'e',
#define s_6_21 257
'i', 's', 't', 'e',
#define s_6_22 261
'i', 'v', 'e',
#define s_6_23 264
'a', 'n', 'z', 'e',
#define s_6_24 268
'e', 'n', 'z', 'e',
#define s_6_25 (s_6_26 + 3)
#define s_6_26 272
'a', 't', 'r', 'i', 'c', 'i',
#define s_6_27 278
'i', 'c', 'h', 'i',
#define s_6_28 282
'a', 'b', 'i', 'l', 'i',
#define s_6_29 287
'i', 'b', 'i', 'l', 'i',
#define s_6_30 292
'i', 's', 'm', 'i',
#define s_6_31 296
'u', 's', 'i', 'o', 'n', 'i',
#define s_6_32 302
'a', 'z', 'i', 'o', 'n', 'i',
#define s_6_33 308
'u', 'z', 'i', 'o', 'n', 'i',
#define s_6_34 314
'a', 't', 'o', 'r', 'i',
#define s_6_35 319
'o', 's', 'i',
#define s_6_36 322
'a', 'n', 't', 'i',
#define s_6_37 326
'a', 'm', 'e', 'n', 't', 'i',
#define s_6_38 332
'i', 'm', 'e', 'n', 't', 'i',
#define s_6_39 338
'i', 's', 't', 'i',
#define s_6_40 342
'i', 'v', 'i',
#define s_6_41 345
'i', 'c', 'o',
#define s_6_42 348
'i', 's', 'm', 'o',
#define s_6_43 352
'o', 's', 'o',
#define s_6_44 355
'a', 'm', 'e', 'n', 't', 'o',
#define s_6_45 361
'i', 'm', 'e', 'n', 't', 'o',
#define s_6_46 367
'i', 'v', 'o',
#define s_6_47 370
'i', 't', 0xC3, 0xA0,
#define s_6_48 374
'i', 's', 't', 0xC3, 0xA0,
#define s_6_49 379
'i', 's', 't', 0xC3, 0xA8,
#define s_6_50 384
'i', 's', 't', 0xC3, 0xAC,
#define s_7_0 s_7_59
#define s_7_1 389
'e', 'n', 'd', 'a',
#define s_7_2 393
'a', 't', 'a',
#define s_7_3 396
'i', 't', 'a',
#define s_7_4 399
'u', 't', 'a',
#define s_7_5 s_7_17
#define s_7_6 s_7_18
#define s_7_7 s_7_19
#define s_7_8 s_7_70
#define s_7_9 s_7_71
#define s_7_10 402
'i', 's', 'c', 'e',
#define s_7_11 406
'e', 'n', 'd', 'e',
#define s_7_12 410
'a', 'r', 'e',
#define s_7_13 s_7_8
#define s_7_14 s_7_9
#define s_7_15 s_7_72
#define s_7_16 (s_7_17 + 2)
#define s_7_17 413
'a', 'v', 'a', 't', 'e',
#define s_7_18 418
'e', 'v', 'a', 't', 'e',
#define s_7_19 423
'i', 'v', 'a', 't', 'e',
#define s_7_20 (s_7_21 + 2)
#define s_7_21 428
'e', 'r', 'e', 't', 'e',
#define s_7_22 433
'i', 'r', 'e', 't', 'e',
#define s_7_23 438
'i', 't', 'e',
#define s_7_24 441
'e', 'r', 'e', 's', 't', 'e',
#define s_7_25 447
'i', 'r', 'e', 's', 't', 'e',
#define s_7_26 453
'u', 't', 'e',
#define s_7_27 456
'e', 'r', 'a', 'i',
#define s_7_28 460
'i', 'r', 'a', 'i',
#define s_7_29 464
'i', 's', 'c', 'i',
#define s_7_30 468
'e', 'n', 'd', 'i',
#define s_7_31 472
'e', 'r', 'e', 'i',
#define s_7_32 476
'i', 'r', 'e', 'i',
#define s_7_33 s_7_52
#define s_7_34 480
'a', 't', 'i',
#define s_7_35 483
'i', 't', 'i',
#define s_7_36 486
'e', 'r', 'e', 's', 't', 'i',
#define s_7_37 492
'i', 'r', 'e', 's', 't', 'i',
#define s_7_38 498
'u', 't', 'i',
#define s_7_39 501
'a', 'v', 'i',
#define s_7_40 504
'e', 'v', 'i',
#define s_7_41 507
'i', 'v', 'i',
#define s_7_42 s_7_66
#define s_7_43 510
'a', 'n', 'd', 'o',
#define s_7_44 514
'e', 'n', 'd', 'o',
#define s_7_45 518
'Y', 'a', 'm', 'o',
#define s_7_46 522
'i', 'a', 'm', 'o',
#define s_7_47 526
'a', 'v', 'a', 'm', 'o',
#define s_7_48 531
'e', 'v', 'a', 'm', 'o',
#define s_7_49 536
'i', 'v', 'a', 'm', 'o',
#define s_7_50 541
'e', 'r', 'e', 'm', 'o',
#define s_7_51 546
'i', 'r', 'e', 'm', 'o',
#define s_7_52 551
'a', 's', 's', 'i', 'm', 'o',
#define s_7_53 557
'a', 'm', 'm', 'o',
#define s_7_54 (s_7_55 + 2)
#define s_7_55 561
'e', 'r', 'e', 'm', 'm', 'o',
#define s_7_56 567
'i', 'r', 'e', 'm', 'm', 'o',
#define s_7_57 573
'i', 'm', 'm', 'o',
#define s_7_58 (s_7_59 + 3)
#define s_7_59 577
'i', 's', 'c', 'a', 'n', 'o',
#define s_7_60 583
'a', 'v', 'a', 'n', 'o',
#define s_7_61 588
'e', 'v', 'a', 'n', 'o',
#define s_7_62 593
'i', 'v', 'a', 'n', 'o',
#define s_7_63 598
'e', 'r', 'a', 'n', 'n', 'o',
#define s_7_64 604
'i', 'r', 'a', 'n', 'n', 'o',
#define s_7_65 (s_7_66 + 3)
#define s_7_66 610
'i', 's', 'c', 'o', 'n', 'o',
#define s_7_67 616
'a', 'r', 'o', 'n', 'o',
#define s_7_68 621
'e', 'r', 'o', 'n', 'o',
#define s_7_69 626
'i', 'r', 'o', 'n', 'o',
#define s_7_70 631
'e', 'r', 'e', 'b', 'b', 'e', 'r', 'o',
#define s_7_71 639
'i', 'r', 'e', 'b', 'b', 'e', 'r', 'o',
#define s_7_72 647
'a', 's', 's', 'e', 'r', 'o',
#define s_7_73 653
'e', 's', 's', 'e', 'r', 'o',
#define s_7_74 659
'i', 's', 's', 'e', 'r', 'o',
#define s_7_75 665
'a', 't', 'o',
#define s_7_76 668
'i', 't', 'o',
#define s_7_77 671
'u', 't', 'o',
#define s_7_78 674
'a', 'v', 'o',
#define s_7_79 677
'e', 'v', 'o',
#define s_7_80 680
'i', 'v', 'o',
#define s_7_81 s_7_12
#define s_7_82 s_7_9
#define s_7_83 683
'e', 'r', 0xC3, 0xA0,
#define s_7_84 687
'i', 'r', 0xC3, 0xA0,
#define s_7_85 691
'e', 'r', 0xC3, 0xB2,
#define s_7_86 695
'i', 'r', 0xC3, 0xB2,
};


static const struct among a_0[7] =
{
/*  0 */ { 0, 0, -1, 7},
/*  1 */ { 2, s_0_1, 0, 6},
/*  2 */ { 2, s_0_2, 0, 1},
/*  3 */ { 2, s_0_3, 0, 2},
/*  4 */ { 2, s_0_4, 0, 3},
/*  5 */ { 2, s_0_5, 0, 4},
/*  6 */ { 2, s_0_6, 0, 5}
};


static const struct among a_1[3] =
{
/*  0 */ { 0, 0, -1, 3},
/*  1 */ { 1, s_1_1, 0, 1},
/*  2 */ { 1, s_1_2, 0, 2}
};


static const struct among a_2[37] =
{
/*  0 */ { 2, s_2_0, -1, -1},
/*  1 */ { 4, s_2_1, 0, -1},
/*  2 */ { 6, s_2_2, 0, -1},
/*  3 */ { 4, s_2_3, 0, -1},
/*  4 */ { 4, s_2_4, 0, -1},
/*  5 */ { 4, s_2_5, 0, -1},
/*  6 */ { 2, s_2_6, -1, -1},
/*  7 */ { 4, s_2_7, 6, -1},
/*  8 */ { 6, s_2_8, 6, -1},
/*  9 */ { 4, s_2_9, 6, -1},
/* 10 */ { 4, s_2_10, 6, -1},
/* 11 */ { 4, s_2_11, 6, -1},
/* 12 */ { 2, s_2_12, -1, -1},
/* 13 */ { 4, s_2_13, 12, -1},
/* 14 */ { 6, s_2_14, 12, -1},
/* 15 */ { 4, s_2_15, 12, -1},
/* 16 */ { 4, s_2_16, 12, -1},
/* 17 */ { 4, s_2_17, 12, -1},
/* 18 */ { 4, s_2_18, 12, -1},
/* 19 */ { 2, s_2_19, -1, -1},
/* 20 */ { 2, s_2_20, -1, -1},
/* 21 */ { 4, s_2_21, 20, -1},
/* 22 */ { 6, s_2_22, 20, -1},
/* 23 */ { 4, s_2_23, 20, -1},
/* 24 */ { 4, s_2_24, 20, -1},
/* 25 */ { 4, s_2_25, 20, -1},
/* 26 */ { 3, s_2_26, 20, -1},
/* 27 */ { 2, s_2_27, -1, -1},
/* 28 */ { 2, s_2_28, -1, -1},
/* 29 */ { 2, s_2_29, -1, -1},
/* 30 */ { 2, s_2_30, -1, -1},
/* 31 */ { 2, s_2_31, -1, -1},
/* 32 */ { 4, s_2_32, 31, -1},
/* 33 */ { 6, s_2_33, 31, -1},
/* 34 */ { 4, s_2_34, 31, -1},
/* 35 */ { 4, s_2_35, 31, -1},
/* 36 */ { 4, s_2_36, 31, -1}
};


static const struct among a_3[5] =
{
/*  0 */ { 4, s_3_0, -1, 1},
/*  1 */ { 4, s_3_1, -1, 1},
/*  2 */ { 2, s_3_2, -1, 2},
/*  3 */ { 2, s_3_3, -1, 2},
/*  4 */ { 2, s_3_4, -1, 2}
};


static const struct among a_4[4] =
{
/*  0 */ { 2, s_4_0, -1, -1},
/*  1 */ { 4, s_4_1, -1, -1},
/*  2 */ { 2, s_4_2, -1, -1},
/*  3 */ { 2, s_4_3, -1, 1}
};


static const struct among a_5[3] =
{
/*  0 */ { 2, s_5_0, -1, 1},
/*  1 */ { 4, s_5_1, -1, 1},
/*  2 */ { 2, s_5_2, -1, 1}
};


static const struct among a_6[51] =
{
/*  0 */ { 3, s_6_0, -1, 1},
/*  1 */ { 5, s_6_1, -1, 3},
/*  2 */ { 3, s_6_2, -1, 1},
/*  3 */ { 4, s_6_3, -1, 1},
/*  4 */ { 3, s_6_4, -1, 9},
/*  5 */ { 4, s_6_5, -1, 1},
/*  6 */ { 4, s_6_6, -1, 5},
/*  7 */ { 3, s_6_7, -1, 1},
/*  8 */ { 6, s_6_8, 7, 1},
/*  9 */ { 4, s_6_9, -1, 1},
/* 10 */ { 5, s_6_10, -1, 3},
/* 11 */ { 5, s_6_11, -1, 1},
/* 12 */ { 5, s_6_12, -1, 1},
/* 13 */ { 6, s_6_13, -1, 4},
/* 14 */ { 6, s_6_14, -1, 2},
/* 15 */ { 6, s_6_15, -1, 4},
/* 16 */ { 5, s_6_16, -1, 2},
/* 17 */ { 3, s_6_17, -1, 1},
/* 18 */ { 4, s_6_18, -1, 1},
/* 19 */ { 5, s_6_19, -1, 1},
/* 20 */ { 6, s_6_20, 19, 7},
/* 21 */ { 4, s_6_21, -1, 1},
/* 22 */ { 3, s_6_22, -1, 9},
/* 23 */ { 4, s_6_23, -1, 1},
/* 24 */ { 4, s_6_24, -1, 5},
/* 25 */ { 3, s_6_25, -1, 1},
/* 26 */ { 6, s_6_26, 25, 1},
/* 27 */ { 4, s_6_27, -1, 1},
/* 28 */ { 5, s_6_28, -1, 1},
/* 29 */ { 5, s_6_29, -1, 1},
/* 30 */ { 4, s_6_30, -1, 1},
/* 31 */ { 6, s_6_31, -1, 4},
/* 32 */ { 6, s_6_32, -1, 2},
/* 33 */ { 6, s_6_33, -1, 4},
/* 34 */ { 5, s_6_34, -1, 2},
/* 35 */ { 3, s_6_35, -1, 1},
/* 36 */ { 4, s_6_36, -1, 1},
/* 37 */ { 6, s_6_37, -1, 6},
/* 38 */ { 6, s_6_38, -1, 6},
/* 39 */ { 4, s_6_39, -1, 1},
/* 40 */ { 3, s_6_40, -1, 9},
/* 41 */ { 3, s_6_41, -1, 1},
/* 42 */ { 4, s_6_42, -1, 1},
/* 43 */ { 3, s_6_43, -1, 1},
/* 44 */ { 6, s_6_44, -1, 6},
/* 45 */ { 6, s_6_45, -1, 6},
/* 46 */ { 3, s_6_46, -1, 9},
/* 47 */ { 4, s_6_47, -1, 8},
/* 48 */ { 5, s_6_48, -1, 1},
/* 49 */ { 5, s_6_49, -1, 1},
/* 50 */ { 5, s_6_50, -1, 1}
};


static const struct among a_7[87] =
{
/*  0 */ { 4, s_7_0, -1, 1},
/*  1 */ { 4, s_7_1, -1, 1},
/*  2 */ { 3, s_7_2, -1, 1},
/*  3 */ { 3, s_7_3, -1, 1},
/*  4 */ { 3, s_7_4, -1, 1},
/*  5 */ { 3, s_7_5, -1, 1},
/*  6 */ { 3, s_7_6, -1, 1},
/*  7 */ { 3, s_7_7, -1, 1},
/*  8 */ { 6, s_7_8, -1, 1},
/*  9 */ { 6, s_7_9, -1, 1},
/* 10 */ { 4, s_7_10, -1, 1},
/* 11 */ { 4, s_7_11, -1, 1},
/* 12 */ { 3, s_7_12, -1, 1},
/* 13 */ { 3, s_7_13, -1, 1},
/* 14 */ { 3, s_7_14, -1, 1},
/* 15 */ { 4, s_7_15, -1, 1},
/* 16 */ { 3, s_7_16, -1, 1},
/* 17 */ { 5, s_7_17, 16, 1},
/* 18 */ { 5, s_7_18, 16, 1},
/* 19 */ { 5, s_7_19, 16, 1},
/* 20 */ { 3, s_7_20, -1, 1},
/* 21 */ { 5, s_7_21, 20, 1},
/* 22 */ { 5, s_7_22, 20, 1},
/* 23 */ { 3, s_7_23, -1, 1},
/* 24 */ { 6, s_7_24, -1, 1},
/* 25 */ { 6, s_7_25, -1, 1},
/* 26 */ { 3, s_7_26, -1, 1},
/* 27 */ { 4, s_7_27, -1, 1},
/* 28 */ { 4, s_7_28, -1, 1},
/* 29 */ { 4, s_7_29, -1, 1},
/* 30 */ { 4, s_7_30, -1, 1},
/* 31 */ { 4, s_7_31, -1, 1},
/* 32 */ { 4, s_7_32, -1, 1},
/* 33 */ { 4, s_7_33, -1, 1},
/* 34 */ { 3, s_7_34, -1, 1},
/* 35 */ { 3, s_7_35, -1, 1},
/* 36 */ { 6, s_7_36, -1, 1},
/* 37 */ { 6, s_7_37, -1, 1},
/* 38 */ { 3, s_7_38, -1, 1},
/* 39 */ { 3, s_7_39, -1, 1},
/* 40 */ { 3, s_7_40, -1, 1},
/* 41 */ { 3, s_7_41, -1, 1},
/* 42 */ { 4, s_7_42, -1, 1},
/* 43 */ { 4, s_7_43, -1, 1},
/* 44 */ { 4, s_7_44, -1, 1},
/* 45 */ { 4, s_7_45, -1, 1},
/* 46 */ { 4, s_7_46, -1, 1},
/* 47 */ { 5, s_7_47, -1, 1},
/* 48 */ { 5, s_7_48, -1, 1},
/* 49 */ { 5, s_7_49, -1, 1},
/* 50 */ { 5, s_7_50, -1, 1},
/* 51 */ { 5, s_7_51, -1, 1},
/* 52 */ { 6, s_7_52, -1, 1},
/* 53 */ { 4, s_7_53, -1, 1},
/* 54 */ { 4, s_7_54, -1, 1},
/* 55 */ { 6, s_7_55, 54, 1},
/* 56 */ { 6, s_7_56, 54, 1},
/* 57 */ { 4, s_7_57, -1, 1},
/* 58 */ { 3, s_7_58, -1, 1},
/* 59 */ { 6, s_7_59, 58, 1},
/* 60 */ { 5, s_7_60, 58, 1},
/* 61 */ { 5, s_7_61, 58, 1},
/* 62 */ { 5, s_7_62, 58, 1},
/* 63 */ { 6, s_7_63, -1, 1},
/* 64 */ { 6, s_7_64, -1, 1},
/* 65 */ { 3, s_7_65, -1, 1},
/* 66 */ { 6, s_7_66, 65, 1},
/* 67 */ { 5, s_7_67, 65, 1},
/* 68 */ { 5, s_7_68, 65, 1},
/* 69 */ { 5, s_7_69, 65, 1},
/* 70 */ { 8, s_7_70, -1, 1},
/* 71 */ { 8, s_7_71, -1, 1},
/* 72 */ { 6, s_7_72, -1, 1},
/* 73 */ { 6, s_7_73, -1, 1},
/* 74 */ { 6, s_7_74, -1, 1},
/* 75 */ { 3, s_7_75, -1, 1},
/* 76 */ { 3, s_7_76, -1, 1},
/* 77 */ { 3, s_7_77, -1, 1},
/* 78 */ { 3, s_7_78, -1, 1},
/* 79 */ { 3, s_7_79, -1, 1},
/* 80 */ { 3, s_7_80, -1, 1},
/* 81 */ { 2, s_7_81, -1, 1},
/* 82 */ { 2, s_7_82, -1, 1},
/* 83 */ { 4, s_7_83, -1, 1},
/* 84 */ { 4, s_7_84, -1, 1},
/* 85 */ { 4, s_7_85, -1, 1},
/* 86 */ { 4, s_7_86, -1, 1}
};

static const unsigned char g_v[] = { 17, 65, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 128, 8, 2, 1 };

static const unsigned char g_AEIO[] = { 17, 65, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 128, 8, 2 };

static const unsigned char g_CG[] = { 17 };

static const symbol s_0[] = { 0xC3, 0xA0 };
static const symbol s_1[] = { 0xC3, 0xA8 };
static const symbol s_2[] = { 0xC3, 0xAC };
static const symbol s_3[] = { 0xC3, 0xB2 };
static const symbol s_4[] = { 0xC3, 0xB9 };
static const symbol s_5[] = { 'q', 'U' };
static const symbol s_6[] = { 'U' };
static const symbol s_7[] = { 'I' };
static const symbol s_8[] = { 'i' };
static const symbol s_9[] = { 'u' };
static const symbol s_10[] = { 'e' };
static const symbol s_11[] = { 'i', 'c' };
static const symbol s_12[] = { 'l', 'o', 'g' };
static const symbol s_13[] = { 'u' };
static const symbol s_14[] = { 'e', 'n', 't', 'e' };
static const symbol s_15[] = { 'a', 't' };
static const symbol s_16[] = { 'a', 't' };
static const symbol s_17[] = { 'i', 'c' };

int Xapian::InternalStemItalian::r_prelude() { /* forwardmode */
    int among_var;
    {   int c_test1 = c; /* test, line 36 */
        while(1) { /* repeat, line 36 */
            int c2 = c;
            bra = c; /* [, line 37 */
            among_var = find_among(s_pool, a_0, 7, 0, 0); /* substring, line 37 */
            if (!(among_var)) goto lab0;
            ket = c; /* ], line 37 */
            switch (among_var) { /* among, line 37 */
                case 0: goto lab0;
                case 1:
                    {   int ret = slice_from_s(2, s_0); /* <-, line 38 */
                        if (ret < 0) return ret;
                    }
                    break;
                case 2:
                    {   int ret = slice_from_s(2, s_1); /* <-, line 39 */
                        if (ret < 0) return ret;
                    }
                    break;
                case 3:
                    {   int ret = slice_from_s(2, s_2); /* <-, line 40 */
                        if (ret < 0) return ret;
                    }
                    break;
                case 4:
                    {   int ret = slice_from_s(2, s_3); /* <-, line 41 */
                        if (ret < 0) return ret;
                    }
                    break;
                case 5:
                    {   int ret = slice_from_s(2, s_4); /* <-, line 42 */
                        if (ret < 0) return ret;
                    }
                    break;
                case 6:
                    {   int ret = slice_from_s(2, s_5); /* <-, line 43 */
                        if (ret < 0) return ret;
                    }
                    break;
                case 7:
                    {   int ret = skip_utf8(p, c, 0, l, 1);
                        if (ret < 0) goto lab0;
                        c = ret; /* next, line 44 */
                    }
                    break;
            }
            continue;
        lab0:
            c = c2;
            break;
        }
        c = c_test1;
    }
    while(1) { /* repeat, line 47 */
        int c3 = c;
        while(1) { /* goto, line 47 */
            int c4 = c;
            if (in_grouping_U(g_v, 97, 249, 0)) goto lab2; /* grouping v, line 48 */
            bra = c; /* [, line 48 */
            {   int c5 = c; /* or, line 48 */
                if (c == l || p[c] != 'u') goto lab4; /* literal, line 48 */
                c++;
                ket = c; /* ], line 48 */
                if (in_grouping_U(g_v, 97, 249, 0)) goto lab4; /* grouping v, line 48 */
                {   int ret = slice_from_s(1, s_6); /* <-, line 48 */
                    if (ret < 0) return ret;
                }
                goto lab3;
            lab4:
                c = c5;
                if (c == l || p[c] != 'i') goto lab2; /* literal, line 49 */
                c++;
                ket = c; /* ], line 49 */
                if (in_grouping_U(g_v, 97, 249, 0)) goto lab2; /* grouping v, line 49 */
                {   int ret = slice_from_s(1, s_7); /* <-, line 49 */
                    if (ret < 0) return ret;
                }
            }
        lab3:
            c = c4;
            break;
        lab2:
            c = c4;
            {   int ret = skip_utf8(p, c, 0, l, 1);
                if (ret < 0) goto lab1;
                c = ret; /* goto, line 47 */
            }
        }
        continue;
    lab1:
        c = c3;
        break;
    }
    return 1;
}

int Xapian::InternalStemItalian::r_mark_regions() { /* forwardmode */
    I_pV = l; /* $pV = <integer expression>, line 55 */
    I_p1 = l; /* $p1 = <integer expression>, line 56 */
    I_p2 = l; /* $p2 = <integer expression>, line 57 */
    {   int c1 = c; /* do, line 59 */
        {   int c2 = c; /* or, line 61 */
            if (in_grouping_U(g_v, 97, 249, 0)) goto lab2; /* grouping v, line 60 */
            {   int c3 = c; /* or, line 60 */
                if (out_grouping_U(g_v, 97, 249, 0)) goto lab4; /* non v, line 60 */
                {    /* gopast */ /* grouping v, line 60 */
                    int ret = out_grouping_U(g_v, 97, 249, 1);
                    if (ret < 0) goto lab4;
                    c += ret;
                }
                goto lab3;
            lab4:
                c = c3;
                if (in_grouping_U(g_v, 97, 249, 0)) goto lab2; /* grouping v, line 60 */
                {    /* gopast */ /* non v, line 60 */
                    int ret = in_grouping_U(g_v, 97, 249, 1);
                    if (ret < 0) goto lab2;
                    c += ret;
                }
            }
        lab3:
            goto lab1;
        lab2:
            c = c2;
            if (out_grouping_U(g_v, 97, 249, 0)) goto lab0; /* non v, line 62 */
            {   int c4 = c; /* or, line 62 */
                if (out_grouping_U(g_v, 97, 249, 0)) goto lab6; /* non v, line 62 */
                {    /* gopast */ /* grouping v, line 62 */
                    int ret = out_grouping_U(g_v, 97, 249, 1);
                    if (ret < 0) goto lab6;
                    c += ret;
                }
                goto lab5;
            lab6:
                c = c4;
                if (in_grouping_U(g_v, 97, 249, 0)) goto lab0; /* grouping v, line 62 */
                {   int ret = skip_utf8(p, c, 0, l, 1);
                    if (ret < 0) goto lab0;
                    c = ret; /* next, line 62 */
                }
            }
        lab5:
            ;
        }
    lab1:
        I_pV = c; /* setmark pV, line 63 */
    lab0:
        c = c1;
    }
    {   int c5 = c; /* do, line 65 */
        {    /* gopast */ /* grouping v, line 66 */
            int ret = out_grouping_U(g_v, 97, 249, 1);
            if (ret < 0) goto lab7;
            c += ret;
        }
        {    /* gopast */ /* non v, line 66 */
            int ret = in_grouping_U(g_v, 97, 249, 1);
            if (ret < 0) goto lab7;
            c += ret;
        }
        I_p1 = c; /* setmark p1, line 66 */
        {    /* gopast */ /* grouping v, line 67 */
            int ret = out_grouping_U(g_v, 97, 249, 1);
            if (ret < 0) goto lab7;
            c += ret;
        }
        {    /* gopast */ /* non v, line 67 */
            int ret = in_grouping_U(g_v, 97, 249, 1);
            if (ret < 0) goto lab7;
            c += ret;
        }
        I_p2 = c; /* setmark p2, line 67 */
    lab7:
        c = c5;
    }
    return 1;
}

int Xapian::InternalStemItalian::r_postlude() { /* forwardmode */
    int among_var;
    while(1) { /* repeat, line 71 */
        int c1 = c;
        bra = c; /* [, line 73 */
        if (c >= l || (p[c + 0] != 73 && p[c + 0] != 85)) among_var = 3; else /* substring, line 73 */
        among_var = find_among(s_pool, a_1, 3, 0, 0);
        if (!(among_var)) goto lab0;
        ket = c; /* ], line 73 */
        switch (among_var) { /* among, line 73 */
            case 0: goto lab0;
            case 1:
                {   int ret = slice_from_s(1, s_8); /* <-, line 74 */
                    if (ret < 0) return ret;
                }
                break;
            case 2:
                {   int ret = slice_from_s(1, s_9); /* <-, line 75 */
                    if (ret < 0) return ret;
                }
                break;
            case 3:
                {   int ret = skip_utf8(p, c, 0, l, 1);
                    if (ret < 0) goto lab0;
                    c = ret; /* next, line 76 */
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

int Xapian::InternalStemItalian::r_RV() { /* backwardmode */
    if (!(I_pV <= c)) return 0; /* $pV <= <integer expression>, line 83 */
    return 1;
}

int Xapian::InternalStemItalian::r_R1() { /* backwardmode */
    if (!(I_p1 <= c)) return 0; /* $p1 <= <integer expression>, line 84 */
    return 1;
}

int Xapian::InternalStemItalian::r_R2() { /* backwardmode */
    if (!(I_p2 <= c)) return 0; /* $p2 <= <integer expression>, line 85 */
    return 1;
}

int Xapian::InternalStemItalian::r_attached_pronoun() { /* backwardmode */
    int among_var;
    ket = c; /* [, line 88 */
    if (c - 1 <= lb || p[c - 1] >> 5 != 3 || !((33314 >> (p[c - 1] & 0x1f)) & 1)) return 0; /* substring, line 88 */
    if (!(find_among_b(s_pool, a_2, 37, 0, 0))) return 0;
    bra = c; /* ], line 88 */
    if (c - 1 <= lb || (p[c - 1] != 111 && p[c - 1] != 114)) return 0; /* among, line 98 */
    among_var = find_among_b(s_pool, a_3, 5, 0, 0);
    if (!(among_var)) return 0;
    {   int ret = r_RV(); /* call RV, line 98 */
        if (ret <= 0) return ret;
    }
    switch (among_var) { /* among, line 98 */
        case 0: return 0;
        case 1:
            if (slice_del() == -1) return -1; /* delete, line 99 */
            break;
        case 2:
            {   int ret = slice_from_s(1, s_10); /* <-, line 100 */
                if (ret < 0) return ret;
            }
            break;
    }
    return 1;
}

int Xapian::InternalStemItalian::r_standard_suffix() { /* backwardmode */
    int among_var;
    ket = c; /* [, line 105 */
    among_var = find_among_b(s_pool, a_6, 51, 0, 0); /* substring, line 105 */
    if (!(among_var)) return 0;
    bra = c; /* ], line 105 */
    switch (among_var) { /* among, line 105 */
        case 0: return 0;
        case 1:
            {   int ret = r_R2(); /* call R2, line 112 */
                if (ret <= 0) return ret;
            }
            if (slice_del() == -1) return -1; /* delete, line 112 */
            break;
        case 2:
            {   int ret = r_R2(); /* call R2, line 114 */
                if (ret <= 0) return ret;
            }
            if (slice_del() == -1) return -1; /* delete, line 114 */
            {   int m1 = l - c; /*(void)m1*/; /* try, line 115 */
                ket = c; /* [, line 115 */
                if (!(eq_s_b(2, s_11))) { c = l - m1; goto lab0; } /* literal, line 115 */
                bra = c; /* ], line 115 */
                {   int ret = r_R2(); /* call R2, line 115 */
                    if (ret == 0) { c = l - m1; goto lab0; }
                    if (ret < 0) return ret;
                }
                if (slice_del() == -1) return -1; /* delete, line 115 */
            lab0:
                ;
            }
            break;
        case 3:
            {   int ret = r_R2(); /* call R2, line 118 */
                if (ret <= 0) return ret;
            }
            {   int ret = slice_from_s(3, s_12); /* <-, line 118 */
                if (ret < 0) return ret;
            }
            break;
        case 4:
            {   int ret = r_R2(); /* call R2, line 120 */
                if (ret <= 0) return ret;
            }
            {   int ret = slice_from_s(1, s_13); /* <-, line 120 */
                if (ret < 0) return ret;
            }
            break;
        case 5:
            {   int ret = r_R2(); /* call R2, line 122 */
                if (ret <= 0) return ret;
            }
            {   int ret = slice_from_s(4, s_14); /* <-, line 122 */
                if (ret < 0) return ret;
            }
            break;
        case 6:
            {   int ret = r_RV(); /* call RV, line 124 */
                if (ret <= 0) return ret;
            }
            if (slice_del() == -1) return -1; /* delete, line 124 */
            break;
        case 7:
            {   int ret = r_R1(); /* call R1, line 126 */
                if (ret <= 0) return ret;
            }
            if (slice_del() == -1) return -1; /* delete, line 126 */
            {   int m2 = l - c; /*(void)m2*/; /* try, line 127 */
                ket = c; /* [, line 128 */
                if (c - 1 <= lb || p[c - 1] >> 5 != 3 || !((4722696 >> (p[c - 1] & 0x1f)) & 1)) { c = l - m2; goto lab1; } /* substring, line 128 */
                among_var = find_among_b(s_pool, a_4, 4, 0, 0);
                if (!(among_var)) { c = l - m2; goto lab1; }
                bra = c; /* ], line 128 */
                {   int ret = r_R2(); /* call R2, line 128 */
                    if (ret == 0) { c = l - m2; goto lab1; }
                    if (ret < 0) return ret;
                }
                if (slice_del() == -1) return -1; /* delete, line 128 */
                switch (among_var) { /* among, line 128 */
                    case 0: { c = l - m2; goto lab1; }
                    case 1:
                        ket = c; /* [, line 129 */
                        if (!(eq_s_b(2, s_15))) { c = l - m2; goto lab1; } /* literal, line 129 */
                        bra = c; /* ], line 129 */
                        {   int ret = r_R2(); /* call R2, line 129 */
                            if (ret == 0) { c = l - m2; goto lab1; }
                            if (ret < 0) return ret;
                        }
                        if (slice_del() == -1) return -1; /* delete, line 129 */
                        break;
                }
            lab1:
                ;
            }
            break;
        case 8:
            {   int ret = r_R2(); /* call R2, line 135 */
                if (ret <= 0) return ret;
            }
            if (slice_del() == -1) return -1; /* delete, line 135 */
            {   int m3 = l - c; /*(void)m3*/; /* try, line 136 */
                ket = c; /* [, line 137 */
                if (c - 1 <= lb || p[c - 1] >> 5 != 3 || !((4198408 >> (p[c - 1] & 0x1f)) & 1)) { c = l - m3; goto lab2; } /* substring, line 137 */
                among_var = find_among_b(s_pool, a_5, 3, 0, 0);
                if (!(among_var)) { c = l - m3; goto lab2; }
                bra = c; /* ], line 137 */
                switch (among_var) { /* among, line 137 */
                    case 0: { c = l - m3; goto lab2; }
                    case 1:
                        {   int ret = r_R2(); /* call R2, line 138 */
                            if (ret == 0) { c = l - m3; goto lab2; }
                            if (ret < 0) return ret;
                        }
                        if (slice_del() == -1) return -1; /* delete, line 138 */
                        break;
                }
            lab2:
                ;
            }
            break;
        case 9:
            {   int ret = r_R2(); /* call R2, line 143 */
                if (ret <= 0) return ret;
            }
            if (slice_del() == -1) return -1; /* delete, line 143 */
            {   int m4 = l - c; /*(void)m4*/; /* try, line 144 */
                ket = c; /* [, line 144 */
                if (!(eq_s_b(2, s_16))) { c = l - m4; goto lab3; } /* literal, line 144 */
                bra = c; /* ], line 144 */
                {   int ret = r_R2(); /* call R2, line 144 */
                    if (ret == 0) { c = l - m4; goto lab3; }
                    if (ret < 0) return ret;
                }
                if (slice_del() == -1) return -1; /* delete, line 144 */
                ket = c; /* [, line 144 */
                if (!(eq_s_b(2, s_17))) { c = l - m4; goto lab3; } /* literal, line 144 */
                bra = c; /* ], line 144 */
                {   int ret = r_R2(); /* call R2, line 144 */
                    if (ret == 0) { c = l - m4; goto lab3; }
                    if (ret < 0) return ret;
                }
                if (slice_del() == -1) return -1; /* delete, line 144 */
            lab3:
                ;
            }
            break;
    }
    return 1;
}

int Xapian::InternalStemItalian::r_verb_suffix() { /* backwardmode */
    int among_var;
    {   int m1 = l - c; /*(void)m1*/; /* setlimit, line 149 */
        int mlimit1;
        if (c < I_pV) return 0;
        c = I_pV; /* tomark, line 149 */
        mlimit1 = lb; lb = c;
        c = l - m1;
        ket = c; /* [, line 150 */
        among_var = find_among_b(s_pool, a_7, 87, 0, 0); /* substring, line 150 */
        if (!(among_var)) { lb = mlimit1; return 0; }
        bra = c; /* ], line 150 */
        switch (among_var) { /* among, line 150 */
            case 0: { lb = mlimit1; return 0; }
            case 1:
                if (slice_del() == -1) return -1; /* delete, line 164 */
                break;
        }
        lb = mlimit1;
    }
    return 1;
}

int Xapian::InternalStemItalian::r_vowel_suffix() { /* backwardmode */
    {   int m1 = l - c; /*(void)m1*/; /* try, line 172 */
        ket = c; /* [, line 173 */
        if (in_grouping_b_U(g_AEIO, 97, 242, 0)) { c = l - m1; goto lab0; } /* grouping AEIO, line 173 */
        bra = c; /* ], line 173 */
        {   int ret = r_RV(); /* call RV, line 173 */
            if (ret == 0) { c = l - m1; goto lab0; }
            if (ret < 0) return ret;
        }
        if (slice_del() == -1) return -1; /* delete, line 173 */
        ket = c; /* [, line 174 */
        if (c <= lb || p[c - 1] != 'i') { c = l - m1; goto lab0; } /* literal, line 174 */
        c--;
        bra = c; /* ], line 174 */
        {   int ret = r_RV(); /* call RV, line 174 */
            if (ret == 0) { c = l - m1; goto lab0; }
            if (ret < 0) return ret;
        }
        if (slice_del() == -1) return -1; /* delete, line 174 */
    lab0:
        ;
    }
    {   int m2 = l - c; /*(void)m2*/; /* try, line 176 */
        ket = c; /* [, line 177 */
        if (c <= lb || p[c - 1] != 'h') { c = l - m2; goto lab1; } /* literal, line 177 */
        c--;
        bra = c; /* ], line 177 */
        if (in_grouping_b_U(g_CG, 99, 103, 0)) { c = l - m2; goto lab1; } /* grouping CG, line 177 */
        {   int ret = r_RV(); /* call RV, line 177 */
            if (ret == 0) { c = l - m2; goto lab1; }
            if (ret < 0) return ret;
        }
        if (slice_del() == -1) return -1; /* delete, line 177 */
    lab1:
        ;
    }
    return 1;
}

int Xapian::InternalStemItalian::stem() { /* forwardmode */
    {   int c1 = c; /* do, line 183 */
        {   int ret = r_prelude(); /* call prelude, line 183 */
            if (ret == 0) goto lab0;
            if (ret < 0) return ret;
        }
    lab0:
        c = c1;
    }
    {   int c2 = c; /* do, line 184 */
        {   int ret = r_mark_regions(); /* call mark_regions, line 184 */
            if (ret == 0) goto lab1;
            if (ret < 0) return ret;
        }
    lab1:
        c = c2;
    }
    lb = c; c = l; /* backwards, line 185 */

    {   int m3 = l - c; /*(void)m3*/; /* do, line 186 */
        {   int ret = r_attached_pronoun(); /* call attached_pronoun, line 186 */
            if (ret == 0) goto lab2;
            if (ret < 0) return ret;
        }
    lab2:
        c = l - m3;
    }
    {   int m4 = l - c; /*(void)m4*/; /* do, line 187 */
        {   int m5 = l - c; /*(void)m5*/; /* or, line 187 */
            {   int ret = r_standard_suffix(); /* call standard_suffix, line 187 */
                if (ret == 0) goto lab5;
                if (ret < 0) return ret;
            }
            goto lab4;
        lab5:
            c = l - m5;
            {   int ret = r_verb_suffix(); /* call verb_suffix, line 187 */
                if (ret == 0) goto lab3;
                if (ret < 0) return ret;
            }
        }
    lab4:
    lab3:
        c = l - m4;
    }
    {   int m6 = l - c; /*(void)m6*/; /* do, line 188 */
        {   int ret = r_vowel_suffix(); /* call vowel_suffix, line 188 */
            if (ret == 0) goto lab6;
            if (ret < 0) return ret;
        }
    lab6:
        c = l - m6;
    }
    c = lb;
    {   int c7 = c; /* do, line 190 */
        {   int ret = r_postlude(); /* call postlude, line 190 */
            if (ret == 0) goto lab7;
            if (ret < 0) return ret;
        }
    lab7:
        c = c7;
    }
    return 1;
}

Xapian::InternalStemItalian::InternalStemItalian()
    : I_p2(0), I_p1(0), I_pV(0)
{
}

Xapian::InternalStemItalian::~InternalStemItalian()
{
}

std::string
Xapian::InternalStemItalian::get_description() const
{
    return "italian";
}
