#include <config.h>

/* This file was generated automatically by the Snowball to ANSI C compiler */

#include "header.h"

extern int snowball_lovins_stem(struct SN_env * z);
static int r_respell(struct SN_env * z);
static int r_undouble(struct SN_env * z);
static int r_endings(struct SN_env * z);
static int r_CC(struct SN_env * z);
static int r_BB(struct SN_env * z);
static int r_AA(struct SN_env * z);
static int r_Z(struct SN_env * z);
static int r_Y(struct SN_env * z);
static int r_X(struct SN_env * z);
static int r_W(struct SN_env * z);
static int r_V(struct SN_env * z);
static int r_U(struct SN_env * z);
static int r_T(struct SN_env * z);
static int r_S(struct SN_env * z);
static int r_R(struct SN_env * z);
static int r_Q(struct SN_env * z);
static int r_P(struct SN_env * z);
static int r_O(struct SN_env * z);
static int r_N(struct SN_env * z);
static int r_M(struct SN_env * z);
static int r_L(struct SN_env * z);
static int r_K(struct SN_env * z);
static int r_J(struct SN_env * z);
static int r_I(struct SN_env * z);
static int r_H(struct SN_env * z);
static int r_G(struct SN_env * z);
static int r_F(struct SN_env * z);
static int r_E(struct SN_env * z);
static int r_D(struct SN_env * z);
static int r_C(struct SN_env * z);
static int r_B(struct SN_env * z);
static int r_A(struct SN_env * z);

static symbol s_0_0[1] = { 'd' };
static symbol s_0_1[1] = { 'f' };
static symbol s_0_2[2] = { 'p', 'h' };
static symbol s_0_3[2] = { 't', 'h' };
static symbol s_0_4[1] = { 'l' };
static symbol s_0_5[2] = { 'e', 'r' };
static symbol s_0_6[2] = { 'o', 'r' };
static symbol s_0_7[2] = { 'e', 's' };
static symbol s_0_8[1] = { 't' };

static struct among a_0[9] =
{
/*  0 */ { 1, s_0_0, -1, -1, 0},
/*  1 */ { 1, s_0_1, -1, -1, 0},
/*  2 */ { 2, s_0_2, -1, -1, 0},
/*  3 */ { 2, s_0_3, -1, -1, 0},
/*  4 */ { 1, s_0_4, -1, -1, 0},
/*  5 */ { 2, s_0_5, -1, -1, 0},
/*  6 */ { 2, s_0_6, -1, -1, 0},
/*  7 */ { 2, s_0_7, -1, -1, 0},
/*  8 */ { 1, s_0_8, -1, -1, 0}
};

