
/* This file was generated automatically by the Snowball to ANSI C compiler */

#include "header.h"

extern int snowball_spanish_stem(struct SN_env * z);
static int r_residual_suffix(struct SN_env * z);
static int r_verb_suffix(struct SN_env * z);
static int r_y_verb_suffix(struct SN_env * z);
static int r_standard_suffix(struct SN_env * z);
static int r_attached_pronoun(struct SN_env * z);
static int r_R2(struct SN_env * z);
static int r_R1(struct SN_env * z);
static int r_RV(struct SN_env * z);
static int r_mark_regions(struct SN_env * z);
static int r_postlude(struct SN_env * z);

static symbol s_0_1[1] = { 225 };
static symbol s_0_2[1] = { 233 };
static symbol s_0_3[1] = { 237 };
static symbol s_0_4[1] = { 243 };
static symbol s_0_5[1] = { 250 };

static struct among a_0[6] =
{
/*  0 */ { 0, 0, -1, 6, 0},
/*  1 */ { 1, s_0_1, 0, 1, 0},
/*  2 */ { 1, s_0_2, 0, 2, 0},
/*  3 */ { 1, s_0_3, 0, 3, 0},
/*  4 */ { 1, s_0_4, 0, 4, 0},
/*  5 */ { 1, s_0_5, 0, 5, 0}
};

static symbol s_1_0[2] = { 'l', 'a' };
static symbol s_1_1[4] = { 's', 'e', 'l', 'a' };
static symbol s_1_2[2] = { 'l', 'e' };
static symbol s_1_3[2] = { 'm', 'e' };
static symbol s_1_4[2] = { 's', 'e' };
static symbol s_1_5[2] = { 'l', 'o' };
static symbol s_1_6[4] = { 's', 'e', 'l', 'o' };
static symbol s_1_7[3] = { 'l', 'a', 's' };
static symbol s_1_8[5] = { 's', 'e', 'l', 'a', 's' };
static symbol s_1_9[3] = { 'l', 'e', 's' };
static symbol s_1_10[3] = { 'l', 'o', 's' };
static symbol s_1_11[5] = { 's', 'e', 'l', 'o', 's' };
static symbol s_1_12[3] = { 'n', 'o', 's' };

static struct among a_1[13] =
{
/*  0 */ { 2, s_1_0, -1, -1, 0},
/*  1 */ { 4, s_1_1, 0, -1, 0},
/*  2 */ { 2, s_1_2, -1, -1, 0},
/*  3 */ { 2, s_1_3, -1, -1, 0},
/*  4 */ { 2, s_1_4, -1, -1, 0},
/*  5 */ { 2, s_1_5, -1, -1, 0},
/*  6 */ { 4, s_1_6, 5, -1, 0},
/*  7 */ { 3, s_1_7, -1, -1, 0},
/*  8 */ { 5, s_1_8, 7, -1, 0},
/*  9 */ { 3, s_1_9, -1, -1, 0},
/* 10 */ { 3, s_1_10, -1, -1, 0},
/* 11 */ { 5, s_1_11, 10, -1, 0},
/* 12 */ { 3, s_1_12, -1, -1, 0}
};

static symbol s_2_0[4] = { 'a', 'n', 'd', 'o' };
static symbol s_2_1[5] = { 'i', 'e', 'n', 'd', 'o' };
static symbol s_2_2[5] = { 'y', 'e', 'n', 'd', 'o' };
static symbol s_2_3[4] = { 225, 'n', 'd', 'o' };
static symbol s_2_4[5] = { 'i', 233, 'n', 'd', 'o' };
static symbol s_2_5[2] = { 'a', 'r' };
static symbol s_2_6[2] = { 'e', 'r' };
static symbol s_2_7[2] = { 'i', 'r' };
static symbol s_2_8[2] = { 225, 'r' };
static symbol s_2_9[2] = { 233, 'r' };
static symbol s_2_10[2] = { 237, 'r' };

static struct among a_2[11] =
{
/*  0 */ { 4, s_2_0, -1, 6, 0},
/*  1 */ { 5, s_2_1, -1, 6, 0},
/*  2 */ { 5, s_2_2, -1, 7, 0},
/*  3 */ { 4, s_2_3, -1, 2, 0},
/*  4 */ { 5, s_2_4, -1, 1, 0},
/*  5 */ { 2, s_2_5, -1, 6, 0},
/*  6 */ { 2, s_2_6, -1, 6, 0},
/*  7 */ { 2, s_2_7, -1, 6, 0},
/*  8 */ { 2, s_2_8, -1, 3, 0},
/*  9 */ { 2, s_2_9, -1, 4, 0},
/* 10 */ { 2, s_2_10, -1, 5, 0}
};

static symbol s_3_0[2] = { 'i', 'c' };
static symbol s_3_1[2] = { 'a', 'd' };
static symbol s_3_2[2] = { 'o', 's' };
static symbol s_3_3[2] = { 'i', 'v' };

static struct among a_3[4] =
{
/*  0 */ { 2, s_3_0, -1, -1, 0},
/*  1 */ { 2, s_3_1, -1, -1, 0},
/*  2 */ { 2, s_3_2, -1, -1, 0},
/*  3 */ { 2, s_3_3, -1, 1, 0}
};

static symbol s_4_0[4] = { 'a', 'b', 'l', 'e' };
static symbol s_4_1[4] = { 'i', 'b', 'l', 'e' };

static struct among a_4[2] =
{
/*  0 */ { 4, s_4_0, -1, 1, 0},
/*  1 */ { 4, s_4_1, -1, 1, 0}
};

static symbol s_5_0[2] = { 'i', 'c' };
static symbol s_5_1[4] = { 'a', 'b', 'i', 'l' };
static symbol s_5_2[2] = { 'i', 'v' };

