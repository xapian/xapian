
/* This file was generated automatically by the Snowball to ANSI C compiler */

#include "header.h"

extern int snowball_portuguese_stem(struct SN_env * z);
static int r_residual_form(struct SN_env * z);
static int r_residual_suffix(struct SN_env * z);
static int r_verb_suffix(struct SN_env * z);
static int r_standard_suffix(struct SN_env * z);
static int r_R2(struct SN_env * z);
static int r_R1(struct SN_env * z);
static int r_RV(struct SN_env * z);
static int r_mark_regions(struct SN_env * z);
static int r_postlude(struct SN_env * z);
static int r_prelude(struct SN_env * z);

static symbol s_0_1[1] = { 227 };
static symbol s_0_2[1] = { 245 };

static struct among a_0[3] =
{
/*  0 */ { 0, 0, -1, 3, 0},
/*  1 */ { 1, s_0_1, 0, 1, 0},
/*  2 */ { 1, s_0_2, 0, 2, 0}
};

static symbol s_1_1[2] = { 'a', '~' };
static symbol s_1_2[2] = { 'o', '~' };

static struct among a_1[3] =
{
/*  0 */ { 0, 0, -1, 3, 0},
/*  1 */ { 2, s_1_1, 0, 1, 0},
/*  2 */ { 2, s_1_2, 0, 2, 0}
};

static symbol s_2_0[2] = { 'i', 'c' };
static symbol s_2_1[2] = { 'a', 'd' };
static symbol s_2_2[2] = { 'o', 's' };
static symbol s_2_3[2] = { 'i', 'v' };

static struct among a_2[4] =
{
/*  0 */ { 2, s_2_0, -1, -1, 0},
/*  1 */ { 2, s_2_1, -1, -1, 0},
/*  2 */ { 2, s_2_2, -1, -1, 0},
/*  3 */ { 2, s_2_3, -1, 1, 0}
};

static symbol s_3_0[4] = { 'a', 'v', 'e', 'l' };
static symbol s_3_1[4] = { 237, 'v', 'e', 'l' };

static struct among a_3[2] =
{
/*  0 */ { 4, s_3_0, -1, 1, 0},
/*  1 */ { 4, s_3_1, -1, 1, 0}
};

static symbol s_4_0[2] = { 'i', 'c' };
static symbol s_4_1[4] = { 'a', 'b', 'i', 'l' };
static symbol s_4_2[2] = { 'i', 'v' };

static struct among a_4[3] =
{
/*  0 */ { 2, s_4_0, -1, 1, 0},
/*  1 */ { 4, s_4_1, -1, 1, 0},
/*  2 */ { 2, s_4_2, -1, 1, 0}
};

static symbol s_5_0[3] = { 'i', 'c', 'a' };
static symbol s_5_1[5] = { 234, 'n', 'c', 'i', 'a' };
static symbol s_5_2[3] = { 'i', 'r', 'a' };
static symbol s_5_3[5] = { 'a', 'd', 'o', 'r', 'a' };
static symbol s_5_4[3] = { 'o', 's', 'a' };
static symbol s_5_5[4] = { 'i', 's', 't', 'a' };
static symbol s_5_6[3] = { 'i', 'v', 'a' };
static symbol s_5_7[3] = { 'e', 'z', 'a' };
static symbol s_5_8[5] = { 'l', 'o', 'g', 237, 'a' };
static symbol s_5_9[5] = { 'i', 'd', 'a', 'd', 'e' };
static symbol s_5_10[5] = { 'm', 'e', 'n', 't', 'e' };
static symbol s_5_11[6] = { 'a', 'm', 'e', 'n', 't', 'e' };
static symbol s_5_12[4] = { 225, 'v', 'e', 'l' };
static symbol s_5_13[4] = { 237, 'v', 'e', 'l' };
static symbol s_5_14[5] = { 'u', 'c', 'i', 243, 'n' };
static symbol s_5_15[3] = { 'i', 'c', 'o' };
static symbol s_5_16[4] = { 'i', 's', 'm', 'o' };
static symbol s_5_17[3] = { 'o', 's', 'o' };
static symbol s_5_18[6] = { 'a', 'm', 'e', 'n', 't', 'o' };
static symbol s_5_19[6] = { 'i', 'm', 'e', 'n', 't', 'o' };
static symbol s_5_20[3] = { 'i', 'v', 'o' };
static symbol s_5_21[5] = { 'a', 231, 'a', '~', 'o' };
static symbol s_5_22[4] = { 'a', 'd', 'o', 'r' };
static symbol s_5_23[4] = { 'i', 'c', 'a', 's' };
static symbol s_5_24[6] = { 234, 'n', 'c', 'i', 'a', 's' };
static symbol s_5_25[4] = { 'i', 'r', 'a', 's' };
static symbol s_5_26[6] = { 'a', 'd', 'o', 'r', 'a', 's' };
static symbol s_5_27[4] = { 'o', 's', 'a', 's' };
static symbol s_5_28[5] = { 'i', 's', 't', 'a', 's' };
static symbol s_5_29[4] = { 'i', 'v', 'a', 's' };
static symbol s_5_30[4] = { 'e', 'z', 'a', 's' };
static symbol s_5_31[6] = { 'l', 'o', 'g', 237, 'a', 's' };
static symbol s_5_32[6] = { 'i', 'd', 'a', 'd', 'e', 's' };
static symbol s_5_33[7] = { 'u', 'c', 'i', 'o', 'n', 'e', 's' };
static symbol s_5_34[6] = { 'a', 'd', 'o', 'r', 'e', 's' };
static symbol s_5_35[6] = { 'a', 231, 'o', '~', 'e', 's' };
static symbol s_5_36[4] = { 'i', 'c', 'o', 's' };
static symbol s_5_37[5] = { 'i', 's', 'm', 'o', 's' };
static symbol s_5_38[4] = { 'o', 's', 'o', 's' };
static symbol s_5_39[7] = { 'a', 'm', 'e', 'n', 't', 'o', 's' };
static symbol s_5_40[7] = { 'i', 'm', 'e', 'n', 't', 'o', 's' };
static symbol s_5_41[4] = { 'i', 'v', 'o', 's' };