static symbol s_1_0[2] = { 's', '\'' };
static symbol s_1_1[1] = { 'a' };
static symbol s_1_2[2] = { 'i', 'a' };
static symbol s_1_3[3] = { 'a', 't', 'a' };
static symbol s_1_4[2] = { 'i', 'c' };
static symbol s_1_5[3] = { 'a', 'i', 'c' };
static symbol s_1_6[5] = { 'a', 'l', 'l', 'i', 'c' };
static symbol s_1_7[4] = { 'a', 'r', 'i', 'c' };
static symbol s_1_8[4] = { 'a', 't', 'i', 'c' };
static symbol s_1_9[4] = { 'i', 't', 'i', 'c' };
static symbol s_1_10[5] = { 'a', 'n', 't', 'i', 'c' };
static symbol s_1_11[5] = { 'i', 's', 't', 'i', 'c' };
static symbol s_1_12[7] = { 'a', 'l', 'i', 's', 't', 'i', 'c' };
static symbol s_1_13[7] = { 'a', 'r', 'i', 's', 't', 'i', 'c' };
static symbol s_1_14[7] = { 'i', 'v', 'i', 's', 't', 'i', 'c' };
static symbol s_1_15[2] = { 'e', 'd' };
static symbol s_1_16[5] = { 'a', 'n', 'c', 'e', 'd' };
static symbol s_1_17[5] = { 'e', 'n', 'c', 'e', 'd' };
static symbol s_1_18[5] = { 'i', 's', 'h', 'e', 'd' };
static symbol s_1_19[3] = { 'i', 'e', 'd' };
static symbol s_1_20[4] = { 'e', 'n', 'e', 'd' };
static symbol s_1_21[5] = { 'i', 'o', 'n', 'e', 'd' };
static symbol s_1_22[4] = { 'a', 't', 'e', 'd' };
static symbol s_1_23[5] = { 'e', 'n', 't', 'e', 'd' };
static symbol s_1_24[4] = { 'i', 'z', 'e', 'd' };
static symbol s_1_25[6] = { 'a', 'r', 'i', 'z', 'e', 'd' };
static symbol s_1_26[3] = { 'o', 'i', 'd' };
static symbol s_1_27[5] = { 'a', 'r', 'o', 'i', 'd' };
static symbol s_1_28[4] = { 'h', 'o', 'o', 'd' };
static symbol s_1_29[5] = { 'e', 'h', 'o', 'o', 'd' };
static symbol s_1_30[5] = { 'i', 'h', 'o', 'o', 'd' };
static symbol s_1_31[7] = { 'e', 'l', 'i', 'h', 'o', 'o', 'd' };
static symbol s_1_32[4] = { 'w', 'a', 'r', 'd' };
static symbol s_1_33[1] = { 'e' };
static symbol s_1_34[2] = { 'a', 'e' };
static symbol s_1_35[4] = { 'a', 'n', 'c', 'e' };
static symbol s_1_36[6] = { 'i', 'c', 'a', 'n', 'c', 'e' };
static symbol s_1_37[4] = { 'e', 'n', 'c', 'e' };
static symbol s_1_38[3] = { 'i', 'd', 'e' };
static symbol s_1_39[5] = { 'i', 'c', 'i', 'd', 'e' };
static symbol s_1_40[5] = { 'o', 't', 'i', 'd', 'e' };
static symbol s_1_41[3] = { 'a', 'g', 'e' };
static symbol s_1_42[4] = { 'a', 'b', 'l', 'e' };
static symbol s_1_43[6] = { 'a', 't', 'a', 'b', 'l', 'e' };
static symbol s_1_44[6] = { 'i', 'z', 'a', 'b', 'l', 'e' };
static symbol s_1_45[8] = { 'a', 'r', 'i', 'z', 'a', 'b', 'l', 'e' };
static symbol s_1_46[4] = { 'i', 'b', 'l', 'e' };
static symbol s_1_47[7] = { 'e', 'n', 'c', 'i', 'b', 'l', 'e' };
static symbol s_1_48[3] = { 'e', 'n', 'e' };
static symbol s_1_49[3] = { 'i', 'n', 'e' };
static symbol s_1_50[5] = { 'i', 'd', 'i', 'n', 'e' };
static symbol s_1_51[3] = { 'o', 'n', 'e' };
static symbol s_1_52[5] = { 'a', 't', 'u', 'r', 'e' };
static symbol s_1_53[6] = { 'e', 'a', 't', 'u', 'r', 'e' };
static symbol s_1_54[3] = { 'e', 's', 'e' };
static symbol s_1_55[4] = { 'w', 'i', 's', 'e' };
static symbol s_1_56[3] = { 'a', 't', 'e' };
static symbol s_1_57[7] = { 'e', 'n', 't', 'i', 'a', 't', 'e' };
static symbol s_1_58[5] = { 'i', 'n', 'a', 't', 'e' };
static symbol s_1_59[6] = { 'i', 'o', 'n', 'a', 't', 'e' };
static symbol s_1_60[3] = { 'i', 't', 'e' };
static symbol s_1_61[3] = { 'i', 'v', 'e' };
static symbol s_1_62[5] = { 'a', 't', 'i', 'v', 'e' };
static symbol s_1_63[3] = { 'i', 'z', 'e' };
static symbol s_1_64[5] = { 'a', 'l', 'i', 'z', 'e' };
static symbol s_1_65[7] = { 'i', 'c', 'a', 'l', 'i', 'z', 'e' };
static symbol s_1_66[6] = { 'i', 'a', 'l', 'i', 'z', 'e' };
static symbol s_1_67[9] = { 'e', 'n', 't', 'i', 'a', 'l', 'i', 'z', 'e' };
static symbol s_1_68[8] = { 'i', 'o', 'n', 'a', 'l', 'i', 'z', 'e' };
static symbol s_1_69[5] = { 'a', 'r', 'i', 'z', 'e' };
static symbol s_1_70[3] = { 'i', 'n', 'g' };
static symbol s_1_71[6] = { 'a', 'n', 'c', 'i', 'n', 'g' };
static symbol s_1_72[6] = { 'e', 'n', 'c', 'i', 'n', 'g' };
static symbol s_1_73[5] = { 'a', 'g', 'i', 'n', 'g' };
static symbol s_1_74[5] = { 'e', 'n', 'i', 'n', 'g' };
static symbol s_1_75[6] = { 'i', 'o', 'n', 'i', 'n', 'g' };
static symbol s_1_76[5] = { 'a', 't', 'i', 'n', 'g' };
static symbol s_1_77[6] = { 'e', 'n', 't', 'i', 'n', 'g' };
static symbol s_1_78[4] = { 'y', 'i', 'n', 'g' };
static symbol s_1_79[5] = { 'i', 'z', 'i', 'n', 'g' };
static symbol s_1_80[7] = { 'a', 'r', 'i', 'z', 'i', 'n', 'g' };
static symbol s_1_81[3] = { 'i', 's', 'h' };
static symbol s_1_82[4] = { 'y', 'i', 's', 'h' };
static symbol s_1_83[1] = { 'i' };
static symbol s_1_84[2] = { 'a', 'l' };
static symbol s_1_85[4] = { 'i', 'c', 'a', 'l' };
static symbol s_1_86[5] = { 'a', 'i', 'c', 'a', 'l' };
static symbol s_1_87[7] = { 'i', 's', 't', 'i', 'c', 'a', 'l' };
static symbol s_1_88[5] = { 'o', 'i', 'd', 'a', 'l' };
static symbol s_1_89[3] = { 'e', 'a', 'l' };
static symbol s_1_90[3] = { 'i', 'a', 'l' };
static symbol s_1_91[6] = { 'a', 'n', 'c', 'i', 'a', 'l' };
static symbol s_1_92[5] = { 'a', 'r', 'i', 'a', 'l' };
static symbol s_1_93[6] = { 'e', 'n', 't', 'i', 'a', 'l' };
static symbol s_1_94[5] = { 'i', 'o', 'n', 'a', 'l' };
static symbol s_1_95[7] = { 'a', 't', 'i', 'o', 'n', 'a', 'l' };
static symbol s_1_96[9] = { 'i', 'z', 'a', 't', 'i', 'o', 'n', 'a', 'l' };
static symbol s_1_97[5] = { 'e', 'n', 't', 'a', 'l' };
static symbol s_1_98[3] = { 'f', 'u', 'l' };
static symbol s_1_99[4] = { 'e', 'f', 'u', 'l' };
static symbol s_1_100[4] = { 'i', 'f', 'u', 'l' };
static symbol s_1_101[2] = { 'y', 'l' };
static symbol s_1_102[3] = { 'i', 's', 'm' };
static symbol s_1_103[5] = { 'i', 'c', 'i', 's', 'm' };
static symbol s_1_104[6] = { 'o', 'i', 'd', 'i', 's', 'm' };
static symbol s_1_105[5] = { 'a', 'l', 'i', 's', 'm' };
static symbol s_1_106[7] = { 'i', 'c', 'a', 'l', 'i', 's', 'm' };
static symbol s_1_107[8] = { 'i', 'o', 'n', 'a', 'l', 'i', 's', 'm' };
static symbol s_1_108[5] = { 'i', 'n', 'i', 's', 'm' };
static symbol s_1_109[7] = { 'a', 't', 'i', 'v', 'i', 's', 'm' };
static symbol s_1_110[2] = { 'u', 'm' };
static symbol s_1_111[3] = { 'i', 'u', 'm' };
static symbol s_1_112[3] = { 'i', 'a', 'n' };
static symbol s_1_113[5] = { 'i', 'c', 'i', 'a', 'n' };
static symbol s_1_114[2] = { 'e', 'n' };
static symbol s_1_115[4] = { 'o', 'g', 'e', 'n' };
static symbol s_1_116[2] = { 'o', 'n' };
static symbol s_1_117[3] = { 'i', 'o', 'n' };
static symbol s_1_118[5] = { 'a', 't', 'i', 'o', 'n' };
static symbol s_1_119[7] = { 'i', 'c', 'a', 't', 'i', 'o', 'n' };
static symbol s_1_120[9] = { 'e', 'n', 't', 'i', 'a', 't', 'i', 'o', 'n' };
static symbol s_1_121[7] = { 'i', 'n', 'a', 't', 'i', 'o', 'n' };
static symbol s_1_122[7] = { 'i', 's', 'a', 't', 'i', 'o', 'n' };
static symbol s_1_123[9] = { 'a', 'r', 'i', 's', 'a', 't', 'i', 'o', 'n' };
static symbol s_1_124[8] = { 'e', 'n', 't', 'a', 't', 'i', 'o', 'n' };
static symbol s_1_125[7] = { 'i', 'z', 'a', 't', 'i', 'o', 'n' };
static symbol s_1_126[9] = { 'a', 'r', 'i', 'z', 'a', 't', 'i', 'o', 'n' };
static symbol s_1_127[6] = { 'a', 'c', 't', 'i', 'o', 'n' };
static symbol s_1_128[1] = { 'o' };
static symbol s_1_129[2] = { 'a', 'r' };
static symbol s_1_130[3] = { 'e', 'a', 'r' };
static symbol s_1_131[3] = { 'i', 'e', 'r' };
static symbol s_1_132[6] = { 'a', 'r', 'i', 's', 'e', 'r' };
static symbol s_1_133[4] = { 'i', 'z', 'e', 'r' };
static symbol s_1_134[6] = { 'a', 'r', 'i', 'z', 'e', 'r' };
static symbol s_1_135[2] = { 'o', 'r' };
static symbol s_1_136[4] = { 'a', 't', 'o', 'r' };
static symbol s_1_137[1] = { 's' };
static symbol s_1_138[2] = { '\'', 's' };
static symbol s_1_139[2] = { 'a', 's' };
static symbol s_1_140[3] = { 'i', 'c', 's' };
static symbol s_1_141[6] = { 'i', 's', 't', 'i', 'c', 's' };
static symbol s_1_142[2] = { 'e', 's' };
static symbol s_1_143[5] = { 'a', 'n', 'c', 'e', 's' };
static symbol s_1_144[5] = { 'e', 'n', 'c', 'e', 's' };
static symbol s_1_145[4] = { 'i', 'd', 'e', 's' };
static symbol s_1_146[5] = { 'o', 'i', 'd', 'e', 's' };
static symbol s_1_147[4] = { 'a', 'g', 'e', 's' };
static symbol s_1_148[3] = { 'i', 'e', 's' };
static symbol s_1_149[5] = { 'a', 'c', 'i', 'e', 's' };
static symbol s_1_150[6] = { 'a', 'n', 'c', 'i', 'e', 's' };
static symbol s_1_151[6] = { 'e', 'n', 'c', 'i', 'e', 's' };
static symbol s_1_152[5] = { 'a', 'r', 'i', 'e', 's' };
static symbol s_1_153[5] = { 'i', 't', 'i', 'e', 's' };
static symbol s_1_154[7] = { 'a', 'l', 'i', 't', 'i', 'e', 's' };
static symbol s_1_155[7] = { 'i', 'v', 'i', 't', 'i', 'e', 's' };
static symbol s_1_156[4] = { 'i', 'n', 'e', 's' };
static symbol s_1_157[6] = { 'n', 'e', 's', 's', 'e', 's' };
static symbol s_1_158[4] = { 'a', 't', 'e', 's' };
static symbol s_1_159[6] = { 'a', 't', 'i', 'v', 'e', 's' };
static symbol s_1_160[4] = { 'i', 'n', 'g', 's' };
static symbol s_1_161[2] = { 'i', 's' };
static symbol s_1_162[3] = { 'a', 'l', 's' };
static symbol s_1_163[4] = { 'i', 'a', 'l', 's' };
static symbol s_1_164[7] = { 'e', 'n', 't', 'i', 'a', 'l', 's' };
static symbol s_1_165[6] = { 'i', 'o', 'n', 'a', 'l', 's' };
static symbol s_1_166[4] = { 'i', 's', 'm', 's' };
static symbol s_1_167[4] = { 'i', 'a', 'n', 's' };
static symbol s_1_168[6] = { 'i', 'c', 'i', 'a', 'n', 's' };
static symbol s_1_169[4] = { 'i', 'o', 'n', 's' };
static symbol s_1_170[6] = { 'a', 't', 'i', 'o', 'n', 's' };
static symbol s_1_171[10] = { 'a', 'r', 'i', 's', 'a', 't', 'i', 'o', 'n', 's' };
static symbol s_1_172[9] = { 'e', 'n', 't', 'a', 't', 'i', 'o', 'n', 's' };
static symbol s_1_173[8] = { 'i', 'z', 'a', 't', 'i', 'o', 'n', 's' };
static symbol s_1_174[10] = { 'a', 'r', 'i', 'z', 'a', 't', 'i', 'o', 'n', 's' };
static symbol s_1_175[3] = { 'a', 'r', 's' };
static symbol s_1_176[4] = { 'i', 'e', 'r', 's' };
static symbol s_1_177[5] = { 'i', 'z', 'e', 'r', 's' };
static symbol s_1_178[5] = { 'a', 't', 'o', 'r', 's' };
static symbol s_1_179[4] = { 'l', 'e', 's', 's' };
static symbol s_1_180[5] = { 'e', 'l', 'e', 's', 's' };
static symbol s_1_181[4] = { 'n', 'e', 's', 's' };
static symbol s_1_182[5] = { 'e', 'n', 'e', 's', 's' };
static symbol s_1_183[8] = { 'a', 'b', 'l', 'e', 'n', 'e', 's', 's' };
static symbol s_1_184[9] = { 'e', 'a', 'b', 'l', 'e', 'n', 'e', 's', 's' };
static symbol s_1_185[8] = { 'i', 'b', 'l', 'e', 'n', 'e', 's', 's' };
static symbol s_1_186[7] = { 'a', 't', 'e', 'n', 'e', 's', 's' };
static symbol s_1_187[7] = { 'i', 't', 'e', 'n', 'e', 's', 's' };
static symbol s_1_188[7] = { 'i', 'v', 'e', 'n', 'e', 's', 's' };
static symbol s_1_189[9] = { 'a', 't', 'i', 'v', 'e', 'n', 'e', 's', 's' };
static symbol s_1_190[7] = { 'i', 'n', 'g', 'n', 'e', 's', 's' };
static symbol s_1_191[7] = { 'i', 's', 'h', 'n', 'e', 's', 's' };
static symbol s_1_192[5] = { 'i', 'n', 'e', 's', 's' };
static symbol s_1_193[7] = { 'a', 'r', 'i', 'n', 'e', 's', 's' };
static symbol s_1_194[6] = { 'a', 'l', 'n', 'e', 's', 's' };
static symbol s_1_195[8] = { 'i', 'c', 'a', 'l', 'n', 'e', 's', 's' };
static symbol s_1_196[10] = { 'a', 'n', 't', 'i', 'a', 'l', 'n', 'e', 's', 's' };
static symbol s_1_197[10] = { 'e', 'n', 't', 'i', 'a', 'l', 'n', 'e', 's', 's' };
static symbol s_1_198[9] = { 'i', 'o', 'n', 'a', 'l', 'n', 'e', 's', 's' };
static symbol s_1_199[7] = { 'f', 'u', 'l', 'n', 'e', 's', 's' };
static symbol s_1_200[8] = { 'l', 'e', 's', 's', 'n', 'e', 's', 's' };
static symbol s_1_201[7] = { 'o', 'u', 's', 'n', 'e', 's', 's' };
static symbol s_1_202[8] = { 'e', 'o', 'u', 's', 'n', 'e', 's', 's' };
static symbol s_1_203[8] = { 'i', 'o', 'u', 's', 'n', 'e', 's', 's' };
static symbol s_1_204[9] = { 'i', 't', 'o', 'u', 's', 'n', 'e', 's', 's' };
static symbol s_1_205[7] = { 'e', 'n', 't', 'n', 'e', 's', 's' };
static symbol s_1_206[4] = { 'a', 'n', 't', 's' };
static symbol s_1_207[4] = { 'i', 's', 't', 's' };
static symbol s_1_208[6] = { 'i', 'c', 'i', 's', 't', 's' };
static symbol s_1_209[2] = { 'u', 's' };
static symbol s_1_210[3] = { 'o', 'u', 's' };
static symbol s_1_211[4] = { 'e', 'o', 'u', 's' };
static symbol s_1_212[6] = { 'a', 'c', 'e', 'o', 'u', 's' };
static symbol s_1_213[9] = { 'a', 'n', 't', 'a', 'n', 'e', 'o', 'u', 's' };
static symbol s_1_214[4] = { 'i', 'o', 'u', 's' };
static symbol s_1_215[6] = { 'a', 'c', 'i', 'o', 'u', 's' };
static symbol s_1_216[5] = { 'i', 't', 'o', 'u', 's' };
static symbol s_1_217[3] = { 'a', 'n', 't' };
static symbol s_1_218[5] = { 'i', 'c', 'a', 'n', 't' };
static symbol s_1_219[3] = { 'e', 'n', 't' };
static symbol s_1_220[5] = { 'e', 'm', 'e', 'n', 't' };
static symbol s_1_221[7] = { 'i', 'z', 'e', 'm', 'e', 'n', 't' };
static symbol s_1_222[3] = { 'i', 's', 't' };
static symbol s_1_223[5] = { 'i', 'c', 'i', 's', 't' };
static symbol s_1_224[5] = { 'a', 'l', 'i', 's', 't' };
static symbol s_1_225[7] = { 'i', 'c', 'a', 'l', 'i', 's', 't' };
static symbol s_1_226[6] = { 'i', 'a', 'l', 'i', 's', 't' };
static symbol s_1_227[6] = { 'i', 'o', 'n', 'i', 's', 't' };
static symbol s_1_228[6] = { 'e', 'n', 't', 'i', 's', 't' };
static symbol s_1_229[1] = { 'y' };
static symbol s_1_230[3] = { 'a', 'c', 'y' };
static symbol s_1_231[4] = { 'a', 'n', 'c', 'y' };
static symbol s_1_232[4] = { 'e', 'n', 'c', 'y' };
static symbol s_1_233[2] = { 'l', 'y' };
static symbol s_1_234[4] = { 'e', 'a', 'l', 'y' };
static symbol s_1_235[4] = { 'a', 'b', 'l', 'y' };
static symbol s_1_236[4] = { 'i', 'b', 'l', 'y' };
static symbol s_1_237[4] = { 'e', 'd', 'l', 'y' };
static symbol s_1_238[5] = { 'i', 'e', 'd', 'l', 'y' };
static symbol s_1_239[3] = { 'e', 'l', 'y' };
static symbol s_1_240[5] = { 'a', 't', 'e', 'l', 'y' };
static symbol s_1_241[5] = { 'i', 'v', 'e', 'l', 'y' };
static symbol s_1_242[7] = { 'a', 't', 'i', 'v', 'e', 'l', 'y' };
static symbol s_1_243[5] = { 'i', 'n', 'g', 'l', 'y' };
static symbol s_1_244[7] = { 'a', 't', 'i', 'n', 'g', 'l', 'y' };
static symbol s_1_245[3] = { 'i', 'l', 'y' };
static symbol s_1_246[4] = { 'l', 'i', 'l', 'y' };
static symbol s_1_247[5] = { 'a', 'r', 'i', 'l', 'y' };
static symbol s_1_248[4] = { 'a', 'l', 'l', 'y' };
static symbol s_1_249[6] = { 'i', 'c', 'a', 'l', 'l', 'y' };
static symbol s_1_250[7] = { 'a', 'i', 'c', 'a', 'l', 'l', 'y' };
static symbol s_1_251[9] = { 'a', 'l', 'l', 'i', 'c', 'a', 'l', 'l', 'y' };
static symbol s_1_252[9] = { 'i', 's', 't', 'i', 'c', 'a', 'l', 'l', 'y' };
static symbol s_1_253[11] = { 'a', 'l', 'i', 's', 't', 'i', 'c', 'a', 'l', 'l', 'y' };
static symbol s_1_254[7] = { 'o', 'i', 'd', 'a', 'l', 'l', 'y' };
static symbol s_1_255[5] = { 'i', 'a', 'l', 'l', 'y' };
static symbol s_1_256[8] = { 'e', 'n', 't', 'i', 'a', 'l', 'l', 'y' };
static symbol s_1_257[7] = { 'i', 'o', 'n', 'a', 'l', 'l', 'y' };
static symbol s_1_258[9] = { 'a', 't', 'i', 'o', 'n', 'a', 'l', 'l', 'y' };
static symbol s_1_259[11] = { 'i', 'z', 'a', 't', 'i', 'o', 'n', 'a', 'l', 'l', 'y' };
static symbol s_1_260[7] = { 'e', 'n', 't', 'a', 'l', 'l', 'y' };
static symbol s_1_261[5] = { 'f', 'u', 'l', 'l', 'y' };
static symbol s_1_262[6] = { 'e', 'f', 'u', 'l', 'l', 'y' };
static symbol s_1_263[6] = { 'i', 'f', 'u', 'l', 'l', 'y' };
static symbol s_1_264[4] = { 'e', 'n', 'l', 'y' };
static symbol s_1_265[4] = { 'a', 'r', 'l', 'y' };
static symbol s_1_266[5] = { 'e', 'a', 'r', 'l', 'y' };
static symbol s_1_267[6] = { 'l', 'e', 's', 's', 'l', 'y' };
static symbol s_1_268[5] = { 'o', 'u', 's', 'l', 'y' };
static symbol s_1_269[6] = { 'e', 'o', 'u', 's', 'l', 'y' };
static symbol s_1_270[6] = { 'i', 'o', 'u', 's', 'l', 'y' };
static symbol s_1_271[5] = { 'e', 'n', 't', 'l', 'y' };
static symbol s_1_272[3] = { 'a', 'r', 'y' };
static symbol s_1_273[3] = { 'e', 'r', 'y' };
static symbol s_1_274[7] = { 'i', 'c', 'i', 'a', 'n', 'r', 'y' };
static symbol s_1_275[5] = { 'a', 't', 'o', 'r', 'y' };
static symbol s_1_276[3] = { 'i', 't', 'y' };
static symbol s_1_277[5] = { 'a', 'c', 'i', 't', 'y' };
static symbol s_1_278[5] = { 'i', 'c', 'i', 't', 'y' };
static symbol s_1_279[4] = { 'e', 'i', 't', 'y' };
static symbol s_1_280[5] = { 'a', 'l', 'i', 't', 'y' };
static symbol s_1_281[7] = { 'i', 'c', 'a', 'l', 'i', 't', 'y' };
static symbol s_1_282[6] = { 'i', 'a', 'l', 'i', 't', 'y' };
static symbol s_1_283[9] = { 'a', 'n', 't', 'i', 'a', 'l', 'i', 't', 'y' };
static symbol s_1_284[9] = { 'e', 'n', 't', 'i', 'a', 'l', 'i', 't', 'y' };
static symbol s_1_285[8] = { 'i', 'o', 'n', 'a', 'l', 'i', 't', 'y' };
static symbol s_1_286[5] = { 'e', 'l', 'i', 't', 'y' };
static symbol s_1_287[7] = { 'a', 'b', 'i', 'l', 'i', 't', 'y' };
static symbol s_1_288[9] = { 'i', 'z', 'a', 'b', 'i', 'l', 'i', 't', 'y' };
static symbol s_1_289[11] = { 'a', 'r', 'i', 'z', 'a', 'b', 'i', 'l', 'i', 't', 'y' };
static symbol s_1_290[7] = { 'i', 'b', 'i', 'l', 'i', 't', 'y' };
static symbol s_1_291[5] = { 'i', 'n', 'i', 't', 'y' };
static symbol s_1_292[5] = { 'a', 'r', 'i', 't', 'y' };
static symbol s_1_293[5] = { 'i', 'v', 'i', 't', 'y' };