static struct among a_5[3] =
{
/*  0 */ { 2, s_5_0, -1, 1, 0},
/*  1 */ { 4, s_5_1, -1, 1, 0},
/*  2 */ { 2, s_5_2, -1, 1, 0}
};

static symbol s_6_0[3] = { 'i', 'c', 'a' };
static symbol s_6_1[5] = { 'e', 'n', 'c', 'i', 'a' };
static symbol s_6_2[5] = { 'a', 'd', 'o', 'r', 'a' };
static symbol s_6_3[3] = { 'o', 's', 'a' };
static symbol s_6_4[4] = { 'i', 's', 't', 'a' };
static symbol s_6_5[3] = { 'i', 'v', 'a' };
static symbol s_6_6[4] = { 'a', 'n', 'z', 'a' };
static symbol s_6_7[5] = { 'l', 'o', 'g', 237, 'a' };
static symbol s_6_8[4] = { 'i', 'd', 'a', 'd' };
static symbol s_6_9[4] = { 'a', 'b', 'l', 'e' };
static symbol s_6_10[4] = { 'i', 'b', 'l', 'e' };
static symbol s_6_11[5] = { 'm', 'e', 'n', 't', 'e' };
static symbol s_6_12[6] = { 'a', 'm', 'e', 'n', 't', 'e' };
static symbol s_6_13[5] = { 'a', 'c', 'i', 243, 'n' };
static symbol s_6_14[5] = { 'u', 'c', 'i', 243, 'n' };
static symbol s_6_15[3] = { 'i', 'c', 'o' };
static symbol s_6_16[4] = { 'i', 's', 'm', 'o' };
static symbol s_6_17[3] = { 'o', 's', 'o' };
static symbol s_6_18[7] = { 'a', 'm', 'i', 'e', 'n', 't', 'o' };
static symbol s_6_19[7] = { 'i', 'm', 'i', 'e', 'n', 't', 'o' };
static symbol s_6_20[3] = { 'i', 'v', 'o' };
static symbol s_6_21[4] = { 'a', 'd', 'o', 'r' };
static symbol s_6_22[4] = { 'i', 'c', 'a', 's' };
static symbol s_6_23[6] = { 'e', 'n', 'c', 'i', 'a', 's' };
static symbol s_6_24[6] = { 'a', 'd', 'o', 'r', 'a', 's' };
static symbol s_6_25[4] = { 'o', 's', 'a', 's' };
static symbol s_6_26[5] = { 'i', 's', 't', 'a', 's' };
static symbol s_6_27[4] = { 'i', 'v', 'a', 's' };
static symbol s_6_28[5] = { 'a', 'n', 'z', 'a', 's' };
static symbol s_6_29[6] = { 'l', 'o', 'g', 237, 'a', 's' };
static symbol s_6_30[6] = { 'i', 'd', 'a', 'd', 'e', 's' };
static symbol s_6_31[5] = { 'a', 'b', 'l', 'e', 's' };
static symbol s_6_32[5] = { 'i', 'b', 'l', 'e', 's' };
static symbol s_6_33[7] = { 'a', 'c', 'i', 'o', 'n', 'e', 's' };
static symbol s_6_34[7] = { 'u', 'c', 'i', 'o', 'n', 'e', 's' };
static symbol s_6_35[6] = { 'a', 'd', 'o', 'r', 'e', 's' };
static symbol s_6_36[4] = { 'i', 'c', 'o', 's' };
static symbol s_6_37[5] = { 'i', 's', 'm', 'o', 's' };
static symbol s_6_38[4] = { 'o', 's', 'o', 's' };
static symbol s_6_39[8] = { 'a', 'm', 'i', 'e', 'n', 't', 'o', 's' };
static symbol s_6_40[8] = { 'i', 'm', 'i', 'e', 'n', 't', 'o', 's' };
static symbol s_6_41[4] = { 'i', 'v', 'o', 's' };

static struct among a_6[42] =
{
/*  0 */ { 3, s_6_0, -1, 1, 0},
/*  1 */ { 5, s_6_1, -1, 5, 0},
/*  2 */ { 5, s_6_2, -1, 2, 0},
/*  3 */ { 3, s_6_3, -1, 1, 0},
/*  4 */ { 4, s_6_4, -1, 1, 0},
/*  5 */ { 3, s_6_5, -1, 9, 0},
/*  6 */ { 4, s_6_6, -1, 1, 0},
/*  7 */ { 5, s_6_7, -1, 3, 0},
/*  8 */ { 4, s_6_8, -1, 8, 0},
/*  9 */ { 4, s_6_9, -1, 1, 0},
/* 10 */ { 4, s_6_10, -1, 1, 0},
/* 11 */ { 5, s_6_11, -1, 7, 0},
/* 12 */ { 6, s_6_12, 11, 6, 0},
/* 13 */ { 5, s_6_13, -1, 2, 0},
/* 14 */ { 5, s_6_14, -1, 4, 0},
/* 15 */ { 3, s_6_15, -1, 1, 0},
/* 16 */ { 4, s_6_16, -1, 1, 0},
/* 17 */ { 3, s_6_17, -1, 1, 0},
/* 18 */ { 7, s_6_18, -1, 1, 0},
/* 19 */ { 7, s_6_19, -1, 1, 0},
/* 20 */ { 3, s_6_20, -1, 9, 0},
/* 21 */ { 4, s_6_21, -1, 2, 0},
/* 22 */ { 4, s_6_22, -1, 1, 0},
/* 23 */ { 6, s_6_23, -1, 5, 0},
/* 24 */ { 6, s_6_24, -1, 2, 0},
/* 25 */ { 4, s_6_25, -1, 1, 0},
/* 26 */ { 5, s_6_26, -1, 1, 0},
/* 27 */ { 4, s_6_27, -1, 9, 0},
/* 28 */ { 5, s_6_28, -1, 1, 0},
/* 29 */ { 6, s_6_29, -1, 3, 0},
/* 30 */ { 6, s_6_30, -1, 8, 0},
/* 31 */ { 5, s_6_31, -1, 1, 0},
/* 32 */ { 5, s_6_32, -1, 1, 0},
/* 33 */ { 7, s_6_33, -1, 2, 0},
/* 34 */ { 7, s_6_34, -1, 4, 0},
/* 35 */ { 6, s_6_35, -1, 2, 0},
/* 36 */ { 4, s_6_36, -1, 1, 0},
/* 37 */ { 5, s_6_37, -1, 1, 0},
/* 38 */ { 4, s_6_38, -1, 1, 0},
/* 39 */ { 8, s_6_39, -1, 1, 0},
/* 40 */ { 8, s_6_40, -1, 1, 0},
/* 41 */ { 4, s_6_41, -1, 9, 0}
};