static struct among a_5[42] =
{
/*  0 */ { 3, s_5_0, -1, 1, 0},
/*  1 */ { 5, s_5_1, -1, 4, 0},
/*  2 */ { 3, s_5_2, -1, 9, 0},
/*  3 */ { 5, s_5_3, -1, 1, 0},
/*  4 */ { 3, s_5_4, -1, 1, 0},
/*  5 */ { 4, s_5_5, -1, 1, 0},
/*  6 */ { 3, s_5_6, -1, 8, 0},
/*  7 */ { 3, s_5_7, -1, 1, 0},
/*  8 */ { 5, s_5_8, -1, 2, 0},
/*  9 */ { 5, s_5_9, -1, 7, 0},
/* 10 */ { 5, s_5_10, -1, 6, 0},
/* 11 */ { 6, s_5_11, 10, 5, 0},
/* 12 */ { 4, s_5_12, -1, 1, 0},
/* 13 */ { 4, s_5_13, -1, 1, 0},
/* 14 */ { 5, s_5_14, -1, 3, 0},
/* 15 */ { 3, s_5_15, -1, 1, 0},
/* 16 */ { 4, s_5_16, -1, 1, 0},
/* 17 */ { 3, s_5_17, -1, 1, 0},
/* 18 */ { 6, s_5_18, -1, 1, 0},
/* 19 */ { 6, s_5_19, -1, 1, 0},
/* 20 */ { 3, s_5_20, -1, 8, 0},
/* 21 */ { 5, s_5_21, -1, 1, 0},
/* 22 */ { 4, s_5_22, -1, 1, 0},
/* 23 */ { 4, s_5_23, -1, 1, 0},
/* 24 */ { 6, s_5_24, -1, 4, 0},
/* 25 */ { 4, s_5_25, -1, 9, 0},
/* 26 */ { 6, s_5_26, -1, 1, 0},
/* 27 */ { 4, s_5_27, -1, 1, 0},
/* 28 */ { 5, s_5_28, -1, 1, 0},
/* 29 */ { 4, s_5_29, -1, 8, 0},
/* 30 */ { 4, s_5_30, -1, 1, 0},
/* 31 */ { 6, s_5_31, -1, 2, 0},
/* 32 */ { 6, s_5_32, -1, 7, 0},
/* 33 */ { 7, s_5_33, -1, 3, 0},
/* 34 */ { 6, s_5_34, -1, 1, 0},
/* 35 */ { 6, s_5_35, -1, 1, 0},
/* 36 */ { 4, s_5_36, -1, 1, 0},
/* 37 */ { 5, s_5_37, -1, 1, 0},
/* 38 */ { 4, s_5_38, -1, 1, 0},
/* 39 */ { 7, s_5_39, -1, 1, 0},
/* 40 */ { 7, s_5_40, -1, 1, 0},
/* 41 */ { 4, s_5_41, -1, 8, 0}
};