static struct among a_1[294] =
{
/*  0 */ { 2, s_1_0, -1, 1, r_A},
/*  1 */ { 1, s_1_1, -1, 1, r_A},
/*  2 */ { 2, s_1_2, 1, 1, r_A},
/*  3 */ { 3, s_1_3, 1, 1, r_A},
/*  4 */ { 2, s_1_4, -1, 1, r_A},
/*  5 */ { 3, s_1_5, 4, 1, r_A},
/*  6 */ { 5, s_1_6, 4, 1, r_BB},
/*  7 */ { 4, s_1_7, 4, 1, r_A},
/*  8 */ { 4, s_1_8, 4, 1, r_B},
/*  9 */ { 4, s_1_9, 4, 1, r_H},
/* 10 */ { 5, s_1_10, 4, 1, r_C},
/* 11 */ { 5, s_1_11, 4, 1, r_A},
/* 12 */ { 7, s_1_12, 11, 1, r_B},
/* 13 */ { 7, s_1_13, 11, 1, r_A},
/* 14 */ { 7, s_1_14, 11, 1, r_A},
/* 15 */ { 2, s_1_15, -1, 1, r_E},
/* 16 */ { 5, s_1_16, 15, 1, r_B},
/* 17 */ { 5, s_1_17, 15, 1, r_A},
/* 18 */ { 5, s_1_18, 15, 1, r_A},
/* 19 */ { 3, s_1_19, 15, 1, r_A},
/* 20 */ { 4, s_1_20, 15, 1, r_E},
/* 21 */ { 5, s_1_21, 15, 1, r_A},
/* 22 */ { 4, s_1_22, 15, 1, r_I},
/* 23 */ { 5, s_1_23, 15, 1, r_C},
/* 24 */ { 4, s_1_24, 15, 1, r_F},
/* 25 */ { 6, s_1_25, 24, 1, r_A},
/* 26 */ { 3, s_1_26, -1, 1, r_A},
/* 27 */ { 5, s_1_27, 26, 1, r_A},
/* 28 */ { 4, s_1_28, -1, 1, r_A},
/* 29 */ { 5, s_1_29, 28, 1, r_A},
/* 30 */ { 5, s_1_30, 28, 1, r_A},
/* 31 */ { 7, s_1_31, 30, 1, r_E},
/* 32 */ { 4, s_1_32, -1, 1, r_A},
/* 33 */ { 1, s_1_33, -1, 1, r_A},
/* 34 */ { 2, s_1_34, 33, 1, r_A},
/* 35 */ { 4, s_1_35, 33, 1, r_B},
/* 36 */ { 6, s_1_36, 35, 1, r_A},
/* 37 */ { 4, s_1_37, 33, 1, r_A},
/* 38 */ { 3, s_1_38, 33, 1, r_L},
/* 39 */ { 5, s_1_39, 38, 1, r_A},
/* 40 */ { 5, s_1_40, 38, 1, r_A},
/* 41 */ { 3, s_1_41, 33, 1, r_B},
/* 42 */ { 4, s_1_42, 33, 1, r_A},
/* 43 */ { 6, s_1_43, 42, 1, r_A},
/* 44 */ { 6, s_1_44, 42, 1, r_E},
/* 45 */ { 8, s_1_45, 44, 1, r_A},
/* 46 */ { 4, s_1_46, 33, 1, r_A},
/* 47 */ { 7, s_1_47, 46, 1, r_A},
/* 48 */ { 3, s_1_48, 33, 1, r_E},
/* 49 */ { 3, s_1_49, 33, 1, r_M},
/* 50 */ { 5, s_1_50, 49, 1, r_I},
/* 51 */ { 3, s_1_51, 33, 1, r_R},
/* 52 */ { 5, s_1_52, 33, 1, r_E},
/* 53 */ { 6, s_1_53, 52, 1, r_Z},
/* 54 */ { 3, s_1_54, 33, 1, r_A},
/* 55 */ { 4, s_1_55, 33, 1, r_A},
/* 56 */ { 3, s_1_56, 33, 1, r_A},
/* 57 */ { 7, s_1_57, 56, 1, r_A},
/* 58 */ { 5, s_1_58, 56, 1, r_A},
/* 59 */ { 6, s_1_59, 56, 1, r_D},
/* 60 */ { 3, s_1_60, 33, 1, r_AA},
/* 61 */ { 3, s_1_61, 33, 1, r_A},
/* 62 */ { 5, s_1_62, 61, 1, r_A},
/* 63 */ { 3, s_1_63, 33, 1, r_F},
/* 64 */ { 5, s_1_64, 63, 1, r_A},
/* 65 */ { 7, s_1_65, 64, 1, r_A},
/* 66 */ { 6, s_1_66, 64, 1, r_A},
/* 67 */ { 9, s_1_67, 66, 1, r_A},
/* 68 */ { 8, s_1_68, 64, 1, r_A},
/* 69 */ { 5, s_1_69, 63, 1, r_A},
/* 70 */ { 3, s_1_70, -1, 1, r_N},
/* 71 */ { 6, s_1_71, 70, 1, r_B},
/* 72 */ { 6, s_1_72, 70, 1, r_A},
/* 73 */ { 5, s_1_73, 70, 1, r_B},
/* 74 */ { 5, s_1_74, 70, 1, r_E},
/* 75 */ { 6, s_1_75, 70, 1, r_A},
/* 76 */ { 5, s_1_76, 70, 1, r_I},
/* 77 */ { 6, s_1_77, 70, 1, r_C},
/* 78 */ { 4, s_1_78, 70, 1, r_B},
/* 79 */ { 5, s_1_79, 70, 1, r_F},
/* 80 */ { 7, s_1_80, 79, 1, r_A},
/* 81 */ { 3, s_1_81, -1, 1, r_C},
/* 82 */ { 4, s_1_82, 81, 1, r_A},
/* 83 */ { 1, s_1_83, -1, 1, r_A},
/* 84 */ { 2, s_1_84, -1, 1, r_BB},
/* 85 */ { 4, s_1_85, 84, 1, r_A},
/* 86 */ { 5, s_1_86, 85, 1, r_A},
/* 87 */ { 7, s_1_87, 85, 1, r_A},
/* 88 */ { 5, s_1_88, 84, 1, r_A},
/* 89 */ { 3, s_1_89, 84, 1, r_Y},
/* 90 */ { 3, s_1_90, 84, 1, r_A},
/* 91 */ { 6, s_1_91, 90, 1, r_A},
/* 92 */ { 5, s_1_92, 90, 1, r_A},
/* 93 */ { 6, s_1_93, 90, 1, r_A},
/* 94 */ { 5, s_1_94, 84, 1, r_A},
/* 95 */ { 7, s_1_95, 94, 1, r_B},
/* 96 */ { 9, s_1_96, 95, 1, r_A},
/* 97 */ { 5, s_1_97, 84, 1, r_A},
/* 98 */ { 3, s_1_98, -1, 1, r_A},
/* 99 */ { 4, s_1_99, 98, 1, r_A},
/*100 */ { 4, s_1_100, 98, 1, r_A},
/*101 */ { 2, s_1_101, -1, 1, r_R},
/*102 */ { 3, s_1_102, -1, 1, r_B},
/*103 */ { 5, s_1_103, 102, 1, r_A},
/*104 */ { 6, s_1_104, 102, 1, r_A},
/*105 */ { 5, s_1_105, 102, 1, r_B},
/*106 */ { 7, s_1_106, 105, 1, r_A},
/*107 */ { 8, s_1_107, 105, 1, r_A},
/*108 */ { 5, s_1_108, 102, 1, r_J},
/*109 */ { 7, s_1_109, 102, 1, r_A},
/*110 */ { 2, s_1_110, -1, 1, r_U},
/*111 */ { 3, s_1_111, 110, 1, r_A},
/*112 */ { 3, s_1_112, -1, 1, r_A},
/*113 */ { 5, s_1_113, 112, 1, r_A},
/*114 */ { 2, s_1_114, -1, 1, r_F},
/*115 */ { 4, s_1_115, 114, 1, r_A},
/*116 */ { 2, s_1_116, -1, 1, r_S},
/*117 */ { 3, s_1_117, 116, 1, r_Q},
/*118 */ { 5, s_1_118, 117, 1, r_B},
/*119 */ { 7, s_1_119, 118, 1, r_G},
/*120 */ { 9, s_1_120, 118, 1, r_A},
/*121 */ { 7, s_1_121, 118, 1, r_A},
/*122 */ { 7, s_1_122, 118, 1, r_A},
/*123 */ { 9, s_1_123, 122, 1, r_A},
/*124 */ { 8, s_1_124, 118, 1, r_A},
/*125 */ { 7, s_1_125, 118, 1, r_F},
/*126 */ { 9, s_1_126, 125, 1, r_A},
/*127 */ { 6, s_1_127, 117, 1, r_G},
/*128 */ { 1, s_1_128, -1, 1, r_A},
/*129 */ { 2, s_1_129, -1, 1, r_X},
/*130 */ { 3, s_1_130, 129, 1, r_Y},
/*131 */ { 3, s_1_131, -1, 1, r_A},
/*132 */ { 6, s_1_132, -1, 1, r_A},
/*133 */ { 4, s_1_133, -1, 1, r_F},
/*134 */ { 6, s_1_134, 133, 1, r_A},
/*135 */ { 2, s_1_135, -1, 1, r_T},
/*136 */ { 4, s_1_136, 135, 1, r_A},
/*137 */ { 1, s_1_137, -1, 1, r_W},
/*138 */ { 2, s_1_138, 137, 1, r_A},
/*139 */ { 2, s_1_139, 137, 1, r_B},
/*140 */ { 3, s_1_140, 137, 1, r_A},
/*141 */ { 6, s_1_141, 140, 1, r_A},
/*142 */ { 2, s_1_142, 137, 1, r_E},
/*143 */ { 5, s_1_143, 142, 1, r_B},
/*144 */ { 5, s_1_144, 142, 1, r_A},
/*145 */ { 4, s_1_145, 142, 1, r_L},
/*146 */ { 5, s_1_146, 145, 1, r_A},
/*147 */ { 4, s_1_147, 142, 1, r_B},
/*148 */ { 3, s_1_148, 142, 1, r_P},
/*149 */ { 5, s_1_149, 148, 1, r_A},
/*150 */ { 6, s_1_150, 148, 1, r_A},
/*151 */ { 6, s_1_151, 148, 1, r_A},
/*152 */ { 5, s_1_152, 148, 1, r_A},
/*153 */ { 5, s_1_153, 148, 1, r_A},
/*154 */ { 7, s_1_154, 153, 1, r_A},
/*155 */ { 7, s_1_155, 153, 1, r_A},
/*156 */ { 4, s_1_156, 142, 1, r_M},
/*157 */ { 6, s_1_157, 142, 1, r_A},
/*158 */ { 4, s_1_158, 142, 1, r_A},
/*159 */ { 6, s_1_159, 142, 1, r_A},
/*160 */ { 4, s_1_160, 137, 1, r_N},
/*161 */ { 2, s_1_161, 137, 1, r_A},
/*162 */ { 3, s_1_162, 137, 1, r_BB},
/*163 */ { 4, s_1_163, 162, 1, r_A},
/*164 */ { 7, s_1_164, 163, 1, r_A},
/*165 */ { 6, s_1_165, 162, 1, r_A},
/*166 */ { 4, s_1_166, 137, 1, r_B},
/*167 */ { 4, s_1_167, 137, 1, r_A},
/*168 */ { 6, s_1_168, 167, 1, r_A},
/*169 */ { 4, s_1_169, 137, 1, r_B},
/*170 */ { 6, s_1_170, 169, 1, r_B},
/*171 */ { 10, s_1_171, 170, 1, r_A},
/*172 */ { 9, s_1_172, 170, 1, r_A},
/*173 */ { 8, s_1_173, 170, 1, r_A},
/*174 */ { 10, s_1_174, 173, 1, r_A},
/*175 */ { 3, s_1_175, 137, 1, r_O},
/*176 */ { 4, s_1_176, 137, 1, r_A},
/*177 */ { 5, s_1_177, 137, 1, r_F},
/*178 */ { 5, s_1_178, 137, 1, r_A},
/*179 */ { 4, s_1_179, 137, 1, r_A},
/*180 */ { 5, s_1_180, 179, 1, r_A},
/*181 */ { 4, s_1_181, 137, 1, r_A},
/*182 */ { 5, s_1_182, 181, 1, r_E},
/*183 */ { 8, s_1_183, 182, 1, r_A},
/*184 */ { 9, s_1_184, 183, 1, r_E},
/*185 */ { 8, s_1_185, 182, 1, r_A},
/*186 */ { 7, s_1_186, 182, 1, r_A},
/*187 */ { 7, s_1_187, 182, 1, r_A},
/*188 */ { 7, s_1_188, 182, 1, r_A},
/*189 */ { 9, s_1_189, 188, 1, r_A},
/*190 */ { 7, s_1_190, 181, 1, r_A},
/*191 */ { 7, s_1_191, 181, 1, r_A},
/*192 */ { 5, s_1_192, 181, 1, r_A},
/*193 */ { 7, s_1_193, 192, 1, r_E},
/*194 */ { 6, s_1_194, 181, 1, r_A},
/*195 */ { 8, s_1_195, 194, 1, r_A},
/*196 */ { 10, s_1_196, 194, 1, r_A},
/*197 */ { 10, s_1_197, 194, 1, r_A},
/*198 */ { 9, s_1_198, 194, 1, r_A},
/*199 */ { 7, s_1_199, 181, 1, r_A},
/*200 */ { 8, s_1_200, 181, 1, r_A},
/*201 */ { 7, s_1_201, 181, 1, r_A},
/*202 */ { 8, s_1_202, 201, 1, r_A},
/*203 */ { 8, s_1_203, 201, 1, r_A},
/*204 */ { 9, s_1_204, 201, 1, r_A},
/*205 */ { 7, s_1_205, 181, 1, r_A},
/*206 */ { 4, s_1_206, 137, 1, r_B},
/*207 */ { 4, s_1_207, 137, 1, r_A},
/*208 */ { 6, s_1_208, 207, 1, r_A},
/*209 */ { 2, s_1_209, 137, 1, r_V},
/*210 */ { 3, s_1_210, 209, 1, r_A},
/*211 */ { 4, s_1_211, 210, 1, r_A},
/*212 */ { 6, s_1_212, 211, 1, r_A},
/*213 */ { 9, s_1_213, 211, 1, r_A},
/*214 */ { 4, s_1_214, 210, 1, r_A},
/*215 */ { 6, s_1_215, 214, 1, r_B},
/*216 */ { 5, s_1_216, 210, 1, r_A},
/*217 */ { 3, s_1_217, -1, 1, r_B},
/*218 */ { 5, s_1_218, 217, 1, r_A},
/*219 */ { 3, s_1_219, -1, 1, r_C},
/*220 */ { 5, s_1_220, 219, 1, r_A},
/*221 */ { 7, s_1_221, 220, 1, r_A},
/*222 */ { 3, s_1_222, -1, 1, r_A},
/*223 */ { 5, s_1_223, 222, 1, r_A},
/*224 */ { 5, s_1_224, 222, 1, r_A},
/*225 */ { 7, s_1_225, 224, 1, r_A},
/*226 */ { 6, s_1_226, 224, 1, r_A},
/*227 */ { 6, s_1_227, 222, 1, r_A},
/*228 */ { 6, s_1_228, 222, 1, r_A},
/*229 */ { 1, s_1_229, -1, 1, r_B},
/*230 */ { 3, s_1_230, 229, 1, r_A},
/*231 */ { 4, s_1_231, 229, 1, r_B},
/*232 */ { 4, s_1_232, 229, 1, r_A},
/*233 */ { 2, s_1_233, 229, 1, r_B},
/*234 */ { 4, s_1_234, 233, 1, r_Y},
/*235 */ { 4, s_1_235, 233, 1, r_A},
/*236 */ { 4, s_1_236, 233, 1, r_A},
/*237 */ { 4, s_1_237, 233, 1, r_E},
/*238 */ { 5, s_1_238, 237, 1, r_A},
/*239 */ { 3, s_1_239, 233, 1, r_E},
/*240 */ { 5, s_1_240, 239, 1, r_A},
/*241 */ { 5, s_1_241, 239, 1, r_A},
/*242 */ { 7, s_1_242, 241, 1, r_A},
/*243 */ { 5, s_1_243, 233, 1, r_B},
/*244 */ { 7, s_1_244, 243, 1, r_A},
/*245 */ { 3, s_1_245, 233, 1, r_A},
/*246 */ { 4, s_1_246, 245, 1, r_A},
/*247 */ { 5, s_1_247, 245, 1, r_A},
/*248 */ { 4, s_1_248, 233, 1, r_B},
/*249 */ { 6, s_1_249, 248, 1, r_A},
/*250 */ { 7, s_1_250, 249, 1, r_A},
/*251 */ { 9, s_1_251, 249, 1, r_C},
/*252 */ { 9, s_1_252, 249, 1, r_A},
/*253 */ { 11, s_1_253, 252, 1, r_B},
/*254 */ { 7, s_1_254, 248, 1, r_A},
/*255 */ { 5, s_1_255, 248, 1, r_A},
/*256 */ { 8, s_1_256, 255, 1, r_A},
/*257 */ { 7, s_1_257, 248, 1, r_A},
/*258 */ { 9, s_1_258, 257, 1, r_B},
/*259 */ { 11, s_1_259, 258, 1, r_B},
/*260 */ { 7, s_1_260, 248, 1, r_A},
/*261 */ { 5, s_1_261, 233, 1, r_A},
/*262 */ { 6, s_1_262, 261, 1, r_A},
/*263 */ { 6, s_1_263, 261, 1, r_A},
/*264 */ { 4, s_1_264, 233, 1, r_E},
/*265 */ { 4, s_1_265, 233, 1, r_K},
/*266 */ { 5, s_1_266, 265, 1, r_Y},
/*267 */ { 6, s_1_267, 233, 1, r_A},
/*268 */ { 5, s_1_268, 233, 1, r_A},
/*269 */ { 6, s_1_269, 268, 1, r_A},
/*270 */ { 6, s_1_270, 268, 1, r_A},
/*271 */ { 5, s_1_271, 233, 1, r_A},
/*272 */ { 3, s_1_272, 229, 1, r_F},
/*273 */ { 3, s_1_273, 229, 1, r_E},
/*274 */ { 7, s_1_274, 229, 1, r_A},
/*275 */ { 5, s_1_275, 229, 1, r_A},
/*276 */ { 3, s_1_276, 229, 1, r_A},
/*277 */ { 5, s_1_277, 276, 1, r_A},
/*278 */ { 5, s_1_278, 276, 1, r_A},
/*279 */ { 4, s_1_279, 276, 1, r_A},
/*280 */ { 5, s_1_280, 276, 1, r_A},
/*281 */ { 7, s_1_281, 280, 1, r_A},
/*282 */ { 6, s_1_282, 280, 1, r_A},
/*283 */ { 9, s_1_283, 282, 1, r_A},
/*284 */ { 9, s_1_284, 282, 1, r_A},
/*285 */ { 8, s_1_285, 280, 1, r_A},
/*286 */ { 5, s_1_286, 276, 1, r_A},
/*287 */ { 7, s_1_287, 276, 1, r_A},
/*288 */ { 9, s_1_288, 287, 1, r_A},
/*289 */ { 11, s_1_289, 288, 1, r_A},
/*290 */ { 7, s_1_290, 276, 1, r_A},
/*291 */ { 5, s_1_291, 276, 1, r_CC},
/*292 */ { 5, s_1_292, 276, 1, r_B},
/*293 */ { 5, s_1_293, 276, 1, r_A}
};