static symbol s_7_0[2] = { 'y', 'a' };
static symbol s_7_1[2] = { 'y', 'e' };
static symbol s_7_2[3] = { 'y', 'a', 'n' };
static symbol s_7_3[3] = { 'y', 'e', 'n' };
static symbol s_7_4[5] = { 'y', 'e', 'r', 'o', 'n' };
static symbol s_7_5[5] = { 'y', 'e', 'n', 'd', 'o' };
static symbol s_7_6[2] = { 'y', 'o' };
static symbol s_7_7[3] = { 'y', 'a', 's' };
static symbol s_7_8[3] = { 'y', 'e', 's' };
static symbol s_7_9[4] = { 'y', 'a', 'i', 's' };
static symbol s_7_10[5] = { 'y', 'a', 'm', 'o', 's' };
static symbol s_7_11[2] = { 'y', 243 };

static struct among a_7[12] =
{
/*  0 */ { 2, s_7_0, -1, 1, 0},
/*  1 */ { 2, s_7_1, -1, 1, 0},
/*  2 */ { 3, s_7_2, -1, 1, 0},
/*  3 */ { 3, s_7_3, -1, 1, 0},
/*  4 */ { 5, s_7_4, -1, 1, 0},
/*  5 */ { 5, s_7_5, -1, 1, 0},
/*  6 */ { 2, s_7_6, -1, 1, 0},
/*  7 */ { 3, s_7_7, -1, 1, 0},
/*  8 */ { 3, s_7_8, -1, 1, 0},
/*  9 */ { 4, s_7_9, -1, 1, 0},
/* 10 */ { 5, s_7_10, -1, 1, 0},
/* 11 */ { 2, s_7_11, -1, 1, 0}
};