static symbol s_6_0[3] = { 'a', 'd', 'a' };
static symbol s_6_1[3] = { 'i', 'd', 'a' };
static symbol s_6_2[2] = { 'i', 'a' };
static symbol s_6_3[4] = { 'a', 'r', 'i', 'a' };
static symbol s_6_4[4] = { 'e', 'r', 'i', 'a' };
static symbol s_6_5[4] = { 'i', 'r', 'i', 'a' };
static symbol s_6_6[3] = { 'a', 'r', 'a' };
static symbol s_6_7[3] = { 'e', 'r', 'a' };
static symbol s_6_8[3] = { 'i', 'r', 'a' };
static symbol s_6_9[3] = { 'a', 'v', 'a' };
static symbol s_6_10[4] = { 'a', 's', 's', 'e' };
static symbol s_6_11[4] = { 'e', 's', 's', 'e' };
static symbol s_6_12[4] = { 'i', 's', 's', 'e' };
static symbol s_6_13[4] = { 'a', 's', 't', 'e' };
static symbol s_6_14[4] = { 'e', 's', 't', 'e' };
static symbol s_6_15[4] = { 'i', 's', 't', 'e' };
static symbol s_6_16[2] = { 'e', 'i' };
static symbol s_6_17[4] = { 'a', 'r', 'e', 'i' };
static symbol s_6_18[4] = { 'e', 'r', 'e', 'i' };
static symbol s_6_19[4] = { 'i', 'r', 'e', 'i' };
static symbol s_6_20[2] = { 'a', 'm' };
static symbol s_6_21[3] = { 'i', 'a', 'm' };
static symbol s_6_22[5] = { 'a', 'r', 'i', 'a', 'm' };
static symbol s_6_23[5] = { 'e', 'r', 'i', 'a', 'm' };
static symbol s_6_24[5] = { 'i', 'r', 'i', 'a', 'm' };
static symbol s_6_25[4] = { 'a', 'r', 'a', 'm' };
static symbol s_6_26[4] = { 'e', 'r', 'a', 'm' };
static symbol s_6_27[4] = { 'i', 'r', 'a', 'm' };
static symbol s_6_28[4] = { 'a', 'v', 'a', 'm' };
static symbol s_6_29[2] = { 'e', 'm' };
static symbol s_6_30[4] = { 'a', 'r', 'e', 'm' };
static symbol s_6_31[4] = { 'e', 'r', 'e', 'm' };
static symbol s_6_32[4] = { 'i', 'r', 'e', 'm' };
static symbol s_6_33[5] = { 'a', 's', 's', 'e', 'm' };
static symbol s_6_34[5] = { 'e', 's', 's', 'e', 'm' };
static symbol s_6_35[5] = { 'i', 's', 's', 'e', 'm' };
static symbol s_6_36[3] = { 'a', 'd', 'o' };
static symbol s_6_37[3] = { 'i', 'd', 'o' };
static symbol s_6_38[4] = { 'a', 'n', 'd', 'o' };
static symbol s_6_39[4] = { 'e', 'n', 'd', 'o' };
static symbol s_6_40[4] = { 'i', 'n', 'd', 'o' };
static symbol s_6_41[5] = { 'a', 'r', 'a', '~', 'o' };
static symbol s_6_42[5] = { 'e', 'r', 'a', '~', 'o' };
static symbol s_6_43[5] = { 'i', 'r', 'a', '~', 'o' };
static symbol s_6_44[2] = { 'a', 'r' };
static symbol s_6_45[2] = { 'e', 'r' };
static symbol s_6_46[2] = { 'i', 'r' };
static symbol s_6_47[2] = { 'a', 's' };
static symbol s_6_48[4] = { 'a', 'd', 'a', 's' };
static symbol s_6_49[4] = { 'i', 'd', 'a', 's' };
static symbol s_6_50[3] = { 'i', 'a', 's' };
static symbol s_6_51[5] = { 'a', 'r', 'i', 'a', 's' };
static symbol s_6_52[5] = { 'e', 'r', 'i', 'a', 's' };
static symbol s_6_53[5] = { 'i', 'r', 'i', 'a', 's' };
static symbol s_6_54[4] = { 'a', 'r', 'a', 's' };
static symbol s_6_55[4] = { 'e', 'r', 'a', 's' };
static symbol s_6_56[4] = { 'i', 'r', 'a', 's' };
static symbol s_6_57[4] = { 'a', 'v', 'a', 's' };
static symbol s_6_58[2] = { 'e', 's' };
static symbol s_6_59[5] = { 'a', 'r', 'd', 'e', 's' };
static symbol s_6_60[5] = { 'e', 'r', 'd', 'e', 's' };
static symbol s_6_61[5] = { 'i', 'r', 'd', 'e', 's' };
static symbol s_6_62[4] = { 'a', 'r', 'e', 's' };
static symbol s_6_63[4] = { 'e', 'r', 'e', 's' };
static symbol s_6_64[4] = { 'i', 'r', 'e', 's' };
static symbol s_6_65[5] = { 'a', 's', 's', 'e', 's' };
static symbol s_6_66[5] = { 'e', 's', 's', 'e', 's' };
static symbol s_6_67[5] = { 'i', 's', 's', 'e', 's' };
static symbol s_6_68[5] = { 'a', 's', 't', 'e', 's' };
static symbol s_6_69[5] = { 'e', 's', 't', 'e', 's' };
static symbol s_6_70[5] = { 'i', 's', 't', 'e', 's' };
static symbol s_6_71[2] = { 'i', 's' };
static symbol s_6_72[3] = { 'a', 'i', 's' };
static symbol s_6_73[3] = { 'e', 'i', 's' };
static symbol s_6_74[5] = { 'a', 'r', 'e', 'i', 's' };
static symbol s_6_75[5] = { 'e', 'r', 'e', 'i', 's' };
static symbol s_6_76[5] = { 'i', 'r', 'e', 'i', 's' };
static symbol s_6_77[5] = { 225, 'r', 'e', 'i', 's' };
static symbol s_6_78[5] = { 233, 'r', 'e', 'i', 's' };
static symbol s_6_79[5] = { 237, 'r', 'e', 'i', 's' };
static symbol s_6_80[6] = { 225, 's', 's', 'e', 'i', 's' };
static symbol s_6_81[6] = { 233, 's', 's', 'e', 'i', 's' };
static symbol s_6_82[6] = { 237, 's', 's', 'e', 'i', 's' };
static symbol s_6_83[5] = { 225, 'v', 'e', 'i', 's' };
static symbol s_6_84[4] = { 237, 'e', 'i', 's' };
static symbol s_6_85[6] = { 'a', 'r', 237, 'e', 'i', 's' };
static symbol s_6_86[6] = { 'e', 'r', 237, 'e', 'i', 's' };
static symbol s_6_87[6] = { 'i', 'r', 237, 'e', 'i', 's' };
static symbol s_6_88[4] = { 'a', 'd', 'o', 's' };
static symbol s_6_89[4] = { 'i', 'd', 'o', 's' };
static symbol s_6_90[4] = { 'a', 'm', 'o', 's' };
static symbol s_6_91[6] = { 225, 'r', 'a', 'm', 'o', 's' };
static symbol s_6_92[6] = { 233, 'r', 'a', 'm', 'o', 's' };
static symbol s_6_93[6] = { 237, 'r', 'a', 'm', 'o', 's' };
static symbol s_6_94[6] = { 225, 'v', 'a', 'm', 'o', 's' };
static symbol s_6_95[5] = { 237, 'a', 'm', 'o', 's' };
static symbol s_6_96[7] = { 'a', 'r', 237, 'a', 'm', 'o', 's' };
static symbol s_6_97[7] = { 'e', 'r', 237, 'a', 'm', 'o', 's' };
static symbol s_6_98[7] = { 'i', 'r', 237, 'a', 'm', 'o', 's' };
static symbol s_6_99[4] = { 'e', 'm', 'o', 's' };
static symbol s_6_100[6] = { 'a', 'r', 'e', 'm', 'o', 's' };
static symbol s_6_101[6] = { 'e', 'r', 'e', 'm', 'o', 's' };
static symbol s_6_102[6] = { 'i', 'r', 'e', 'm', 'o', 's' };
static symbol s_6_103[7] = { 225, 's', 's', 'e', 'm', 'o', 's' };
static symbol s_6_104[7] = { 234, 's', 's', 'e', 'm', 'o', 's' };
static symbol s_6_105[7] = { 237, 's', 's', 'e', 'm', 'o', 's' };
static symbol s_6_106[4] = { 'i', 'm', 'o', 's' };
static symbol s_6_107[5] = { 'a', 'r', 'm', 'o', 's' };
static symbol s_6_108[5] = { 'e', 'r', 'm', 'o', 's' };
static symbol s_6_109[5] = { 'i', 'r', 'm', 'o', 's' };
static symbol s_6_110[4] = { 225, 'm', 'o', 's' };
static symbol s_6_111[4] = { 'a', 'r', 225, 's' };
static symbol s_6_112[4] = { 'e', 'r', 225, 's' };
static symbol s_6_113[4] = { 'i', 'r', 225, 's' };
static symbol s_6_114[2] = { 'e', 'u' };
static symbol s_6_115[2] = { 'i', 'u' };
static symbol s_6_116[2] = { 'o', 'u' };
static symbol s_6_117[3] = { 'a', 'r', 225 };
static symbol s_6_118[3] = { 'e', 'r', 225 };
static symbol s_6_119[3] = { 'i', 'r', 225 };