static symbol s_2_0[2] = { 'b', 'b' };
static symbol s_2_1[2] = { 'd', 'd' };
static symbol s_2_2[2] = { 'g', 'g' };
static symbol s_2_3[2] = { 'l', 'l' };
static symbol s_2_4[2] = { 'm', 'm' };
static symbol s_2_5[2] = { 'n', 'n' };
static symbol s_2_6[2] = { 'p', 'p' };
static symbol s_2_7[2] = { 'r', 'r' };
static symbol s_2_8[2] = { 's', 's' };
static symbol s_2_9[2] = { 't', 't' };

static struct among a_2[10] =
{
/*  0 */ { 2, s_2_0, -1, -1, 0},
/*  1 */ { 2, s_2_1, -1, -1, 0},
/*  2 */ { 2, s_2_2, -1, -1, 0},
/*  3 */ { 2, s_2_3, -1, -1, 0},
/*  4 */ { 2, s_2_4, -1, -1, 0},
/*  5 */ { 2, s_2_5, -1, -1, 0},
/*  6 */ { 2, s_2_6, -1, -1, 0},
/*  7 */ { 2, s_2_7, -1, -1, 0},
/*  8 */ { 2, s_2_8, -1, -1, 0},
/*  9 */ { 2, s_2_9, -1, -1, 0}
};