static symbol s_8_0[3] = { 'a', 'b', 'a' };
static symbol s_8_1[3] = { 'a', 'd', 'a' };
static symbol s_8_2[3] = { 'i', 'd', 'a' };
static symbol s_8_3[3] = { 'a', 'r', 'a' };
static symbol s_8_4[4] = { 'i', 'e', 'r', 'a' };
static symbol s_8_5[2] = { 237, 'a' };
static symbol s_8_6[4] = { 'a', 'r', 237, 'a' };
static symbol s_8_7[4] = { 'e', 'r', 237, 'a' };
static symbol s_8_8[4] = { 'i', 'r', 237, 'a' };
static symbol s_8_9[2] = { 'a', 'd' };
static symbol s_8_10[2] = { 'e', 'd' };
static symbol s_8_11[2] = { 'i', 'd' };
static symbol s_8_12[3] = { 'a', 's', 'e' };
static symbol s_8_13[4] = { 'i', 'e', 's', 'e' };
static symbol s_8_14[4] = { 'a', 's', 't', 'e' };
static symbol s_8_15[4] = { 'i', 's', 't', 'e' };
static symbol s_8_16[2] = { 'a', 'n' };
static symbol s_8_17[4] = { 'a', 'b', 'a', 'n' };
static symbol s_8_18[4] = { 'a', 'r', 'a', 'n' };
static symbol s_8_19[5] = { 'i', 'e', 'r', 'a', 'n' };
static symbol s_8_20[3] = { 237, 'a', 'n' };
static symbol s_8_21[5] = { 'a', 'r', 237, 'a', 'n' };
static symbol s_8_22[5] = { 'e', 'r', 237, 'a', 'n' };
static symbol s_8_23[5] = { 'i', 'r', 237, 'a', 'n' };
static symbol s_8_24[2] = { 'e', 'n' };
static symbol s_8_25[4] = { 'a', 's', 'e', 'n' };
static symbol s_8_26[5] = { 'i', 'e', 's', 'e', 'n' };
static symbol s_8_27[4] = { 'a', 'r', 'o', 'n' };
static symbol s_8_28[5] = { 'i', 'e', 'r', 'o', 'n' };
static symbol s_8_29[4] = { 'a', 'r', 225, 'n' };
static symbol s_8_30[4] = { 'e', 'r', 225, 'n' };
static symbol s_8_31[4] = { 'i', 'r', 225, 'n' };
static symbol s_8_32[3] = { 'a', 'd', 'o' };
static symbol s_8_33[3] = { 'i', 'd', 'o' };
static symbol s_8_34[4] = { 'a', 'n', 'd', 'o' };
static symbol s_8_35[5] = { 'i', 'e', 'n', 'd', 'o' };
static symbol s_8_36[2] = { 'a', 'r' };
static symbol s_8_37[2] = { 'e', 'r' };
static symbol s_8_38[2] = { 'i', 'r' };
static symbol s_8_39[2] = { 'a', 's' };
static symbol s_8_40[4] = { 'a', 'b', 'a', 's' };
static symbol s_8_41[4] = { 'a', 'd', 'a', 's' };
static symbol s_8_42[4] = { 'i', 'd', 'a', 's' };
static symbol s_8_43[4] = { 'a', 'r', 'a', 's' };
static symbol s_8_44[5] = { 'i', 'e', 'r', 'a', 's' };
static symbol s_8_45[3] = { 237, 'a', 's' };
static symbol s_8_46[5] = { 'a', 'r', 237, 'a', 's' };
static symbol s_8_47[5] = { 'e', 'r', 237, 'a', 's' };
static symbol s_8_48[5] = { 'i', 'r', 237, 'a', 's' };
static symbol s_8_49[2] = { 'e', 's' };
static symbol s_8_50[4] = { 'a', 's', 'e', 's' };
static symbol s_8_51[5] = { 'i', 'e', 's', 'e', 's' };
static symbol s_8_52[5] = { 'a', 'b', 'a', 'i', 's' };
static symbol s_8_53[5] = { 'a', 'r', 'a', 'i', 's' };
static symbol s_8_54[6] = { 'i', 'e', 'r', 'a', 'i', 's' };
static symbol s_8_55[4] = { 237, 'a', 'i', 's' };
static symbol s_8_56[6] = { 'a', 'r', 237, 'a', 'i', 's' };
static symbol s_8_57[6] = { 'e', 'r', 237, 'a', 'i', 's' };
static symbol s_8_58[6] = { 'i', 'r', 237, 'a', 'i', 's' };
static symbol s_8_59[5] = { 'a', 's', 'e', 'i', 's' };
static symbol s_8_60[6] = { 'i', 'e', 's', 'e', 'i', 's' };
static symbol s_8_61[6] = { 'a', 's', 't', 'e', 'i', 's' };
static symbol s_8_62[6] = { 'i', 's', 't', 'e', 'i', 's' };
static symbol s_8_63[3] = { 225, 'i', 's' };
static symbol s_8_64[3] = { 233, 'i', 's' };
static symbol s_8_65[5] = { 'a', 'r', 233, 'i', 's' };
static symbol s_8_66[5] = { 'e', 'r', 233, 'i', 's' };
static symbol s_8_67[5] = { 'i', 'r', 233, 'i', 's' };
static symbol s_8_68[4] = { 'a', 'd', 'o', 's' };
static symbol s_8_69[4] = { 'i', 'd', 'o', 's' };
static symbol s_8_70[4] = { 'a', 'm', 'o', 's' };
static symbol s_8_71[6] = { 225, 'b', 'a', 'm', 'o', 's' };
static symbol s_8_72[6] = { 225, 'r', 'a', 'm', 'o', 's' };
static symbol s_8_73[7] = { 'i', 233, 'r', 'a', 'm', 'o', 's' };
static symbol s_8_74[5] = { 237, 'a', 'm', 'o', 's' };
static symbol s_8_75[7] = { 'a', 'r', 237, 'a', 'm', 'o', 's' };
static symbol s_8_76[7] = { 'e', 'r', 237, 'a', 'm', 'o', 's' };
static symbol s_8_77[7] = { 'i', 'r', 237, 'a', 'm', 'o', 's' };
static symbol s_8_78[4] = { 'e', 'm', 'o', 's' };
static symbol s_8_79[6] = { 'a', 'r', 'e', 'm', 'o', 's' };
static symbol s_8_80[6] = { 'e', 'r', 'e', 'm', 'o', 's' };
static symbol s_8_81[6] = { 'i', 'r', 'e', 'm', 'o', 's' };
static symbol s_8_82[6] = { 225, 's', 'e', 'm', 'o', 's' };
static symbol s_8_83[7] = { 'i', 233, 's', 'e', 'm', 'o', 's' };
static symbol s_8_84[4] = { 'i', 'm', 'o', 's' };
static symbol s_8_85[4] = { 'a', 'r', 225, 's' };
static symbol s_8_86[4] = { 'e', 'r', 225, 's' };
static symbol s_8_87[4] = { 'i', 'r', 225, 's' };
static symbol s_8_88[2] = { 237, 's' };
static symbol s_8_89[3] = { 'a', 'r', 225 };
static symbol s_8_90[3] = { 'e', 'r', 225 };
static symbol s_8_91[3] = { 'i', 'r', 225 };
static symbol s_8_92[3] = { 'a', 'r', 233 };
static symbol s_8_93[3] = { 'e', 'r', 233 };
static symbol s_8_94[3] = { 'i', 'r', 233 };
static symbol s_8_95[2] = { 'i', 243 };