static struct among a_6[120] =
{
/*  0 */ { 3, s_6_0, -1, 1, 0},
/*  1 */ { 3, s_6_1, -1, 1, 0},
/*  2 */ { 2, s_6_2, -1, 1, 0},
/*  3 */ { 4, s_6_3, 2, 1, 0},
/*  4 */ { 4, s_6_4, 2, 1, 0},
/*  5 */ { 4, s_6_5, 2, 1, 0},
/*  6 */ { 3, s_6_6, -1, 1, 0},
/*  7 */ { 3, s_6_7, -1, 1, 0},
/*  8 */ { 3, s_6_8, -1, 1, 0},
/*  9 */ { 3, s_6_9, -1, 1, 0},
/* 10 */ { 4, s_6_10, -1, 1, 0},
/* 11 */ { 4, s_6_11, -1, 1, 0},
/* 12 */ { 4, s_6_12, -1, 1, 0},
/* 13 */ { 4, s_6_13, -1, 1, 0},
/* 14 */ { 4, s_6_14, -1, 1, 0},
/* 15 */ { 4, s_6_15, -1, 1, 0},
/* 16 */ { 2, s_6_16, -1, 1, 0},
/* 17 */ { 4, s_6_17, 16, 1, 0},
/* 18 */ { 4, s_6_18, 16, 1, 0},
/* 19 */ { 4, s_6_19, 16, 1, 0},
/* 20 */ { 2, s_6_20, -1, 1, 0},
/* 21 */ { 3, s_6_21, 20, 1, 0},
/* 22 */ { 5, s_6_22, 21, 1, 0},
/* 23 */ { 5, s_6_23, 21, 1, 0},
/* 24 */ { 5, s_6_24, 21, 1, 0},
/* 25 */ { 4, s_6_25, 20, 1, 0},
/* 26 */ { 4, s_6_26, 20, 1, 0},
/* 27 */ { 4, s_6_27, 20, 1, 0},
/* 28 */ { 4, s_6_28, 20, 1, 0},
/* 29 */ { 2, s_6_29, -1, 1, 0},
/* 30 */ { 4, s_6_30, 29, 1, 0},
/* 31 */ { 4, s_6_31, 29, 1, 0},
/* 32 */ { 4, s_6_32, 29, 1, 0},
/* 33 */ { 5, s_6_33, 29, 1, 0},
/* 34 */ { 5, s_6_34, 29, 1, 0},
/* 35 */ { 5, s_6_35, 29, 1, 0},
/* 36 */ { 3, s_6_36, -1, 1, 0},
/* 37 */ { 3, s_6_37, -1, 1, 0},
/* 38 */ { 4, s_6_38, -1, 1, 0},
/* 39 */ { 4, s_6_39, -1, 1, 0},
/* 40 */ { 4, s_6_40, -1, 1, 0},
/* 41 */ { 5, s_6_41, -1, 1, 0},
/* 42 */ { 5, s_6_42, -1, 1, 0},
/* 43 */ { 5, s_6_43, -1, 1, 0},
/* 44 */ { 2, s_6_44, -1, 1, 0},
/* 45 */ { 2, s_6_45, -1, 1, 0},
/* 46 */ { 2, s_6_46, -1, 1, 0},
/* 47 */ { 2, s_6_47, -1, 1, 0},
/* 48 */ { 4, s_6_48, 47, 1, 0},
/* 49 */ { 4, s_6_49, 47, 1, 0},
/* 50 */ { 3, s_6_50, 47, 1, 0},
/* 51 */ { 5, s_6_51, 50, 1, 0},
/* 52 */ { 5, s_6_52, 50, 1, 0},
/* 53 */ { 5, s_6_53, 50, 1, 0},
/* 54 */ { 4, s_6_54, 47, 1, 0},
/* 55 */ { 4, s_6_55, 47, 1, 0},
/* 56 */ { 4, s_6_56, 47, 1, 0},
/* 57 */ { 4, s_6_57, 47, 1, 0},
/* 58 */ { 2, s_6_58, -1, 1, 0},
/* 59 */ { 5, s_6_59, 58, 1, 0},
/* 60 */ { 5, s_6_60, 58, 1, 0},
/* 61 */ { 5, s_6_61, 58, 1, 0},
/* 62 */ { 4, s_6_62, 58, 1, 0},
/* 63 */ { 4, s_6_63, 58, 1, 0},
/* 64 */ { 4, s_6_64, 58, 1, 0},
/* 65 */ { 5, s_6_65, 58, 1, 0},
/* 66 */ { 5, s_6_66, 58, 1, 0},
/* 67 */ { 5, s_6_67, 58, 1, 0},
/* 68 */ { 5, s_6_68, 58, 1, 0},
/* 69 */ { 5, s_6_69, 58, 1, 0},
/* 70 */ { 5, s_6_70, 58, 1, 0},
/* 71 */ { 2, s_6_71, -1, 1, 0},
/* 72 */ { 3, s_6_72, 71, 1, 0},
/* 73 */ { 3, s_6_73, 71, 1, 0},
/* 74 */ { 5, s_6_74, 73, 1, 0},
/* 75 */ { 5, s_6_75, 73, 1, 0},
/* 76 */ { 5, s_6_76, 73, 1, 0},
/* 77 */ { 5, s_6_77, 73, 1, 0},
/* 78 */ { 5, s_6_78, 73, 1, 0},
/* 79 */ { 5, s_6_79, 73, 1, 0},
/* 80 */ { 6, s_6_80, 73, 1, 0},
/* 81 */ { 6, s_6_81, 73, 1, 0},
/* 82 */ { 6, s_6_82, 73, 1, 0},
/* 83 */ { 5, s_6_83, 73, 1, 0},
/* 84 */ { 4, s_6_84, 73, 1, 0},
/* 85 */ { 6, s_6_85, 84, 1, 0},
/* 86 */ { 6, s_6_86, 84, 1, 0},
/* 87 */ { 6, s_6_87, 84, 1, 0},
/* 88 */ { 4, s_6_88, -1, 1, 0},
/* 89 */ { 4, s_6_89, -1, 1, 0},
/* 90 */ { 4, s_6_90, -1, 1, 0},
/* 91 */ { 6, s_6_91, 90, 1, 0},
/* 92 */ { 6, s_6_92, 90, 1, 0},
/* 93 */ { 6, s_6_93, 90, 1, 0},
/* 94 */ { 6, s_6_94, 90, 1, 0},
/* 95 */ { 5, s_6_95, 90, 1, 0},
/* 96 */ { 7, s_6_96, 95, 1, 0},
/* 97 */ { 7, s_6_97, 95, 1, 0},
/* 98 */ { 7, s_6_98, 95, 1, 0},
/* 99 */ { 4, s_6_99, -1, 1, 0},
/*100 */ { 6, s_6_100, 99, 1, 0},
/*101 */ { 6, s_6_101, 99, 1, 0},
/*102 */ { 6, s_6_102, 99, 1, 0},
/*103 */ { 7, s_6_103, 99, 1, 0},
/*104 */ { 7, s_6_104, 99, 1, 0},
/*105 */ { 7, s_6_105, 99, 1, 0},
/*106 */ { 4, s_6_106, -1, 1, 0},
/*107 */ { 5, s_6_107, -1, 1, 0},
/*108 */ { 5, s_6_108, -1, 1, 0},
/*109 */ { 5, s_6_109, -1, 1, 0},
/*110 */ { 4, s_6_110, -1, 1, 0},
/*111 */ { 4, s_6_111, -1, 1, 0},
/*112 */ { 4, s_6_112, -1, 1, 0},
/*113 */ { 4, s_6_113, -1, 1, 0},
/*114 */ { 2, s_6_114, -1, 1, 0},
/*115 */ { 2, s_6_115, -1, 1, 0},
/*116 */ { 2, s_6_116, -1, 1, 0},
/*117 */ { 3, s_6_117, -1, 1, 0},
/*118 */ { 3, s_6_118, -1, 1, 0},
/*119 */ { 3, s_6_119, -1, 1, 0}
};