static symbol s_3_0[3] = { 'u', 'a', 'd' };
static symbol s_3_1[3] = { 'v', 'a', 'd' };
static symbol s_3_2[3] = { 'c', 'i', 'd' };
static symbol s_3_3[3] = { 'l', 'i', 'd' };
static symbol s_3_4[4] = { 'e', 'r', 'i', 'd' };
static symbol s_3_5[4] = { 'p', 'a', 'n', 'd' };
static symbol s_3_6[3] = { 'e', 'n', 'd' };
static symbol s_3_7[3] = { 'o', 'n', 'd' };
static symbol s_3_8[3] = { 'l', 'u', 'd' };
static symbol s_3_9[3] = { 'r', 'u', 'd' };
static symbol s_3_10[2] = { 'u', 'l' };
static symbol s_3_11[3] = { 'h', 'e', 'r' };
static symbol s_3_12[4] = { 'm', 'e', 't', 'r' };
static symbol s_3_13[4] = { 'i', 's', 't', 'r' };
static symbol s_3_14[3] = { 'u', 'r', 's' };
static symbol s_3_15[3] = { 'u', 'c', 't' };
static symbol s_3_16[2] = { 'e', 't' };
static symbol s_3_17[3] = { 'm', 'i', 't' };
static symbol s_3_18[3] = { 'e', 'n', 't' };
static symbol s_3_19[4] = { 'u', 'm', 'p', 't' };
static symbol s_3_20[3] = { 'r', 'p', 't' };
static symbol s_3_21[3] = { 'e', 'r', 't' };
static symbol s_3_22[2] = { 'y', 't' };
static symbol s_3_23[3] = { 'i', 'e', 'v' };
static symbol s_3_24[3] = { 'o', 'l', 'v' };
static symbol s_3_25[2] = { 'a', 'x' };
static symbol s_3_26[2] = { 'e', 'x' };
static symbol s_3_27[3] = { 'b', 'e', 'x' };
static symbol s_3_28[3] = { 'd', 'e', 'x' };
static symbol s_3_29[3] = { 'p', 'e', 'x' };
static symbol s_3_30[3] = { 't', 'e', 'x' };
static symbol s_3_31[2] = { 'i', 'x' };
static symbol s_3_32[3] = { 'l', 'u', 'x' };
static symbol s_3_33[2] = { 'y', 'z' };