static struct among a_8[96] =
{
/*  0 */ { 3, s_8_0, -1, 2, 0},
/*  1 */ { 3, s_8_1, -1, 2, 0},
/*  2 */ { 3, s_8_2, -1, 2, 0},
/*  3 */ { 3, s_8_3, -1, 2, 0},
/*  4 */ { 4, s_8_4, -1, 2, 0},
/*  5 */ { 2, s_8_5, -1, 2, 0},
/*  6 */ { 4, s_8_6, 5, 2, 0},
/*  7 */ { 4, s_8_7, 5, 2, 0},
/*  8 */ { 4, s_8_8, 5, 2, 0},
/*  9 */ { 2, s_8_9, -1, 2, 0},
/* 10 */ { 2, s_8_10, -1, 2, 0},
/* 11 */ { 2, s_8_11, -1, 2, 0},
/* 12 */ { 3, s_8_12, -1, 2, 0},
/* 13 */ { 4, s_8_13, -1, 2, 0},
/* 14 */ { 4, s_8_14, -1, 2, 0},
/* 15 */ { 4, s_8_15, -1, 2, 0},
/* 16 */ { 2, s_8_16, -1, 2, 0},
/* 17 */ { 4, s_8_17, 16, 2, 0},
/* 18 */ { 4, s_8_18, 16, 2, 0},
/* 19 */ { 5, s_8_19, 16, 2, 0},
/* 20 */ { 3, s_8_20, 16, 2, 0},
/* 21 */ { 5, s_8_21, 20, 2, 0},
/* 22 */ { 5, s_8_22, 20, 2, 0},
/* 23 */ { 5, s_8_23, 20, 2, 0},
/* 24 */ { 2, s_8_24, -1, 1, 0},
/* 25 */ { 4, s_8_25, 24, 2, 0},
/* 26 */ { 5, s_8_26, 24, 2, 0},
/* 27 */ { 4, s_8_27, -1, 2, 0},
/* 28 */ { 5, s_8_28, -1, 2, 0},
/* 29 */ { 4, s_8_29, -1, 2, 0},
/* 30 */ { 4, s_8_30, -1, 2, 0},
/* 31 */ { 4, s_8_31, -1, 2, 0},
/* 32 */ { 3, s_8_32, -1, 2, 0},
/* 33 */ { 3, s_8_33, -1, 2, 0},
/* 34 */ { 4, s_8_34, -1, 2, 0},
/* 35 */ { 5, s_8_35, -1, 2, 0},
/* 36 */ { 2, s_8_36, -1, 2, 0},
/* 37 */ { 2, s_8_37, -1, 2, 0},
/* 38 */ { 2, s_8_38, -1, 2, 0},
/* 39 */ { 2, s_8_39, -1, 2, 0},
/* 40 */ { 4, s_8_40, 39, 2, 0},
/* 41 */ { 4, s_8_41, 39, 2, 0},
/* 42 */ { 4, s_8_42, 39, 2, 0},
/* 43 */ { 4, s_8_43, 39, 2, 0},
/* 44 */ { 5, s_8_44, 39, 2, 0},
/* 45 */ { 3, s_8_45, 39, 2, 0},
/* 46 */ { 5, s_8_46, 45, 2, 0},
/* 47 */ { 5, s_8_47, 45, 2, 0},
/* 48 */ { 5, s_8_48, 45, 2, 0},
/* 49 */ { 2, s_8_49, -1, 1, 0},
/* 50 */ { 4, s_8_50, 49, 2, 0},
/* 51 */ { 5, s_8_51, 49, 2, 0},
/* 52 */ { 5, s_8_52, -1, 2, 0},
/* 53 */ { 5, s_8_53, -1, 2, 0},
/* 54 */ { 6, s_8_54, -1, 2, 0},
/* 55 */ { 4, s_8_55, -1, 2, 0},
/* 56 */ { 6, s_8_56, 55, 2, 0},
/* 57 */ { 6, s_8_57, 55, 2, 0},
/* 58 */ { 6, s_8_58, 55, 2, 0},
/* 59 */ { 5, s_8_59, -1, 2, 0},
/* 60 */ { 6, s_8_60, -1, 2, 0},
/* 61 */ { 6, s_8_61, -1, 2, 0},
/* 62 */ { 6, s_8_62, -1, 2, 0},
/* 63 */ { 3, s_8_63, -1, 2, 0},
/* 64 */ { 3, s_8_64, -1, 1, 0},
/* 65 */ { 5, s_8_65, 64, 2, 0},
/* 66 */ { 5, s_8_66, 64, 2, 0},
/* 67 */ { 5, s_8_67, 64, 2, 0},
/* 68 */ { 4, s_8_68, -1, 2, 0},
/* 69 */ { 4, s_8_69, -1, 2, 0},
/* 70 */ { 4, s_8_70, -1, 2, 0},
/* 71 */ { 6, s_8_71, 70, 2, 0},
/* 72 */ { 6, s_8_72, 70, 2, 0},
/* 73 */ { 7, s_8_73, 70, 2, 0},
/* 74 */ { 5, s_8_74, 70, 2, 0},
/* 75 */ { 7, s_8_75, 74, 2, 0},
/* 76 */ { 7, s_8_76, 74, 2, 0},
/* 77 */ { 7, s_8_77, 74, 2, 0},
/* 78 */ { 4, s_8_78, -1, 1, 0},
/* 79 */ { 6, s_8_79, 78, 2, 0},
/* 80 */ { 6, s_8_80, 78, 2, 0},
/* 81 */ { 6, s_8_81, 78, 2, 0},
/* 82 */ { 6, s_8_82, 78, 2, 0},
/* 83 */ { 7, s_8_83, 78, 2, 0},
/* 84 */ { 4, s_8_84, -1, 2, 0},
/* 85 */ { 4, s_8_85, -1, 2, 0},
/* 86 */ { 4, s_8_86, -1, 2, 0},
/* 87 */ { 4, s_8_87, -1, 2, 0},
/* 88 */ { 2, s_8_88, -1, 2, 0},
/* 89 */ { 3, s_8_89, -1, 2, 0},
/* 90 */ { 3, s_8_90, -1, 2, 0},
/* 91 */ { 3, s_8_91, -1, 2, 0},
/* 92 */ { 3, s_8_92, -1, 2, 0},
/* 93 */ { 3, s_8_93, -1, 2, 0},
/* 94 */ { 3, s_8_94, -1, 2, 0},
/* 95 */ { 2, s_8_95, -1, 2, 0}
};

static symbol s_9_0[1] = { 'a' };
static symbol s_9_1[1] = { 'e' };
static symbol s_9_2[1] = { 'o' };
static symbol s_9_3[2] = { 'o', 's' };
static symbol s_9_4[1] = { 225 };
static symbol s_9_5[1] = { 233 };
static symbol s_9_6[1] = { 237 };
static symbol s_9_7[1] = { 243 };