static symbol s_7_0[1] = { 'a' };
static symbol s_7_1[1] = { 'i' };
static symbol s_7_2[1] = { 'o' };
static symbol s_7_3[2] = { 'o', 's' };
static symbol s_7_4[1] = { 225 };
static symbol s_7_5[1] = { 237 };
static symbol s_7_6[1] = { 243 };

static struct among a_7[7] =
{
/*  0 */ { 1, s_7_0, -1, 1, 0},
/*  1 */ { 1, s_7_1, -1, 1, 0},
/*  2 */ { 1, s_7_2, -1, 1, 0},
/*  3 */ { 2, s_7_3, -1, 1, 0},
/*  4 */ { 1, s_7_4, -1, 1, 0},
/*  5 */ { 1, s_7_5, -1, 1, 0},
/*  6 */ { 1, s_7_6, -1, 1, 0}
};

static symbol s_8_0[1] = { 'e' };
static symbol s_8_1[1] = { 231 };
static symbol s_8_2[1] = { 233 };
static symbol s_8_3[1] = { 234 };

static struct among a_8[4] =
{
/*  0 */ { 1, s_8_0, -1, 1, 0},
/*  1 */ { 1, s_8_1, -1, 2, 0},
/*  2 */ { 1, s_8_2, -1, 1, 0},
/*  3 */ { 1, s_8_3, -1, 1, 0}
};