static struct among a_3[34] =
{
/*  0 */ { 3, s_3_0, -1, 18, 0},
/*  1 */ { 3, s_3_1, -1, 19, 0},
/*  2 */ { 3, s_3_2, -1, 20, 0},
/*  3 */ { 3, s_3_3, -1, 21, 0},
/*  4 */ { 4, s_3_4, -1, 22, 0},
/*  5 */ { 4, s_3_5, -1, 23, 0},
/*  6 */ { 3, s_3_6, -1, 24, 0},
/*  7 */ { 3, s_3_7, -1, 25, 0},
/*  8 */ { 3, s_3_8, -1, 26, 0},
/*  9 */ { 3, s_3_9, -1, 27, 0},
/* 10 */ { 2, s_3_10, -1, 9, 0},
/* 11 */ { 3, s_3_11, -1, 28, 0},
/* 12 */ { 4, s_3_12, -1, 7, 0},
/* 13 */ { 4, s_3_13, -1, 6, 0},
/* 14 */ { 3, s_3_14, -1, 5, 0},
/* 15 */ { 3, s_3_15, -1, 2, 0},
/* 16 */ { 2, s_3_16, -1, 32, 0},
/* 17 */ { 3, s_3_17, -1, 29, 0},
/* 18 */ { 3, s_3_18, -1, 30, 0},
/* 19 */ { 4, s_3_19, -1, 3, 0},
/* 20 */ { 3, s_3_20, -1, 4, 0},
/* 21 */ { 3, s_3_21, -1, 31, 0},
/* 22 */ { 2, s_3_22, -1, 33, 0},
/* 23 */ { 3, s_3_23, -1, 1, 0},
/* 24 */ { 3, s_3_24, -1, 8, 0},
/* 25 */ { 2, s_3_25, -1, 14, 0},
/* 26 */ { 2, s_3_26, -1, 15, 0},
/* 27 */ { 3, s_3_27, 26, 10, 0},
/* 28 */ { 3, s_3_28, 26, 11, 0},
/* 29 */ { 3, s_3_29, 26, 12, 0},
/* 30 */ { 3, s_3_30, 26, 13, 0},
/* 31 */ { 2, s_3_31, -1, 16, 0},
/* 32 */ { 3, s_3_32, -1, 17, 0},
/* 33 */ { 2, s_3_33, -1, 34, 0}
};

static symbol s_0[] = { 'e' };
static symbol s_1[] = { 'e' };
static symbol s_2[] = { 'f' };
static symbol s_3[] = { 't' };
static symbol s_4[] = { 'l', 'l' };
static symbol s_5[] = { 'o' };
static symbol s_6[] = { 'e' };
static symbol s_7[] = { 'a' };
static symbol s_8[] = { 'e' };
static symbol s_9[] = { 'l' };
static symbol s_10[] = { 'i' };
static symbol s_11[] = { 'e' };
static symbol s_12[] = { 'u' };
static symbol s_13[] = { 'u' };
static symbol s_14[] = { 'x' };
static symbol s_15[] = { 's' };
static symbol s_16[] = { 'o' };
static symbol s_17[] = { 'a' };
static symbol s_18[] = { 'c' };
static symbol s_19[] = { 'e' };
static symbol s_20[] = { 'm' };
static symbol s_21[] = { 's' };
static symbol s_22[] = { 'l' };
static symbol s_23[] = { 'i' };
static symbol s_24[] = { 'c' };
static symbol s_25[] = { 'l' };
static symbol s_26[] = { 'n' };
static symbol s_27[] = { 'n' };
static symbol s_28[] = { 'r' };
static symbol s_29[] = { 'd', 'r' };
static symbol s_30[] = { 't' };
static symbol s_31[] = { 't' };
static symbol s_32[] = { 's' };
static symbol s_33[] = { 't' };
static symbol s_34[] = { 'o' };
static symbol s_35[] = { 'l' };
static symbol s_36[] = { 'm' };
static symbol s_37[] = { 'n' };
static symbol s_38[] = { 'r' };
static symbol s_39[] = { 'c' };
static symbol s_40[] = { 's' };
static symbol s_41[] = { 'u' };
static symbol s_42[] = { 'l' };
static symbol s_43[] = { 'i' };
static symbol s_44[] = { 'e' };
static symbol s_45[] = { 'u' };
static symbol s_46[] = { 'i', 'n' };
static symbol s_47[] = { 'f' };
static symbol s_48[] = { 'm', 'e', 't' };
static symbol s_49[] = { 'r', 'y', 's', 't' };
static symbol s_50[] = { 'l' };
static symbol s_51[] = { 'i', 'e', 'f' };
static symbol s_52[] = { 'u', 'c' };
static symbol s_53[] = { 'u', 'm' };
static symbol s_54[] = { 'r', 'b' };
static symbol s_55[] = { 'u', 'r' };
static symbol s_56[] = { 'i', 's', 't', 'e', 'r' };
static symbol s_57[] = { 'm', 'e', 't', 'e', 'r' };
static symbol s_58[] = { 'o', 'l', 'u', 't' };
static symbol s_59[] = { 'a' };
static symbol s_60[] = { 'i' };
static symbol s_61[] = { 'o' };
static symbol s_62[] = { 'l' };
static symbol s_63[] = { 'b', 'i', 'c' };
static symbol s_64[] = { 'd', 'i', 'c' };
static symbol s_65[] = { 'p', 'i', 'c' };
static symbol s_66[] = { 't', 'i', 'c' };
static symbol s_67[] = { 'a', 'c' };
static symbol s_68[] = { 'e', 'c' };
static symbol s_69[] = { 'i', 'c' };
static symbol s_70[] = { 'l', 'u', 'c' };
static symbol s_71[] = { 'u', 'a', 's' };
static symbol s_72[] = { 'v', 'a', 's' };
static symbol s_73[] = { 'c', 'i', 's' };
static symbol s_74[] = { 'l', 'i', 's' };
static symbol s_75[] = { 'e', 'r', 'i', 's' };
static symbol s_76[] = { 'p', 'a', 'n', 's' };
static symbol s_77[] = { 's' };
static symbol s_78[] = { 'e', 'n', 's' };
static symbol s_79[] = { 'o', 'n', 's' };
static symbol s_80[] = { 'l', 'u', 's' };
static symbol s_81[] = { 'r', 'u', 's' };
static symbol s_82[] = { 'p' };
static symbol s_83[] = { 't' };
static symbol s_84[] = { 'h', 'e', 's' };
static symbol s_85[] = { 'm', 'i', 's' };
static symbol s_86[] = { 'm' };
static symbol s_87[] = { 'e', 'n', 's' };
static symbol s_88[] = { 'e', 'r', 's' };
static symbol s_89[] = { 'n' };
static symbol s_90[] = { 'e', 's' };
static symbol s_91[] = { 'y', 's' };
static symbol s_92[] = { 'y', 's' };

static int r_A(struct SN_env * z) {
    {   int c = z->c - 2;
        if (z->lb > c || c > z->l) return 0;
        z->c = c; /* hop, line 21 */
    }
    return 1;
}

static int r_B(struct SN_env * z) {
    {   int c = z->c - 3;
        if (z->lb > c || c > z->l) return 0;
        z->c = c; /* hop, line 22 */
    }
    return 1;
}

static int r_C(struct SN_env * z) {
    {   int c = z->c - 4;
        if (z->lb > c || c > z->l) return 0;
        z->c = c; /* hop, line 23 */
    }
    return 1;
}

static int r_D(struct SN_env * z) {
    {   int c = z->c - 5;
        if (z->lb > c || c > z->l) return 0;
        z->c = c; /* hop, line 24 */
    }
    return 1;
}

static int r_E(struct SN_env * z) {
    {   int m_test = z->l - z->c; /* test, line 25 */
        {   int c = z->c - 2;
            if (z->lb > c || c > z->l) return 0;
            z->c = c; /* hop, line 25 */
        }
        z->c = z->l - m_test;
    }
    {   int m = z->l - z->c; /* not, line 25 */
        if (!(eq_s_b(z, 1, s_0))) goto lab0;
        return 0;
    lab0:
        z->c = z->l - m;
    }
    return 1;
}

static int r_F(struct SN_env * z) {
    {   int m_test = z->l - z->c; /* test, line 26 */
        {   int c = z->c - 3;
            if (z->lb > c || c > z->l) return 0;
            z->c = c; /* hop, line 26 */
        }
        z->c = z->l - m_test;
    }
    {   int m = z->l - z->c; /* not, line 26 */
        if (!(eq_s_b(z, 1, s_1))) goto lab0;
        return 0;
    lab0:
        z->c = z->l - m;
    }
    return 1;
}

static int r_G(struct SN_env * z) {
    {   int m_test = z->l - z->c; /* test, line 27 */
        {   int c = z->c - 3;
            if (z->lb > c || c > z->l) return 0;
            z->c = c; /* hop, line 27 */
        }
        z->c = z->l - m_test;
    }
    if (!(eq_s_b(z, 1, s_2))) return 0;
    return 1;
}

static int r_H(struct SN_env * z) {
    {   int m_test = z->l - z->c; /* test, line 28 */
        {   int c = z->c - 2;
            if (z->lb > c || c > z->l) return 0;
            z->c = c; /* hop, line 28 */
        }
        z->c = z->l - m_test;
    }
    {   int m = z->l - z->c; /* or, line 28 */
        if (!(eq_s_b(z, 1, s_3))) goto lab1;
        goto lab0;
    lab1:
        z->c = z->l - m;
        if (!(eq_s_b(z, 2, s_4))) return 0;
    }
lab0:
    return 1;
}