static struct among a_9[8] =
{
/*  0 */ { 1, s_9_0, -1, 1, 0},
/*  1 */ { 1, s_9_1, -1, 2, 0},
/*  2 */ { 1, s_9_2, -1, 1, 0},
/*  3 */ { 2, s_9_3, -1, 1, 0},
/*  4 */ { 1, s_9_4, -1, 1, 0},
/*  5 */ { 1, s_9_5, -1, 2, 0},
/*  6 */ { 1, s_9_6, -1, 1, 0},
/*  7 */ { 1, s_9_7, -1, 1, 0}
};

static unsigned char g_v[] = { 17, 65, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 17, 4, 10 };

static symbol s_0[] = { 'a' };
static symbol s_1[] = { 'e' };
static symbol s_2[] = { 'i' };
static symbol s_3[] = { 'o' };
static symbol s_4[] = { 'u' };
static symbol s_5[] = { 'i', 'e', 'n', 'd', 'o' };
static symbol s_6[] = { 'a', 'n', 'd', 'o' };
static symbol s_7[] = { 'a', 'r' };
static symbol s_8[] = { 'e', 'r' };
static symbol s_9[] = { 'i', 'r' };
static symbol s_10[] = { 'u' };
static symbol s_11[] = { 'i', 'c' };
static symbol s_12[] = { 'l', 'o', 'g' };
static symbol s_13[] = { 'u' };
static symbol s_14[] = { 'e', 'n', 't', 'e' };
static symbol s_15[] = { 'a', 't' };
static symbol s_16[] = { 'a', 't' };
static symbol s_17[] = { 'u' };
static symbol s_18[] = { 'u' };
static symbol s_19[] = { 'g' };
static symbol s_20[] = { 'u' };
static symbol s_21[] = { 'g' };