static unsigned char g_v[] = { 17, 65, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 19, 12, 2 };

static symbol s_0[] = { 'a', '~' };
static symbol s_1[] = { 'o', '~' };
static symbol s_2[] = { 227 };
static symbol s_3[] = { 245 };
static symbol s_4[] = { 'l', 'o', 'g' };
static symbol s_5[] = { 'u' };
static symbol s_6[] = { 'e', 'n', 't', 'e' };
static symbol s_7[] = { 'a', 't' };
static symbol s_8[] = { 'a', 't' };
static symbol s_9[] = { 'e' };
static symbol s_10[] = { 'i', 'r' };
static symbol s_11[] = { 'u' };
static symbol s_12[] = { 'g' };
static symbol s_13[] = { 'i' };
static symbol s_14[] = { 'c' };
static symbol s_15[] = { 'c' };
static symbol s_16[] = { 'i' };
static symbol s_17[] = { 'c' };

static int r_prelude(struct SN_env * z) {
    int among_var;
    while(1) { /* repeat, line 36 */
        int c = z->c;
        z->bra = z->c; /* [, line 37 */
        among_var = find_among(z, a_0, 3); /* substring, line 37 */
        if (!(among_var)) goto lab0;
        z->ket = z->c; /* ], line 37 */
        switch(among_var) {
            case 0: goto lab0;
            case 1:
                slice_from_s(z, 2, s_0); /* <-, line 38 */
                break;
            case 2:
                slice_from_s(z, 2, s_1); /* <-, line 39 */
                break;
            case 3:
                if (z->c >= z->l) goto lab0;
                z->c++; /* next, line 40 */
                break;
        }
        continue;
    lab0:
        z->c = c;
        break;
    }
    return 1;
}

static int r_mark_regions(struct SN_env * z) {
    z->I[0] = z->l;
    z->I[1] = z->l;
    z->I[2] = z->l;
    {   int c = z->c; /* do, line 50 */
        {   int c = z->c; /* or, line 52 */
            if (!(in_grouping(z, g_v, 97, 250))) goto lab2;
            {   int c = z->c; /* or, line 51 */
                if (!(out_grouping(z, g_v, 97, 250))) goto lab4;
                while(1) { /* gopast, line 51 */
                    if (!(in_grouping(z, g_v, 97, 250))) goto lab5;
                    break;
                lab5:
                    if (z->c >= z->l) goto lab4;
                    z->c++;
                }
                goto lab3;
            lab4:
                z->c = c;
                if (!(in_grouping(z, g_v, 97, 250))) goto lab2;
                while(1) { /* gopast, line 51 */
                    if (!(out_grouping(z, g_v, 97, 250))) goto lab6;
                    break;
                lab6:
                    if (z->c >= z->l) goto lab2;
                    z->c++;
                }
            }
        lab3:
            goto lab1;
        lab2:
            z->c = c;
            if (!(out_grouping(z, g_v, 97, 250))) goto lab0;
            {   int c = z->c; /* or, line 53 */
                if (!(out_grouping(z, g_v, 97, 250))) goto lab8;
                while(1) { /* gopast, line 53 */
                    if (!(in_grouping(z, g_v, 97, 250))) goto lab9;
                    break;
                lab9:
                    if (z->c >= z->l) goto lab8;
                    z->c++;
                }
                goto lab7;
            lab8:
                z->c = c;
                if (!(in_grouping(z, g_v, 97, 250))) goto lab0;
                if (z->c >= z->l) goto lab0;
                z->c++; /* next, line 53 */
            }
        lab7:
            ;
        }
    lab1:
        z->I[0] = z->c; /* setmark pV, line 54 */
    lab0:
        z->c = c;
    }
    {   int c = z->c; /* do, line 56 */
        while(1) { /* gopast, line 57 */
            if (!(in_grouping(z, g_v, 97, 250))) goto lab11;
            break;
        lab11:
            if (z->c >= z->l) goto lab10;
            z->c++;
        }
        while(1) { /* gopast, line 57 */
            if (!(out_grouping(z, g_v, 97, 250))) goto lab12;
            break;
        lab12:
            if (z->c >= z->l) goto lab10;
            z->c++;
        }
        z->I[1] = z->c; /* setmark p1, line 57 */
        while(1) { /* gopast, line 58 */
            if (!(in_grouping(z, g_v, 97, 250))) goto lab13;
            break;
        lab13:
            if (z->c >= z->l) goto lab10;
            z->c++;
        }
        while(1) { /* gopast, line 58 */
            if (!(out_grouping(z, g_v, 97, 250))) goto lab14;
            break;
        lab14:
            if (z->c >= z->l) goto lab10;
            z->c++;
        }
        z->I[2] = z->c; /* setmark p2, line 58 */
    lab10:
        z->c = c;
    }
    return 1;
}