static int r_I(struct SN_env * z) {
    {   int m_test = z->l - z->c; /* test, line 29 */
        {   int c = z->c - 2;
            if (z->lb > c || c > z->l) return 0;
            z->c = c; /* hop, line 29 */
        }
        z->c = z->l - m_test;
    }
    {   int m = z->l - z->c; /* not, line 29 */
        if (!(eq_s_b(z, 1, s_5))) goto lab0;
        return 0;
    lab0:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; /* not, line 29 */
        if (!(eq_s_b(z, 1, s_6))) goto lab1;
        return 0;
    lab1:
        z->c = z->l - m;
    }
    return 1;
}

static int r_J(struct SN_env * z) {
    {   int m_test = z->l - z->c; /* test, line 30 */
        {   int c = z->c - 2;
            if (z->lb > c || c > z->l) return 0;
            z->c = c; /* hop, line 30 */
        }
        z->c = z->l - m_test;
    }
    {   int m = z->l - z->c; /* not, line 30 */
        if (!(eq_s_b(z, 1, s_7))) goto lab0;
        return 0;
    lab0:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; /* not, line 30 */
        if (!(eq_s_b(z, 1, s_8))) goto lab1;
        return 0;
    lab1:
        z->c = z->l - m;
    }
    return 1;
}

static int r_K(struct SN_env * z) {
    {   int m_test = z->l - z->c; /* test, line 31 */
        {   int c = z->c - 3;
            if (z->lb > c || c > z->l) return 0;
            z->c = c; /* hop, line 31 */
        }
        z->c = z->l - m_test;
    }
    {   int m = z->l - z->c; /* or, line 31 */
        if (!(eq_s_b(z, 1, s_9))) goto lab1;
        goto lab0;
    lab1:
        z->c = z->l - m;
        if (!(eq_s_b(z, 1, s_10))) goto lab2;
        goto lab0;
    lab2:
        z->c = z->l - m;
        if (!(eq_s_b(z, 1, s_11))) return 0;
        if (z->c <= z->lb) return 0;
        z->c--; /* next, line 31 */
        if (!(eq_s_b(z, 1, s_12))) return 0;
    }
lab0:
    return 1;
}

static int r_L(struct SN_env * z) {
    {   int m_test = z->l - z->c; /* test, line 32 */
        {   int c = z->c - 2;
            if (z->lb > c || c > z->l) return 0;
            z->c = c; /* hop, line 32 */
        }
        z->c = z->l - m_test;
    }
    {   int m = z->l - z->c; /* not, line 32 */
        if (!(eq_s_b(z, 1, s_13))) goto lab0;
        return 0;
    lab0:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; /* not, line 32 */
        if (!(eq_s_b(z, 1, s_14))) goto lab1;
        return 0;
    lab1:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; /* not, line 32 */
        if (!(eq_s_b(z, 1, s_15))) goto lab2;
        {   int m = z->l - z->c; /* not, line 32 */
            if (!(eq_s_b(z, 1, s_16))) goto lab3;
            goto lab2;
        lab3:
            z->c = z->l - m;
        }
        return 0;
    lab2:
        z->c = z->l - m;
    }
    return 1;
}

static int r_M(struct SN_env * z) {
    {   int m_test = z->l - z->c; /* test, line 33 */
        {   int c = z->c - 2;
            if (z->lb > c || c > z->l) return 0;
            z->c = c; /* hop, line 33 */
        }
        z->c = z->l - m_test;
    }
    {   int m = z->l - z->c; /* not, line 33 */
        if (!(eq_s_b(z, 1, s_17))) goto lab0;
        return 0;
    lab0:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; /* not, line 33 */
        if (!(eq_s_b(z, 1, s_18))) goto lab1;
        return 0;
    lab1:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; /* not, line 33 */
        if (!(eq_s_b(z, 1, s_19))) goto lab2;
        return 0;
    lab2:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; /* not, line 33 */
        if (!(eq_s_b(z, 1, s_20))) goto lab3;
        return 0;
    lab3:
        z->c = z->l - m;
    }
    return 1;
}

static int r_N(struct SN_env * z) {
    {   int m_test = z->l - z->c; /* test, line 34 */
        {   int c = z->c - 3;
            if (z->lb > c || c > z->l) return 0;
            z->c = c; /* hop, line 34 */
        }
        z->c = z->l - m_test;
    }
    {   int c = z->c - 2;
        if (z->lb > c || c > z->l) return 0;
        z->c = c; /* hop, line 34 */
    }
    {   int m = z->l - z->c; /* or, line 34 */
        {   int m = z->l - z->c; /* not, line 34 */
            if (!(eq_s_b(z, 1, s_21))) goto lab2;
            goto lab1;
        lab2:
            z->c = z->l - m;
        }
        goto lab0;
    lab1:
        z->c = z->l - m;
        {   int c = z->c - 2;
            if (z->lb > c || c > z->l) return 0;
            z->c = c; /* hop, line 34 */
        }
    }
lab0:
    return 1;
}

static int r_O(struct SN_env * z) {
    {   int m_test = z->l - z->c; /* test, line 35 */
        {   int c = z->c - 2;
            if (z->lb > c || c > z->l) return 0;
            z->c = c; /* hop, line 35 */
        }
        z->c = z->l - m_test;
    }
    {   int m = z->l - z->c; /* or, line 35 */
        if (!(eq_s_b(z, 1, s_22))) goto lab1;
        goto lab0;
    lab1:
        z->c = z->l - m;
        if (!(eq_s_b(z, 1, s_23))) return 0;
    }
lab0:
    return 1;
}

static int r_P(struct SN_env * z) {
    {   int m_test = z->l - z->c; /* test, line 36 */
        {   int c = z->c - 2;
            if (z->lb > c || c > z->l) return 0;
            z->c = c; /* hop, line 36 */
        }
        z->c = z->l - m_test;
    }
    {   int m = z->l - z->c; /* not, line 36 */
        if (!(eq_s_b(z, 1, s_24))) goto lab0;
        return 0;
    lab0:
        z->c = z->l - m;
    }
    return 1;
}

static int r_Q(struct SN_env * z) {
    {   int m_test = z->l - z->c; /* test, line 37 */
        {   int c = z->c - 2;
            if (z->lb > c || c > z->l) return 0;
            z->c = c; /* hop, line 37 */
        }
        z->c = z->l - m_test;
    }
    {   int m_test = z->l - z->c; /* test, line 37 */
        {   int c = z->c - 3;
            if (z->lb > c || c > z->l) return 0;
            z->c = c; /* hop, line 37 */
        }
        z->c = z->l - m_test;
    }
    {   int m = z->l - z->c; /* not, line 37 */
        if (!(eq_s_b(z, 1, s_25))) goto lab0;
        return 0;
    lab0:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; /* not, line 37 */
        if (!(eq_s_b(z, 1, s_26))) goto lab1;
        return 0;
    lab1:
        z->c = z->l - m;
    }
    return 1;
}

static int r_R(struct SN_env * z) {
    {   int m_test = z->l - z->c; /* test, line 38 */
        {   int c = z->c - 2;
            if (z->lb > c || c > z->l) return 0;
            z->c = c; /* hop, line 38 */
        }
        z->c = z->l - m_test;
    }
    {   int m = z->l - z->c; /* or, line 38 */
        if (!(eq_s_b(z, 1, s_27))) goto lab1;
        goto lab0;
    lab1:
        z->c = z->l - m;
        if (!(eq_s_b(z, 1, s_28))) return 0;
    }
lab0:
    return 1;
}

static int r_S(struct SN_env * z) {
    {   int m_test = z->l - z->c; /* test, line 39 */
        {   int c = z->c - 2;
            if (z->lb > c || c > z->l) return 0;
            z->c = c; /* hop, line 39 */
        }
        z->c = z->l - m_test;
    }
    {   int m = z->l - z->c; /* or, line 39 */
        if (!(eq_s_b(z, 2, s_29))) goto lab1;
        goto lab0;
    lab1:
        z->c = z->l - m;
        if (!(eq_s_b(z, 1, s_30))) return 0;
        {   int m = z->l - z->c; /* not, line 39 */
            if (!(eq_s_b(z, 1, s_31))) goto lab2;
            return 0;
        lab2:
            z->c = z->l - m;
        }
    }
lab0:
    return 1;
}

static int r_T(struct SN_env * z) {
    {   int m_test = z->l - z->c; /* test, line 40 */
        {   int c = z->c - 2;
            if (z->lb > c || c > z->l) return 0;
            z->c = c; /* hop, line 40 */
        }
        z->c = z->l - m_test;
    }
    {   int m = z->l - z->c; /* or, line 40 */
        if (!(eq_s_b(z, 1, s_32))) goto lab1;
        goto lab0;
    lab1:
        z->c = z->l - m;
        if (!(eq_s_b(z, 1, s_33))) return 0;
        {   int m = z->l - z->c; /* not, line 40 */
            if (!(eq_s_b(z, 1, s_34))) goto lab2;
            return 0;
        lab2:
            z->c = z->l - m;
        }
    }
lab0:
    return 1;
}

static int r_U(struct SN_env * z) {
    {   int m_test = z->l - z->c; /* test, line 41 */
        {   int c = z->c - 2;
            if (z->lb > c || c > z->l) return 0;
            z->c = c; /* hop, line 41 */
        }
        z->c = z->l - m_test;
    }
    {   int m = z->l - z->c; /* or, line 41 */
        if (!(eq_s_b(z, 1, s_35))) goto lab1;
        goto lab0;
    lab1:
        z->c = z->l - m;
        if (!(eq_s_b(z, 1, s_36))) goto lab2;
        goto lab0;
    lab2:
        z->c = z->l - m;
        if (!(eq_s_b(z, 1, s_37))) goto lab3;
        goto lab0;
    lab3:
        z->c = z->l - m;
        if (!(eq_s_b(z, 1, s_38))) return 0;
    }
lab0:
    return 1;
}

static int r_V(struct SN_env * z) {
    {   int m_test = z->l - z->c; /* test, line 42 */
        {   int c = z->c - 2;
            if (z->lb > c || c > z->l) return 0;
            z->c = c; /* hop, line 42 */
        }
        z->c = z->l - m_test;
    }
    if (!(eq_s_b(z, 1, s_39))) return 0;
    return 1;
}