static int r_mark_regions(struct SN_env * z) {
    z->I[0] = z->l;
    z->I[1] = z->l;
    z->I[2] = z->l;
    {   int c = z->c; /* do, line 37 */
        {   int c = z->c; /* or, line 39 */
            if (!(in_grouping(z, g_v, 97, 252))) goto lab2;
            {   int c = z->c; /* or, line 38 */
                if (!(out_grouping(z, g_v, 97, 252))) goto lab4;
                while(1) { /* gopast, line 38 */
                    if (!(in_grouping(z, g_v, 97, 252))) goto lab5;
                    break;
                lab5:
                    if (z->c >= z->l) goto lab4;
                    z->c++;
                }
                goto lab3;
            lab4:
                z->c = c;
                if (!(in_grouping(z, g_v, 97, 252))) goto lab2;
                while(1) { /* gopast, line 38 */
                    if (!(out_grouping(z, g_v, 97, 252))) goto lab6;
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
            if (!(out_grouping(z, g_v, 97, 252))) goto lab0;
            {   int c = z->c; /* or, line 40 */
                if (!(out_grouping(z, g_v, 97, 252))) goto lab8;
                while(1) { /* gopast, line 40 */
                    if (!(in_grouping(z, g_v, 97, 252))) goto lab9;
                    break;
                lab9:
                    if (z->c >= z->l) goto lab8;
                    z->c++;
                }
                goto lab7;
            lab8:
                z->c = c;
                if (!(in_grouping(z, g_v, 97, 252))) goto lab0;
                if (z->c >= z->l) goto lab0;
                z->c++; /* next, line 40 */
            }
        lab7:
            ;
        }
    lab1:
        z->I[0] = z->c; /* setmark pV, line 41 */
    lab0:
        z->c = c;
    }
    {   int c = z->c; /* do, line 43 */
        while(1) { /* gopast, line 44 */
            if (!(in_grouping(z, g_v, 97, 252))) goto lab11;
            break;
        lab11:
            if (z->c >= z->l) goto lab10;
            z->c++;
        }
        while(1) { /* gopast, line 44 */
            if (!(out_grouping(z, g_v, 97, 252))) goto lab12;
            break;
        lab12:
            if (z->c >= z->l) goto lab10;
            z->c++;
        }
        z->I[1] = z->c; /* setmark p1, line 44 */
        while(1) { /* gopast, line 45 */
            if (!(in_grouping(z, g_v, 97, 252))) goto lab13;
            break;
        lab13:
            if (z->c >= z->l) goto lab10;
            z->c++;
        }
        while(1) { /* gopast, line 45 */
            if (!(out_grouping(z, g_v, 97, 252))) goto lab14;
            break;
        lab14:
            if (z->c >= z->l) goto lab10;
            z->c++;
        }
        z->I[2] = z->c; /* setmark p2, line 45 */
    lab10:
        z->c = c;
    }
    return 1;
}

static int r_postlude(struct SN_env * z) {
    int among_var;
    while(1) { /* repeat, line 49 */
        int c = z->c;
        z->bra = z->c; /* [, line 50 */
        among_var = find_among(z, a_0, 6); /* substring, line 50 */
        if (!(among_var)) goto lab0;
        z->ket = z->c; /* ], line 50 */
        switch(among_var) {
            case 0: goto lab0;
            case 1:
                slice_from_s(z, 1, s_0); /* <-, line 51 */
                break;
            case 2:
                slice_from_s(z, 1, s_1); /* <-, line 52 */
                break;
            case 3:
                slice_from_s(z, 1, s_2); /* <-, line 53 */
                break;
            case 4:
                slice_from_s(z, 1, s_3); /* <-, line 54 */
                break;
            case 5:
                slice_from_s(z, 1, s_4); /* <-, line 55 */
                break;
            case 6:
                if (z->c >= z->l) goto lab0;
                z->c++; /* next, line 57 */
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

static int r_attached_pronoun(struct SN_env * z) {
    int among_var;
    z->ket = z->c; /* [, line 68 */
    if (!(find_among_b(z, a_1, 13))) return 0; /* substring, line 68 */
    z->bra = z->c; /* ], line 68 */
    among_var = find_among_b(z, a_2, 11); /* substring, line 72 */
    if (!(among_var)) return 0;
    if (!r_RV(z)) return 0; /* call RV, line 72 */
    switch(among_var) {
        case 0: return 0;
        case 1:
            z->bra = z->c; /* ], line 73 */
            slice_from_s(z, 5, s_5); /* <-, line 73 */
            break;
        case 2:
            z->bra = z->c; /* ], line 74 */
            slice_from_s(z, 4, s_6); /* <-, line 74 */
            break;
        case 3:
            z->bra = z->c; /* ], line 75 */
            slice_from_s(z, 2, s_7); /* <-, line 75 */
            break;
        case 4:
            z->bra = z->c; /* ], line 76 */
            slice_from_s(z, 2, s_8); /* <-, line 76 */
            break;
        case 5:
            z->bra = z->c; /* ], line 77 */
            slice_from_s(z, 2, s_9); /* <-, line 77 */
            break;
        case 6:
            slice_del(z); /* delete, line 81 */
            break;
        case 7:
            if (!(eq_s_b(z, 1, s_10))) return 0;
            slice_del(z); /* delete, line 82 */
            break;
    }
    return 1;
}

static int r_standard_suffix(struct SN_env * z) {
    int among_var;
    z->ket = z->c; /* [, line 87 */
    among_var = find_among_b(z, a_6, 42); /* substring, line 87 */
    if (!(among_var)) return 0;
    z->bra = z->c; /* ], line 87 */
    switch(among_var) {
        case 0: return 0;
        case 1:
            if (!r_R2(z)) return 0; /* call R2, line 99 */
            slice_del(z); /* delete, line 99 */
            break;
        case 2:
            if (!r_R2(z)) return 0; /* call R2, line 104 */
            slice_del(z); /* delete, line 104 */
            {   int m = z->l - z->c; /* try, line 105 */
                z->ket = z->c; /* [, line 105 */
                if (!(eq_s_b(z, 2, s_11))) { z->c = z->l - m; goto lab0; }
                z->bra = z->c; /* ], line 105 */
                if (!r_R2(z)) { z->c = z->l - m; goto lab0; } /* call R2, line 105 */
                slice_del(z); /* delete, line 105 */
            lab0:
                ;
            }
            break;
        case 3:
            if (!r_R2(z)) return 0; /* call R2, line 110 */
            slice_from_s(z, 3, s_12); /* <-, line 110 */
            break;
        case 4:
            if (!r_R2(z)) return 0; /* call R2, line 114 */
            slice_from_s(z, 1, s_13); /* <-, line 114 */
            break;
        case 5:
            if (!r_R2(z)) return 0; /* call R2, line 118 */
            slice_from_s(z, 4, s_14); /* <-, line 118 */
            break;
        case 6:
            if (!r_R1(z)) return 0; /* call R1, line 122 */
            slice_del(z); /* delete, line 122 */
            {   int m = z->l - z->c; /* try, line 123 */
                z->ket = z->c; /* [, line 124 */
                among_var = find_among_b(z, a_3, 4); /* substring, line 124 */
                if (!(among_var)) { z->c = z->l - m; goto lab1; }
                z->bra = z->c; /* ], line 124 */
                if (!r_R2(z)) { z->c = z->l - m; goto lab1; } /* call R2, line 124 */
                slice_del(z); /* delete, line 124 */
                switch(among_var) {
                    case 0: { z->c = z->l - m; goto lab1; }
                    case 1:
                        z->ket = z->c; /* [, line 125 */
                        if (!(eq_s_b(z, 2, s_15))) { z->c = z->l - m; goto lab1; }
                        z->bra = z->c; /* ], line 125 */
                        if (!r_R2(z)) { z->c = z->l - m; goto lab1; } /* call R2, line 125 */
                        slice_del(z); /* delete, line 125 */
                        break;
                }
            lab1:
                ;
            }
            break;
        case 7:
            if (!r_R2(z)) return 0; /* call R2, line 134 */
            slice_del(z); /* delete, line 134 */
            {   int m = z->l - z->c; /* try, line 135 */
                z->ket = z->c; /* [, line 136 */
                among_var = find_among_b(z, a_4, 2); /* substring, line 136 */
                if (!(among_var)) { z->c = z->l - m; goto lab2; }
                z->bra = z->c; /* ], line 136 */
                switch(among_var) {
                    case 0: { z->c = z->l - m; goto lab2; }
                    case 1:
                        if (!r_R2(z)) { z->c = z->l - m; goto lab2; } /* call R2, line 138 */
                        slice_del(z); /* delete, line 138 */
                        break;
                }
            lab2:
                ;
            }
            break;
        case 8:
            if (!r_R2(z)) return 0; /* call R2, line 145 */
            slice_del(z); /* delete, line 145 */
            {   int m = z->l - z->c; /* try, line 146 */
                z->ket = z->c; /* [, line 147 */
                among_var = find_among_b(z, a_5, 3); /* substring, line 147 */
                if (!(among_var)) { z->c = z->l - m; goto lab3; }
                z->bra = z->c; /* ], line 147 */
                switch(among_var) {
                    case 0: { z->c = z->l - m; goto lab3; }
                    case 1:
                        if (!r_R2(z)) { z->c = z->l - m; goto lab3; } /* call R2, line 150 */
                        slice_del(z); /* delete, line 150 */
                        break;
                }
            lab3:
                ;
            }
            break;
        case 9:
            if (!r_R2(z)) return 0; /* call R2, line 157 */
            slice_del(z); /* delete, line 157 */
            {   int m = z->l - z->c; /* try, line 158 */
                z->ket = z->c; /* [, line 159 */
                if (!(eq_s_b(z, 2, s_16))) { z->c = z->l - m; goto lab4; }
                z->bra = z->c; /* ], line 159 */
                if (!r_R2(z)) { z->c = z->l - m; goto lab4; } /* call R2, line 159 */
                slice_del(z); /* delete, line 159 */
            lab4:
                ;
            }
            break;
    }
    return 1;
}

static int r_y_verb_suffix(struct SN_env * z) {
    int among_var;
    {   int m = z->l - z->c; /* setlimit, line 166 */
        int m3;
        if (z->c < z->I[0]) return 0;
        z->c = z->I[0]; /* tomark, line 166 */
        m3 = z->lb; z->lb = z->c;
        z->c = z->l - m;
        z->ket = z->c; /* [, line 166 */
        among_var = find_among_b(z, a_7, 12); /* substring, line 166 */
        if (!(among_var)) { z->lb = m3; return 0; }
        z->bra = z->c; /* ], line 166 */
        z->lb = m3;
    }
    switch(among_var) {
        case 0: return 0;
        case 1:
            if (!(eq_s_b(z, 1, s_17))) return 0;
            slice_del(z); /* delete, line 169 */
            break;
    }
    return 1;
}

static int r_verb_suffix(struct SN_env * z) {
    int among_var;
    {   int m = z->l - z->c; /* setlimit, line 174 */
        int m3;
        if (z->c < z->I[0]) return 0;
        z->c = z->I[0]; /* tomark, line 174 */
        m3 = z->lb; z->lb = z->c;
        z->c = z->l - m;
        z->ket = z->c; /* [, line 174 */
        among_var = find_among_b(z, a_8, 96); /* substring, line 174 */
        if (!(among_var)) { z->lb = m3; return 0; }
        z->bra = z->c; /* ], line 174 */
        z->lb = m3;
    }
    switch(among_var) {
        case 0: return 0;
        case 1:
            {   int m = z->l - z->c; /* try, line 177 */
                if (!(eq_s_b(z, 1, s_18))) { z->c = z->l - m; goto lab0; }
                {   int m_test = z->l - z->c; /* test, line 177 */
                    if (!(eq_s_b(z, 1, s_19))) { z->c = z->l - m; goto lab0; }
                    z->c = z->l - m_test;
                }
            lab0:
                ;
            }
            z->bra = z->c; /* ], line 177 */
            slice_del(z); /* delete, line 177 */
            break;
        case 2:
            slice_del(z); /* delete, line 198 */
            break;
    }
    return 1;
}

static int r_residual_suffix(struct SN_env * z) {
    int among_var;
    z->ket = z->c; /* [, line 203 */
    among_var = find_among_b(z, a_9, 8); /* substring, line 203 */
    if (!(among_var)) return 0;
    z->bra = z->c; /* ], line 203 */
    switch(among_var) {
        case 0: return 0;
        case 1:
            if (!r_RV(z)) return 0; /* call RV, line 206 */
            slice_del(z); /* delete, line 206 */
            break;
        case 2:
            if (!r_RV(z)) return 0; /* call RV, line 208 */
            slice_del(z); /* delete, line 208 */
            {   int m = z->l - z->c; /* try, line 208 */
                z->ket = z->c; /* [, line 208 */
                if (!(eq_s_b(z, 1, s_20))) { z->c = z->l - m; goto lab0; }
                z->bra = z->c; /* ], line 208 */
                {   int m_test = z->l - z->c; /* test, line 208 */
                    if (!(eq_s_b(z, 1, s_21))) { z->c = z->l - m; goto lab0; }
                    z->c = z->l - m_test;
                }
                if (!r_RV(z)) { z->c = z->l - m; goto lab0; } /* call RV, line 208 */
                slice_del(z); /* delete, line 208 */
            lab0:
                ;
            }
            break;
    }
    return 1;
}

extern int snowball_spanish_stem(struct SN_env * z) {
    {   int c = z->c; /* do, line 214 */
        if (!r_mark_regions(z)) goto lab0; /* call mark_regions, line 214 */
    lab0:
        z->c = c;
    }
    z->lb = z->c; z->c = z->l; /* backwards, line 215 */

    {   int m = z->l - z->c; /* do, line 216 */
        if (!r_attached_pronoun(z)) goto lab1; /* call attached_pronoun, line 216 */
    lab1:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; /* do, line 217 */
        {   int m = z->l - z->c; /* or, line 217 */
            if (!r_standard_suffix(z)) goto lab4; /* call standard_suffix, line 217 */
            goto lab3;
        lab4:
            z->c = z->l - m;
            if (!r_y_verb_suffix(z)) goto lab5; /* call y_verb_suffix, line 218 */
            goto lab3;
        lab5:
            z->c = z->l - m;
            if (!r_verb_suffix(z)) goto lab2; /* call verb_suffix, line 219 */
        }
    lab3:
    lab2:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; /* do, line 221 */
        if (!r_residual_suffix(z)) goto lab6; /* call residual_suffix, line 221 */
    lab6:
        z->c = z->l - m;
    }
    z->c = z->lb;
    {   int c = z->c; /* do, line 223 */
        if (!r_postlude(z)) goto lab7; /* call postlude, line 223 */
    lab7:
        z->c = c;
    }
    return 1;
}

extern struct SN_env * snowball_spanish_create_env(void) { return SN_create_env(0, 3, 0); }

extern void snowball_spanish_close_env(struct SN_env * z) { SN_close_env(z); }