static int r_postlude(struct SN_env * z) {
    int among_var;
    while(1) { /* repeat, line 62 */
        int c = z->c;
        z->bra = z->c; /* [, line 63 */
        among_var = find_among(z, a_1, 3); /* substring, line 63 */
        if (!(among_var)) goto lab0;
        z->ket = z->c; /* ], line 63 */
        switch(among_var) {
            case 0: goto lab0;
            case 1:
                slice_from_s(z, 1, s_2); /* <-, line 64 */
                break;
            case 2:
                slice_from_s(z, 1, s_3); /* <-, line 65 */
                break;
            case 3:
                if (z->c >= z->l) goto lab0;
                z->c++; /* next, line 66 */
                break;
        }
        continue;
    lab0:
        z->c = c;
        break;
    }
    return 1;
}

static int r_RV(struct SN_env * z) {
    if (!(z->I[0] <= z->c)) return 0;
    return 1;
}

static int r_R1(struct SN_env * z) {
    if (!(z->I[1] <= z->c)) return 0;
    return 1;
}

static int r_R2(struct SN_env * z) {
    if (!(z->I[2] <= z->c)) return 0;
    return 1;
}

static int r_standard_suffix(struct SN_env * z) {
    int among_var;
    z->ket = z->c; /* [, line 77 */
    among_var = find_among_b(z, a_5, 42); /* substring, line 77 */
    if (!(among_var)) return 0;
    z->bra = z->c; /* ], line 77 */
    switch(among_var) {
        case 0: return 0;
        case 1:
            if (!r_R2(z)) return 0; /* call R2, line 92 */
            slice_del(z); /* delete, line 92 */
            break;
        case 2:
            if (!r_R2(z)) return 0; /* call R2, line 97 */
            slice_from_s(z, 3, s_4); /* <-, line 97 */
            break;
        case 3:
            if (!r_R2(z)) return 0; /* call R2, line 101 */
            slice_from_s(z, 1, s_5); /* <-, line 101 */
            break;
        case 4:
            if (!r_R2(z)) return 0; /* call R2, line 105 */
            slice_from_s(z, 4, s_6); /* <-, line 105 */
            break;
        case 5:
            if (!r_R1(z)) return 0; /* call R1, line 109 */
            slice_del(z); /* delete, line 109 */
            {   int m = z->l - z->c; /* try, line 110 */
                z->ket = z->c; /* [, line 111 */
                among_var = find_among_b(z, a_2, 4); /* substring, line 111 */
                if (!(among_var)) { z->c = z->l - m; goto lab0; }
                z->bra = z->c; /* ], line 111 */
                if (!r_R2(z)) { z->c = z->l - m; goto lab0; } /* call R2, line 111 */
                slice_del(z); /* delete, line 111 */
                switch(among_var) {
                    case 0: { z->c = z->l - m; goto lab0; }
                    case 1:
                        z->ket = z->c; /* [, line 112 */
                        if (!(eq_s_b(z, 2, s_7))) { z->c = z->l - m; goto lab0; }
                        z->bra = z->c; /* ], line 112 */
                        if (!r_R2(z)) { z->c = z->l - m; goto lab0; } /* call R2, line 112 */
                        slice_del(z); /* delete, line 112 */
                        break;
                }
            lab0:
                ;
            }
            break;
        case 6:
            if (!r_R2(z)) return 0; /* call R2, line 121 */
            slice_del(z); /* delete, line 121 */
            {   int m = z->l - z->c; /* try, line 122 */
                z->ket = z->c; /* [, line 123 */
                among_var = find_among_b(z, a_3, 2); /* substring, line 123 */
                if (!(among_var)) { z->c = z->l - m; goto lab1; }
                z->bra = z->c; /* ], line 123 */
                switch(among_var) {
                    case 0: { z->c = z->l - m; goto lab1; }
                    case 1:
                        if (!r_R2(z)) { z->c = z->l - m; goto lab1; } /* call R2, line 125 */
                        slice_del(z); /* delete, line 125 */
                        break;
                }
            lab1:
                ;
            }
            break;
        case 7:
            if (!r_R2(z)) return 0; /* call R2, line 132 */
            slice_del(z); /* delete, line 132 */
            {   int m = z->l - z->c; /* try, line 133 */
                z->ket = z->c; /* [, line 134 */
                among_var = find_among_b(z, a_4, 3); /* substring, line 134 */
                if (!(among_var)) { z->c = z->l - m; goto lab2; }
                z->bra = z->c; /* ], line 134 */
                switch(among_var) {
                    case 0: { z->c = z->l - m; goto lab2; }
                    case 1:
                        if (!r_R2(z)) { z->c = z->l - m; goto lab2; } /* call R2, line 137 */
                        slice_del(z); /* delete, line 137 */
                        break;
                }
            lab2:
                ;
            }
            break;
        case 8:
            if (!r_R2(z)) return 0; /* call R2, line 144 */
            slice_del(z); /* delete, line 144 */
            {   int m = z->l - z->c; /* try, line 145 */
                z->ket = z->c; /* [, line 146 */
                if (!(eq_s_b(z, 2, s_8))) { z->c = z->l - m; goto lab3; }
                z->bra = z->c; /* ], line 146 */
                if (!r_R2(z)) { z->c = z->l - m; goto lab3; } /* call R2, line 146 */
                slice_del(z); /* delete, line 146 */
            lab3:
                ;
            }
            break;
        case 9:
            if (!r_RV(z)) return 0; /* call RV, line 151 */
            if (!(eq_s_b(z, 1, s_9))) return 0;
            slice_from_s(z, 2, s_10); /* <-, line 152 */
            break;
    }
    return 1;
}