static int r_W(struct SN_env * z) {
    {   int m_test = z->l - z->c; /* test, line 43 */
        {   int c = z->c - 2;
            if (z->lb > c || c > z->l) return 0;
            z->c = c; /* hop, line 43 */
        }
        z->c = z->l - m_test;
    }
    {   int m = z->l - z->c; /* not, line 43 */
        if (!(eq_s_b(z, 1, s_40))) goto lab0;
        return 0;
    lab0:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; /* not, line 43 */
        if (!(eq_s_b(z, 1, s_41))) goto lab1;
        return 0;
    lab1:
        z->c = z->l - m;
    }
    return 1;
}

static int r_X(struct SN_env * z) {
    {   int m_test = z->l - z->c; /* test, line 44 */
        {   int c = z->c - 2;
            if (z->lb > c || c > z->l) return 0;
            z->c = c; /* hop, line 44 */
        }
        z->c = z->l - m_test;
    }
    {   int m = z->l - z->c; /* or, line 44 */
        if (!(eq_s_b(z, 1, s_42))) goto lab1;
        goto lab0;
    lab1:
        z->c = z->l - m;
        if (!(eq_s_b(z, 1, s_43))) goto lab2;
        goto lab0;
    lab2:
        z->c = z->l - m;
        if (!(eq_s_b(z, 1, s_44))) return 0;
        if (z->c <= z->lb) return 0;
        z->c--; /* next, line 44 */
        if (!(eq_s_b(z, 1, s_45))) return 0;
    }
lab0:
    return 1;
}

static int r_Y(struct SN_env * z) {
    {   int m_test = z->l - z->c; /* test, line 45 */
        {   int c = z->c - 2;
            if (z->lb > c || c > z->l) return 0;
            z->c = c; /* hop, line 45 */
        }
        z->c = z->l - m_test;
    }
    if (!(eq_s_b(z, 2, s_46))) return 0;
    return 1;
}

static int r_Z(struct SN_env * z) {
    {   int m_test = z->l - z->c; /* test, line 46 */
        {   int c = z->c - 2;
            if (z->lb > c || c > z->l) return 0;
            z->c = c; /* hop, line 46 */
        }
        z->c = z->l - m_test;
    }
    {   int m = z->l - z->c; /* not, line 46 */
        if (!(eq_s_b(z, 1, s_47))) goto lab0;
        return 0;
    lab0:
        z->c = z->l - m;
    }
    return 1;
}

static int r_AA(struct SN_env * z) {
    {   int m_test = z->l - z->c; /* test, line 47 */
        {   int c = z->c - 2;
            if (z->lb > c || c > z->l) return 0;
            z->c = c; /* hop, line 47 */
        }
        z->c = z->l - m_test;
    }
    if (!(find_among_b(z, a_0, 9))) return 0; /* among, line 47 */
    return 1;
}

static int r_BB(struct SN_env * z) {
    {   int m_test = z->l - z->c; /* test, line 48 */
        {   int c = z->c - 3;
            if (z->lb > c || c > z->l) return 0;
            z->c = c; /* hop, line 48 */
        }
        z->c = z->l - m_test;
    }
    {   int m = z->l - z->c; /* not, line 48 */
        if (!(eq_s_b(z, 3, s_48))) goto lab0;
        return 0;
    lab0:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; /* not, line 48 */
        if (!(eq_s_b(z, 4, s_49))) goto lab1;
        return 0;
    lab1:
        z->c = z->l - m;
    }
    return 1;
}

static int r_CC(struct SN_env * z) {
    {   int m_test = z->l - z->c; /* test, line 49 */
        {   int c = z->c - 2;
            if (z->lb > c || c > z->l) return 0;
            z->c = c; /* hop, line 49 */
        }
        z->c = z->l - m_test;
    }
    if (!(eq_s_b(z, 1, s_50))) return 0;
    return 1;
}

static int r_endings(struct SN_env * z) {
    int among_var;
    z->ket = z->c; /* [, line 55 */
    among_var = find_among_b(z, a_1, 294); /* substring, line 55 */
    if (!(among_var)) return 0;
    z->bra = z->c; /* ], line 55 */
    switch(among_var) {
        case 0: return 0;
        case 1:
            slice_del(z); /* delete, line 144 */
            break;
    }
    return 1;
}

static int r_undouble(struct SN_env * z) {
    {   int m_test = z->l - z->c; /* test, line 151 */
        if (!(find_among_b(z, a_2, 10))) return 0; /* substring, line 151 */
        z->c = z->l - m_test;
    }
    z->ket = z->c; /* [, line 152 */
    if (z->c <= z->lb) return 0;
    z->c--; /* next, line 152 */
    z->bra = z->c; /* ], line 152 */
    slice_del(z); /* delete, line 152 */
    return 1;
}

static int r_respell(struct SN_env * z) {
    int among_var;
    z->ket = z->c; /* [, line 158 */
    among_var = find_among_b(z, a_3, 34); /* substring, line 158 */
    if (!(among_var)) return 0;
    z->bra = z->c; /* ], line 158 */
    switch(among_var) {
        case 0: return 0;
        case 1:
            slice_from_s(z, 3, s_51); /* <-, line 159 */
            break;
        case 2:
            slice_from_s(z, 2, s_52); /* <-, line 160 */
            break;
        case 3:
            slice_from_s(z, 2, s_53); /* <-, line 161 */
            break;
        case 4:
            slice_from_s(z, 2, s_54); /* <-, line 162 */
            break;
        case 5:
            slice_from_s(z, 2, s_55); /* <-, line 163 */
            break;
        case 6:
            slice_from_s(z, 5, s_56); /* <-, line 164 */
            break;
        case 7:
            slice_from_s(z, 5, s_57); /* <-, line 165 */
            break;
        case 8:
            slice_from_s(z, 4, s_58); /* <-, line 166 */
            break;
        case 9:
            {   int m = z->l - z->c; /* not, line 167 */
                if (!(eq_s_b(z, 1, s_59))) goto lab0;
                return 0;
            lab0:
                z->c = z->l - m;
            }
            {   int m = z->l - z->c; /* not, line 167 */
                if (!(eq_s_b(z, 1, s_60))) goto lab1;
                return 0;
            lab1:
                z->c = z->l - m;
            }
            {   int m = z->l - z->c; /* not, line 167 */
                if (!(eq_s_b(z, 1, s_61))) goto lab2;
                return 0;
            lab2:
                z->c = z->l - m;
            }
            slice_from_s(z, 1, s_62); /* <-, line 167 */
            break;
        case 10:
            slice_from_s(z, 3, s_63); /* <-, line 168 */
            break;
        case 11:
            slice_from_s(z, 3, s_64); /* <-, line 169 */
            break;
        case 12:
            slice_from_s(z, 3, s_65); /* <-, line 170 */
            break;
        case 13:
            slice_from_s(z, 3, s_66); /* <-, line 171 */
            break;
        case 14:
            slice_from_s(z, 2, s_67); /* <-, line 172 */
            break;
        case 15:
            slice_from_s(z, 2, s_68); /* <-, line 173 */
            break;
        case 16:
            slice_from_s(z, 2, s_69); /* <-, line 174 */
            break;
        case 17:
            slice_from_s(z, 3, s_70); /* <-, line 175 */
            break;
        case 18:
            slice_from_s(z, 3, s_71); /* <-, line 176 */
            break;
        case 19:
            slice_from_s(z, 3, s_72); /* <-, line 177 */
            break;
        case 20:
            slice_from_s(z, 3, s_73); /* <-, line 178 */
            break;
        case 21:
            slice_from_s(z, 3, s_74); /* <-, line 179 */
            break;
        case 22:
            slice_from_s(z, 4, s_75); /* <-, line 180 */
            break;
        case 23:
            slice_from_s(z, 4, s_76); /* <-, line 181 */
            break;
        case 24:
            {   int m = z->l - z->c; /* not, line 182 */
                if (!(eq_s_b(z, 1, s_77))) goto lab3;
                return 0;
            lab3:
                z->c = z->l - m;
            }
            slice_from_s(z, 3, s_78); /* <-, line 182 */
            break;
        case 25:
            slice_from_s(z, 3, s_79); /* <-, line 183 */
            break;
        case 26:
            slice_from_s(z, 3, s_80); /* <-, line 184 */
            break;
        case 27:
            slice_from_s(z, 3, s_81); /* <-, line 185 */
            break;
        case 28:
            {   int m = z->l - z->c; /* not, line 186 */
                if (!(eq_s_b(z, 1, s_82))) goto lab4;
                return 0;
            lab4:
                z->c = z->l - m;
            }
            {   int m = z->l - z->c; /* not, line 186 */
                if (!(eq_s_b(z, 1, s_83))) goto lab5;
                return 0;
            lab5:
                z->c = z->l - m;
            }
            slice_from_s(z, 3, s_84); /* <-, line 186 */
            break;
        case 29:
            slice_from_s(z, 3, s_85); /* <-, line 187 */
            break;
        case 30:
            {   int m = z->l - z->c; /* not, line 188 */
                if (!(eq_s_b(z, 1, s_86))) goto lab6;
                return 0;
            lab6:
                z->c = z->l - m;
            }
            slice_from_s(z, 3, s_87); /* <-, line 188 */
            break;
        case 31:
            slice_from_s(z, 3, s_88); /* <-, line 189 */
            break;
        case 32:
            {   int m = z->l - z->c; /* not, line 190 */
                if (!(eq_s_b(z, 1, s_89))) goto lab7;
                return 0;
            lab7:
                z->c = z->l - m;
            }
            slice_from_s(z, 2, s_90); /* <-, line 190 */
            break;
        case 33:
            slice_from_s(z, 2, s_91); /* <-, line 191 */
            break;
        case 34:
            slice_from_s(z, 2, s_92); /* <-, line 192 */
            break;
    }
    return 1;
}

extern int snowball_lovins_stem(struct SN_env * z) {
    z->lb = z->c; z->c = z->l; /* backwards, line 199 */

    {   int m = z->l - z->c; /* do, line 200 */
        if (!r_endings(z)) goto lab0; /* call endings, line 200 */
    lab0:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; /* do, line 201 */
        if (!r_undouble(z)) goto lab1; /* call undouble, line 201 */
    lab1:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; /* do, line 202 */
        if (!r_respell(z)) goto lab2; /* call respell, line 202 */
    lab2:
        z->c = z->l - m;
    }
    z->c = z->lb;
    return 1;
}

extern struct SN_env * snowball_lovins_create_env(void) { return SN_create_env(0, 0, 0); }

extern void snowball_lovins_close_env(struct SN_env * z) { SN_close_env(z); }