static int r_verb_suffix(struct SN_env * z) {
    int among_var;
    {   int m = z->l - z->c; /* setlimit, line 157 */
        int m3;
        if (z->c < z->I[0]) return 0;
        z->c = z->I[0]; /* tomark, line 157 */
        m3 = z->lb; z->lb = z->c;
        z->c = z->l - m;
        z->ket = z->c; /* [, line 158 */
        among_var = find_among_b(z, a_6, 120); /* substring, line 158 */
        if (!(among_var)) { z->lb = m3; return 0; }
        z->bra = z->c; /* ], line 158 */
        switch(among_var) {
            case 0: { z->lb = m3; return 0; }
            case 1:
                slice_del(z); /* delete, line 177 */
                break;
        }
        z->lb = m3;
    }
    return 1;
}

static int r_residual_suffix(struct SN_env * z) {
    int among_var;
    z->ket = z->c; /* [, line 182 */
    among_var = find_among_b(z, a_7, 7); /* substring, line 182 */
    if (!(among_var)) return 0;
    z->bra = z->c; /* ], line 182 */
    switch(among_var) {
        case 0: return 0;
        case 1:
            if (!r_RV(z)) return 0; /* call RV, line 185 */
            slice_del(z); /* delete, line 185 */
            break;
    }
    return 1;
}

static int r_residual_form(struct SN_env * z) {
    int among_var;
    z->ket = z->c; /* [, line 190 */
    among_var = find_among_b(z, a_8, 4); /* substring, line 190 */
    if (!(among_var)) return 0;
    z->bra = z->c; /* ], line 190 */
    switch(among_var) {
        case 0: return 0;
        case 1:
            if (!r_RV(z)) return 0; /* call RV, line 192 */
            slice_del(z); /* delete, line 192 */
            z->ket = z->c; /* [, line 192 */
            {   int m = z->l - z->c; /* or, line 192 */
                if (!(eq_s_b(z, 1, s_11))) goto lab1;
                z->bra = z->c; /* ], line 192 */
                {   int m_test = z->l - z->c; /* test, line 192 */
                    if (!(eq_s_b(z, 1, s_12))) goto lab1;
                    z->c = z->l - m_test;
                }
                goto lab0;
            lab1:
                z->c = z->l - m;
                if (!(eq_s_b(z, 1, s_13))) return 0;
                z->bra = z->c; /* ], line 193 */
                {   int m_test = z->l - z->c; /* test, line 193 */
                    if (!(eq_s_b(z, 1, s_14))) return 0;
                    z->c = z->l - m_test;
                }
            }
        lab0:
            if (!r_RV(z)) return 0; /* call RV, line 193 */
            slice_del(z); /* delete, line 193 */
            break;
        case 2:
            slice_from_s(z, 1, s_15); /* <-, line 194 */
            break;
    }
    return 1;
}

extern int snowball_portuguese_stem(struct SN_env * z) {
    {   int c = z->c; /* do, line 200 */
        if (!r_prelude(z)) goto lab0; /* call prelude, line 200 */
    lab0:
        z->c = c;
    }
    {   int c = z->c; /* do, line 201 */
        if (!r_mark_regions(z)) goto lab1; /* call mark_regions, line 201 */
    lab1:
        z->c = c;
    }
    z->lb = z->c; z->c = z->l; /* backwards, line 202 */

    {   int m = z->l - z->c; /* do, line 203 */
        {   int m = z->l - z->c; /* or, line 207 */
            {   int m = z->l - z->c; /* or, line 204 */
                if (!r_standard_suffix(z)) goto lab6; /* call standard_suffix, line 204 */
                goto lab5;
            lab6:
                z->c = z->l - m;
                if (!r_verb_suffix(z)) goto lab4; /* call verb_suffix, line 204 */
            }
        lab5:
            {   int m = z->l - z->c; /* do, line 205 */
                z->ket = z->c; /* [, line 205 */
                if (!(eq_s_b(z, 1, s_16))) goto lab7;
                z->bra = z->c; /* ], line 205 */
                {   int m_test = z->l - z->c; /* test, line 205 */
                    if (!(eq_s_b(z, 1, s_17))) goto lab7;
                    z->c = z->l - m_test;
                }
                if (!r_RV(z)) goto lab7; /* call RV, line 205 */
                slice_del(z); /* delete, line 205 */
            lab7:
                z->c = z->l - m;
            }
            goto lab3;
        lab4:
            z->c = z->l - m;
            if (!r_residual_suffix(z)) goto lab2; /* call residual_suffix, line 207 */
        }
    lab3:
    lab2:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; /* do, line 209 */
        if (!r_residual_form(z)) goto lab8; /* call residual_form, line 209 */
    lab8:
        z->c = z->l - m;
    }
    z->c = z->lb;
    {   int c = z->c; /* do, line 211 */
        if (!r_postlude(z)) goto lab9; /* call postlude, line 211 */
    lab9:
        z->c = c;
    }
    return 1;
}

extern struct SN_env * snowball_portuguese_create_env(void) { return SN_create_env(0, 3, 0); }

extern void snowball_portuguese_close_env(struct SN_env * z) { SN_close_env(z); }

