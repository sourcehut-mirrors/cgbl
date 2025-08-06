/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <string.h>
#include "processor.h"
#include "timer.h"

typedef enum {
    CGBL_INSTRUCTION_ADC_A = 0x8F, CGBL_INSTRUCTION_ADC_B = 0x88, CGBL_INSTRUCTION_ADC_C = 0x89, CGBL_INSTRUCTION_ADC_D = 0x8A,
    CGBL_INSTRUCTION_ADC_E = 0x8B, CGBL_INSTRUCTION_ADC_H = 0x8C, CGBL_INSTRUCTION_ADC_HLI = 0x8E, CGBL_INSTRUCTION_ADC_L = 0x8D,
    CGBL_INSTRUCTION_ADC_N = 0xCE,
    CGBL_INSTRUCTION_ADD_A = 0x87, CGBL_INSTRUCTION_ADD_B = 0x80, CGBL_INSTRUCTION_ADD_C = 0x81, CGBL_INSTRUCTION_ADD_D = 0x82,
    CGBL_INSTRUCTION_ADD_E = 0x83, CGBL_INSTRUCTION_ADD_H = 0x84, CGBL_INSTRUCTION_ADD_HL_BC = 0x09, CGBL_INSTRUCTION_ADD_HL_DE = 0x19,
    CGBL_INSTRUCTION_ADD_HL_HL = 0x29, CGBL_INSTRUCTION_ADD_HL_SP = 0x39, CGBL_INSTRUCTION_ADD_HLI = 0x86, CGBL_INSTRUCTION_ADD_L = 0x85,
    CGBL_INSTRUCTION_ADD_N = 0xC6,
    CGBL_INSTRUCTION_AND_A = 0xA7, CGBL_INSTRUCTION_AND_B = 0xA0, CGBL_INSTRUCTION_AND_C = 0xA1, CGBL_INSTRUCTION_AND_D = 0xA2,
    CGBL_INSTRUCTION_AND_E = 0xA3, CGBL_INSTRUCTION_AND_H = 0xA4, CGBL_INSTRUCTION_AND_HLI = 0xA6, CGBL_INSTRUCTION_AND_L = 0xA5,
    CGBL_INSTRUCTION_AND_N = 0xE6,
    CGBL_INSTRUCTION_BIT_0_A = 0x47, CGBL_INSTRUCTION_BIT_0_B = 0x40, CGBL_INSTRUCTION_BIT_0_C = 0x41, CGBL_INSTRUCTION_BIT_0_D = 0x42,
    CGBL_INSTRUCTION_BIT_0_E = 0x43, CGBL_INSTRUCTION_BIT_0_H = 0x44, CGBL_INSTRUCTION_BIT_0_HLI = 0x46, CGBL_INSTRUCTION_BIT_0_L = 0x45,
    CGBL_INSTRUCTION_BIT_1_A = 0x4F, CGBL_INSTRUCTION_BIT_1_B = 0x48, CGBL_INSTRUCTION_BIT_1_C = 0x49, CGBL_INSTRUCTION_BIT_1_D = 0x4A,
    CGBL_INSTRUCTION_BIT_1_E = 0x4B, CGBL_INSTRUCTION_BIT_1_H = 0x4C, CGBL_INSTRUCTION_BIT_1_HLI = 0x4E, CGBL_INSTRUCTION_BIT_1_L = 0x4D,
    CGBL_INSTRUCTION_BIT_2_A = 0x57, CGBL_INSTRUCTION_BIT_2_B = 0x50, CGBL_INSTRUCTION_BIT_2_C = 0x51, CGBL_INSTRUCTION_BIT_2_D = 0x52,
    CGBL_INSTRUCTION_BIT_2_E = 0x53, CGBL_INSTRUCTION_BIT_2_H = 0x54, CGBL_INSTRUCTION_BIT_2_HLI = 0x56, CGBL_INSTRUCTION_BIT_2_L = 0x55,
    CGBL_INSTRUCTION_BIT_3_A = 0x5F, CGBL_INSTRUCTION_BIT_3_B = 0x58, CGBL_INSTRUCTION_BIT_3_C = 0x59, CGBL_INSTRUCTION_BIT_3_D = 0x5A,
    CGBL_INSTRUCTION_BIT_3_E = 0x5B, CGBL_INSTRUCTION_BIT_3_H = 0x5C, CGBL_INSTRUCTION_BIT_3_HLI = 0x5E, CGBL_INSTRUCTION_BIT_3_L = 0x5D,
    CGBL_INSTRUCTION_BIT_4_A = 0x67, CGBL_INSTRUCTION_BIT_4_B = 0x60, CGBL_INSTRUCTION_BIT_4_C = 0x61, CGBL_INSTRUCTION_BIT_4_D = 0x62,
    CGBL_INSTRUCTION_BIT_4_E = 0x63, CGBL_INSTRUCTION_BIT_4_H = 0x64, CGBL_INSTRUCTION_BIT_4_HLI = 0x66, CGBL_INSTRUCTION_BIT_4_L = 0x65,
    CGBL_INSTRUCTION_BIT_5_A = 0x6F, CGBL_INSTRUCTION_BIT_5_B = 0x68, CGBL_INSTRUCTION_BIT_5_C = 0x69, CGBL_INSTRUCTION_BIT_5_D = 0x6A,
    CGBL_INSTRUCTION_BIT_5_E = 0x6B, CGBL_INSTRUCTION_BIT_5_H = 0x6C, CGBL_INSTRUCTION_BIT_5_HLI = 0x6E, CGBL_INSTRUCTION_BIT_5_L = 0x6D,
    CGBL_INSTRUCTION_BIT_6_A = 0x77, CGBL_INSTRUCTION_BIT_6_B = 0x70, CGBL_INSTRUCTION_BIT_6_C = 0x71, CGBL_INSTRUCTION_BIT_6_D = 0x72,
    CGBL_INSTRUCTION_BIT_6_E = 0x73, CGBL_INSTRUCTION_BIT_6_H = 0x74, CGBL_INSTRUCTION_BIT_6_HLI = 0x76, CGBL_INSTRUCTION_BIT_6_L = 0x75,
    CGBL_INSTRUCTION_BIT_7_A = 0x7F, CGBL_INSTRUCTION_BIT_7_B = 0x78, CGBL_INSTRUCTION_BIT_7_C = 0x79, CGBL_INSTRUCTION_BIT_7_D = 0x7A,
    CGBL_INSTRUCTION_BIT_7_E = 0x7B, CGBL_INSTRUCTION_BIT_7_H = 0x7C, CGBL_INSTRUCTION_BIT_7_HLI = 0x7E, CGBL_INSTRUCTION_BIT_7_L = 0x7D,
    CGBL_INSTRUCTION_CALL = 0xCD, CGBL_INSTRUCTION_CALL_C = 0xDC, CGBL_INSTRUCTION_CALL_NC = 0xD4, CGBL_INSTRUCTION_CALL_NZ = 0xC4,
    CGBL_INSTRUCTION_CALL_Z = 0xCC,
    CGBL_INSTRUCTION_CP_A = 0xBF, CGBL_INSTRUCTION_CP_B = 0xB8, CGBL_INSTRUCTION_CP_C = 0xB9, CGBL_INSTRUCTION_CP_D = 0xBA,
    CGBL_INSTRUCTION_CP_E = 0xBB, CGBL_INSTRUCTION_CP_H = 0xBC, CGBL_INSTRUCTION_CP_HLI = 0xBE, CGBL_INSTRUCTION_CP_L = 0xBD,
    CGBL_INSTRUCTION_CP_N = 0xFE,
    CGBL_INSTRUCTION_DEC_A = 0x3D, CGBL_INSTRUCTION_DEC_B = 0x05, CGBL_INSTRUCTION_DEC_BC = 0x0B, CGBL_INSTRUCTION_DEC_C = 0x0D,
    CGBL_INSTRUCTION_DEC_D = 0x15, CGBL_INSTRUCTION_DEC_DE = 0x1B, CGBL_INSTRUCTION_DEC_E = 0x1D, CGBL_INSTRUCTION_DEC_H = 0x25,
    CGBL_INSTRUCTION_DEC_HL = 0x2B, CGBL_INSTRUCTION_DEC_HLI = 0x35, CGBL_INSTRUCTION_DEC_L = 0x2D, CGBL_INSTRUCTION_DEC_SP = 0x3B,
    CGBL_INSTRUCTION_INC_A = 0x3C, CGBL_INSTRUCTION_INC_B = 0x04, CGBL_INSTRUCTION_INC_BC = 0x03, CGBL_INSTRUCTION_INC_C = 0x0C,
    CGBL_INSTRUCTION_INC_D = 0x14, CGBL_INSTRUCTION_INC_DE = 0x13, CGBL_INSTRUCTION_INC_E = 0x1C, CGBL_INSTRUCTION_INC_H = 0x24,
    CGBL_INSTRUCTION_INC_HL = 0x23, CGBL_INSTRUCTION_INC_HLI = 0x34, CGBL_INSTRUCTION_INC_L = 0x2C, CGBL_INSTRUCTION_INC_SP = 0x33,
    CGBL_INSTRUCTION_JP = 0xC3, CGBL_INSTRUCTION_JP_C = 0xDA, CGBL_INSTRUCTION_JP_NC = 0xD2, CGBL_INSTRUCTION_JP_NZ = 0xC2,
    CGBL_INSTRUCTION_JP_Z = 0xCA,
    CGBL_INSTRUCTION_JR = 0x18, CGBL_INSTRUCTION_JR_C = 0x38, CGBL_INSTRUCTION_JR_NC = 0x30, CGBL_INSTRUCTION_JR_NZ = 0x20,
    CGBL_INSTRUCTION_JR_Z = 0x28,
    CGBL_INSTRUCTION_LD_A_A = 0x7F, CGBL_INSTRUCTION_LD_A_B = 0x78, CGBL_INSTRUCTION_LD_A_BCI = 0x0A, CGBL_INSTRUCTION_LD_A_C = 0x79,
    CGBL_INSTRUCTION_LD_A_D = 0x7A, CGBL_INSTRUCTION_LD_A_DEI = 0x1A, CGBL_INSTRUCTION_LD_A_E = 0x7B, CGBL_INSTRUCTION_LD_A_H = 0x7C,
    CGBL_INSTRUCTION_LD_A_HLI = 0x7E, CGBL_INSTRUCTION_LD_A_HLID = 0x3A, CGBL_INSTRUCTION_LD_A_HLII = 0x2A, CGBL_INSTRUCTION_LD_A_L = 0x7D,
    CGBL_INSTRUCTION_LD_A_N = 0x3E, CGBL_INSTRUCTION_LD_A_NNI = 0xFA, CGBL_INSTRUCTION_LD_A_FF00_CI = 0xF2, CGBL_INSTRUCTION_LD_A_FF00_NI = 0xF0,
    CGBL_INSTRUCTION_LD_B_A = 0x47, CGBL_INSTRUCTION_LD_B_B = 0x40, CGBL_INSTRUCTION_LD_B_C = 0x41, CGBL_INSTRUCTION_LD_B_D = 0x42,
    CGBL_INSTRUCTION_LD_B_E = 0x43, CGBL_INSTRUCTION_LD_B_H = 0x44, CGBL_INSTRUCTION_LD_B_HLI = 0x46, CGBL_INSTRUCTION_LD_B_L = 0x45,
    CGBL_INSTRUCTION_LD_B_N = 0x06, CGBL_INSTRUCTION_LD_BC_NN = 0x01, CGBL_INSTRUCTION_LD_BCI_A = 0x02, CGBL_INSTRUCTION_LD_C_A = 0x4F,
    CGBL_INSTRUCTION_LD_C_B = 0x48, CGBL_INSTRUCTION_LD_C_C = 0x49, CGBL_INSTRUCTION_LD_C_D = 0x4A, CGBL_INSTRUCTION_LD_C_E = 0x4B,
    CGBL_INSTRUCTION_LD_C_H = 0x4C, CGBL_INSTRUCTION_LD_C_HLI = 0x4E, CGBL_INSTRUCTION_LD_C_L = 0x4D, CGBL_INSTRUCTION_LD_C_N = 0x0E,
    CGBL_INSTRUCTION_LD_D_A = 0x57, CGBL_INSTRUCTION_LD_D_B = 0x50, CGBL_INSTRUCTION_LD_D_C = 0x51, CGBL_INSTRUCTION_LD_D_D = 0x52,
    CGBL_INSTRUCTION_LD_D_E = 0x53, CGBL_INSTRUCTION_LD_D_H = 0x54, CGBL_INSTRUCTION_LD_D_HLI = 0x56, CGBL_INSTRUCTION_LD_D_L = 0x55,
    CGBL_INSTRUCTION_LD_D_N = 0x16, CGBL_INSTRUCTION_LD_DE_NN = 0x11, CGBL_INSTRUCTION_LD_DEI_A = 0x12, CGBL_INSTRUCTION_LD_E_A = 0x5F,
    CGBL_INSTRUCTION_LD_E_B = 0x58, CGBL_INSTRUCTION_LD_E_C = 0x59, CGBL_INSTRUCTION_LD_E_D = 0x5A, CGBL_INSTRUCTION_LD_E_E = 0x5B,
    CGBL_INSTRUCTION_LD_E_H = 0x5C, CGBL_INSTRUCTION_LD_E_HLI = 0x5E, CGBL_INSTRUCTION_LD_E_L = 0x5D, CGBL_INSTRUCTION_LD_E_N = 0x1E,
    CGBL_INSTRUCTION_LD_FF00_CI_A = 0xE2, CGBL_INSTRUCTION_LD_FF00_NI_A = 0xE0, CGBL_INSTRUCTION_LD_H_A = 0x67, CGBL_INSTRUCTION_LD_H_B = 0x60,
    CGBL_INSTRUCTION_LD_H_C = 0x61, CGBL_INSTRUCTION_LD_H_D = 0x62, CGBL_INSTRUCTION_LD_H_E = 0x63, CGBL_INSTRUCTION_LD_H_H = 0x64,
    CGBL_INSTRUCTION_LD_H_HLI = 0x66, CGBL_INSTRUCTION_LD_H_L = 0x65, CGBL_INSTRUCTION_LD_H_N = 0x26, CGBL_INSTRUCTION_LD_HL_NN = 0x21,
    CGBL_INSTRUCTION_LD_HLI_A = 0x77, CGBL_INSTRUCTION_LD_HLI_B = 0x70, CGBL_INSTRUCTION_LD_HLI_C = 0x71, CGBL_INSTRUCTION_LD_HLI_D = 0x72,
    CGBL_INSTRUCTION_LD_HLI_E = 0x73, CGBL_INSTRUCTION_LD_HLI_H = 0x74, CGBL_INSTRUCTION_LD_HLI_L = 0x75, CGBL_INSTRUCTION_LD_HLI_N = 0x36,
    CGBL_INSTRUCTION_LD_HLID_A = 0x32, CGBL_INSTRUCTION_LD_HLII_A = 0x22, CGBL_INSTRUCTION_LD_L_A = 0x6F, CGBL_INSTRUCTION_LD_L_B = 0x68,
    CGBL_INSTRUCTION_LD_L_C = 0x69, CGBL_INSTRUCTION_LD_L_D = 0x6A, CGBL_INSTRUCTION_LD_L_E = 0x6B, CGBL_INSTRUCTION_LD_L_H = 0x6C,
    CGBL_INSTRUCTION_LD_L_HLI = 0x6E, CGBL_INSTRUCTION_LD_L_L = 0x6D, CGBL_INSTRUCTION_LD_L_N = 0x2E, CGBL_INSTRUCTION_LD_NNI_A = 0xEA,
    CGBL_INSTRUCTION_LD_NNI_SP = 0x08, CGBL_INSTRUCTION_LD_SP_HL = 0xF9, CGBL_INSTRUCTION_LD_SP_NN = 0x31,
    CGBL_INSTRUCTION_OR_A = 0xB7, CGBL_INSTRUCTION_OR_B = 0xB0, CGBL_INSTRUCTION_OR_C = 0xB1, CGBL_INSTRUCTION_OR_D = 0xB2,
    CGBL_INSTRUCTION_OR_E = 0xB3, CGBL_INSTRUCTION_OR_H = 0xB4, CGBL_INSTRUCTION_OR_HLI = 0xB6, CGBL_INSTRUCTION_OR_L = 0xB5,
    CGBL_INSTRUCTION_OR_N = 0xF6,
    CGBL_INSTRUCTION_POP_AF = 0xF1, CGBL_INSTRUCTION_POP_BC = 0xC1, CGBL_INSTRUCTION_POP_DE = 0xD1, CGBL_INSTRUCTION_POP_HL = 0xE1,
    CGBL_INSTRUCTION_PUSH_AF = 0xF5, CGBL_INSTRUCTION_PUSH_BC = 0xC5, CGBL_INSTRUCTION_PUSH_DE = 0xD5, CGBL_INSTRUCTION_PUSH_HL = 0xE5,
    CGBL_INSTRUCTION_RES_0_A = 0x87, CGBL_INSTRUCTION_RES_0_B = 0x80, CGBL_INSTRUCTION_RES_0_C = 0x81, CGBL_INSTRUCTION_RES_0_D = 0x82,
    CGBL_INSTRUCTION_RES_0_E = 0x83, CGBL_INSTRUCTION_RES_0_H = 0x84, CGBL_INSTRUCTION_RES_0_HLI = 0x86, CGBL_INSTRUCTION_RES_0_L = 0x85,
    CGBL_INSTRUCTION_RES_1_A = 0x8F, CGBL_INSTRUCTION_RES_1_B = 0x88, CGBL_INSTRUCTION_RES_1_C = 0x89, CGBL_INSTRUCTION_RES_1_D = 0x8A,
    CGBL_INSTRUCTION_RES_1_E = 0x8B, CGBL_INSTRUCTION_RES_1_H = 0x8C, CGBL_INSTRUCTION_RES_1_HLI = 0x8E, CGBL_INSTRUCTION_RES_1_L = 0x8D,
    CGBL_INSTRUCTION_RES_2_A = 0x97, CGBL_INSTRUCTION_RES_2_B = 0x90, CGBL_INSTRUCTION_RES_2_C = 0x91, CGBL_INSTRUCTION_RES_2_D = 0x92,
    CGBL_INSTRUCTION_RES_2_E = 0x93, CGBL_INSTRUCTION_RES_2_H = 0x94, CGBL_INSTRUCTION_RES_2_HLI = 0x96, CGBL_INSTRUCTION_RES_2_L = 0x95,
    CGBL_INSTRUCTION_RES_3_A = 0x9F, CGBL_INSTRUCTION_RES_3_B = 0x98, CGBL_INSTRUCTION_RES_3_C = 0x99, CGBL_INSTRUCTION_RES_3_D = 0x9A,
    CGBL_INSTRUCTION_RES_3_E = 0x9B, CGBL_INSTRUCTION_RES_3_H = 0x9C, CGBL_INSTRUCTION_RES_3_HLI = 0x9E, CGBL_INSTRUCTION_RES_3_L = 0x9D,
    CGBL_INSTRUCTION_RES_4_A = 0xA7, CGBL_INSTRUCTION_RES_4_B = 0xA0, CGBL_INSTRUCTION_RES_4_C = 0xA1, CGBL_INSTRUCTION_RES_4_D = 0xA2,
    CGBL_INSTRUCTION_RES_4_E = 0xA3, CGBL_INSTRUCTION_RES_4_H = 0xA4, CGBL_INSTRUCTION_RES_4_HLI = 0xA6, CGBL_INSTRUCTION_RES_4_L = 0xA5,
    CGBL_INSTRUCTION_RES_5_A = 0xAF, CGBL_INSTRUCTION_RES_5_B = 0xA8, CGBL_INSTRUCTION_RES_5_C = 0xA9, CGBL_INSTRUCTION_RES_5_D = 0xAA,
    CGBL_INSTRUCTION_RES_5_E = 0xAB, CGBL_INSTRUCTION_RES_5_H = 0xAC, CGBL_INSTRUCTION_RES_5_HLI = 0xAE, CGBL_INSTRUCTION_RES_5_L = 0xAD,
    CGBL_INSTRUCTION_RES_6_A = 0xB7, CGBL_INSTRUCTION_RES_6_B = 0xB0, CGBL_INSTRUCTION_RES_6_C = 0xB1, CGBL_INSTRUCTION_RES_6_D = 0xB2,
    CGBL_INSTRUCTION_RES_6_E = 0xB3, CGBL_INSTRUCTION_RES_6_H = 0xB4, CGBL_INSTRUCTION_RES_6_HLI = 0xB6, CGBL_INSTRUCTION_RES_6_L = 0xB5,
    CGBL_INSTRUCTION_RES_7_A = 0xBF, CGBL_INSTRUCTION_RES_7_B = 0xB8, CGBL_INSTRUCTION_RES_7_C = 0xB9, CGBL_INSTRUCTION_RES_7_D = 0xBA,
    CGBL_INSTRUCTION_RES_7_E = 0xBB, CGBL_INSTRUCTION_RES_7_H = 0xBC, CGBL_INSTRUCTION_RES_7_HLI = 0xBE, CGBL_INSTRUCTION_RES_7_L = 0xBD,
    CGBL_INSTRUCTION_RL_A = 0x17, CGBL_INSTRUCTION_RL_B = 0x10, CGBL_INSTRUCTION_RL_C = 0x11, CGBL_INSTRUCTION_RL_D = 0x12,
    CGBL_INSTRUCTION_RL_E = 0x13, CGBL_INSTRUCTION_RL_H = 0x14, CGBL_INSTRUCTION_RL_HLI = 0x16, CGBL_INSTRUCTION_RL_L = 0x15,
    CGBL_INSTRUCTION_RLC_A = 0x07, CGBL_INSTRUCTION_RLC_B = 0x00, CGBL_INSTRUCTION_RLC_C = 0x01, CGBL_INSTRUCTION_RLC_D = 0x02,
    CGBL_INSTRUCTION_RLC_E = 0x03, CGBL_INSTRUCTION_RLC_H = 0x04, CGBL_INSTRUCTION_RLC_HLI = 0x06, CGBL_INSTRUCTION_RLC_L = 0x05,
    CGBL_INSTRUCTION_RR_A = 0x1F, CGBL_INSTRUCTION_RR_B = 0x18, CGBL_INSTRUCTION_RR_C = 0x19, CGBL_INSTRUCTION_RR_D = 0x1A,
    CGBL_INSTRUCTION_RR_E = 0x1B, CGBL_INSTRUCTION_RR_H = 0x1C, CGBL_INSTRUCTION_RR_HLI = 0x1E, CGBL_INSTRUCTION_RR_L = 0x1D,
    CGBL_INSTRUCTION_RRC_A = 0x0F, CGBL_INSTRUCTION_RRC_B = 0x08, CGBL_INSTRUCTION_RRC_C = 0x09, CGBL_INSTRUCTION_RRC_D = 0x0A,
    CGBL_INSTRUCTION_RRC_E = 0x0B, CGBL_INSTRUCTION_RRC_H = 0x0C, CGBL_INSTRUCTION_RRC_HLI = 0x0E, CGBL_INSTRUCTION_RRC_L = 0x0D,
    CGBL_INSTRUCTION_RET = 0xC9, CGBL_INSTRUCTION_RET_C = 0xD8, CGBL_INSTRUCTION_RET_NC = 0xD0, CGBL_INSTRUCTION_RET_NZ = 0xC0,
    CGBL_INSTRUCTION_RET_Z = 0xC8,
    CGBL_INSTRUCTION_RST_00 = 0xC7,
    CGBL_INSTRUCTION_SBC_A = 0x9F, CGBL_INSTRUCTION_SBC_B = 0x98, CGBL_INSTRUCTION_SBC_C = 0x99, CGBL_INSTRUCTION_SBC_D = 0x9A,
    CGBL_INSTRUCTION_SBC_E = 0x9B, CGBL_INSTRUCTION_SBC_H = 0x9C, CGBL_INSTRUCTION_SBC_HLI = 0x9E, CGBL_INSTRUCTION_SBC_L = 0x9D,
    CGBL_INSTRUCTION_SBC_N = 0xDE,
    CGBL_INSTRUCTION_SET_0_A = 0xC7, CGBL_INSTRUCTION_SET_0_B = 0xC0, CGBL_INSTRUCTION_SET_0_C = 0xC1, CGBL_INSTRUCTION_SET_0_D = 0xC2,
    CGBL_INSTRUCTION_SET_0_E = 0xC3, CGBL_INSTRUCTION_SET_0_H = 0xC4, CGBL_INSTRUCTION_SET_0_HLI = 0xC6, CGBL_INSTRUCTION_SET_0_L = 0xC5,
    CGBL_INSTRUCTION_SET_1_A = 0xCF, CGBL_INSTRUCTION_SET_1_B = 0xC8, CGBL_INSTRUCTION_SET_1_C = 0xC9, CGBL_INSTRUCTION_SET_1_D = 0xCA,
    CGBL_INSTRUCTION_SET_1_E = 0xCB, CGBL_INSTRUCTION_SET_1_H = 0xCC, CGBL_INSTRUCTION_SET_1_HLI = 0xCE, CGBL_INSTRUCTION_SET_1_L = 0xCD,
    CGBL_INSTRUCTION_SET_2_A = 0xD7, CGBL_INSTRUCTION_SET_2_B = 0xD0, CGBL_INSTRUCTION_SET_2_C = 0xD1, CGBL_INSTRUCTION_SET_2_D = 0xD2,
    CGBL_INSTRUCTION_SET_2_E = 0xD3, CGBL_INSTRUCTION_SET_2_H = 0xD4, CGBL_INSTRUCTION_SET_2_HLI = 0xD6, CGBL_INSTRUCTION_SET_2_L = 0xD5,
    CGBL_INSTRUCTION_SET_3_A = 0xDF, CGBL_INSTRUCTION_SET_3_B = 0xD8, CGBL_INSTRUCTION_SET_3_C = 0xD9, CGBL_INSTRUCTION_SET_3_D = 0xDA,
    CGBL_INSTRUCTION_SET_3_E = 0xDB, CGBL_INSTRUCTION_SET_3_H = 0xDC, CGBL_INSTRUCTION_SET_3_HLI = 0xDE, CGBL_INSTRUCTION_SET_3_L = 0xDD,
    CGBL_INSTRUCTION_SET_4_A = 0xE7, CGBL_INSTRUCTION_SET_4_B = 0xE0, CGBL_INSTRUCTION_SET_4_C = 0xE1, CGBL_INSTRUCTION_SET_4_D = 0xE2,
    CGBL_INSTRUCTION_SET_4_E = 0xE3, CGBL_INSTRUCTION_SET_4_H = 0xE4, CGBL_INSTRUCTION_SET_4_HLI = 0xE6, CGBL_INSTRUCTION_SET_4_L = 0xE5,
    CGBL_INSTRUCTION_SET_5_A = 0xEF, CGBL_INSTRUCTION_SET_5_B = 0xE8, CGBL_INSTRUCTION_SET_5_C = 0xE9, CGBL_INSTRUCTION_SET_5_D = 0xEA,
    CGBL_INSTRUCTION_SET_5_E = 0xEB, CGBL_INSTRUCTION_SET_5_H = 0xEC, CGBL_INSTRUCTION_SET_5_HLI = 0xEE, CGBL_INSTRUCTION_SET_5_L = 0xED,
    CGBL_INSTRUCTION_SET_6_A = 0xF7, CGBL_INSTRUCTION_SET_6_B = 0xF0, CGBL_INSTRUCTION_SET_6_C = 0xF1, CGBL_INSTRUCTION_SET_6_D = 0xF2,
    CGBL_INSTRUCTION_SET_6_E = 0xF3, CGBL_INSTRUCTION_SET_6_H = 0xF4, CGBL_INSTRUCTION_SET_6_HLI = 0xF6, CGBL_INSTRUCTION_SET_6_L = 0xF5,
    CGBL_INSTRUCTION_SET_7_A = 0xFF, CGBL_INSTRUCTION_SET_7_B = 0xF8, CGBL_INSTRUCTION_SET_7_C = 0xF9, CGBL_INSTRUCTION_SET_7_D = 0xFA,
    CGBL_INSTRUCTION_SET_7_E = 0xFB, CGBL_INSTRUCTION_SET_7_H = 0xFC, CGBL_INSTRUCTION_SET_7_HLI = 0xFE, CGBL_INSTRUCTION_SET_7_L = 0xFD,
    CGBL_INSTRUCTION_SLA_A = 0x27, CGBL_INSTRUCTION_SLA_B = 0x20, CGBL_INSTRUCTION_SLA_C = 0x21, CGBL_INSTRUCTION_SLA_D = 0x22,
    CGBL_INSTRUCTION_SLA_E = 0x23, CGBL_INSTRUCTION_SLA_H = 0x24, CGBL_INSTRUCTION_SLA_HLI = 0x26, CGBL_INSTRUCTION_SLA_L = 0x25,
    CGBL_INSTRUCTION_SRA_A = 0x2F, CGBL_INSTRUCTION_SRA_B = 0x28, CGBL_INSTRUCTION_SRA_C = 0x29, CGBL_INSTRUCTION_SRA_D = 0x2A,
    CGBL_INSTRUCTION_SRA_E = 0x2B, CGBL_INSTRUCTION_SRA_H = 0x2C, CGBL_INSTRUCTION_SRA_HLI = 0x2E, CGBL_INSTRUCTION_SRA_L = 0x2D,
    CGBL_INSTRUCTION_SRL_A = 0x3F, CGBL_INSTRUCTION_SRL_B = 0x38, CGBL_INSTRUCTION_SRL_C = 0x39, CGBL_INSTRUCTION_SRL_D = 0x3A,
    CGBL_INSTRUCTION_SRL_E = 0x3B, CGBL_INSTRUCTION_SRL_H = 0x3C, CGBL_INSTRUCTION_SRL_HLI = 0x3E, CGBL_INSTRUCTION_SRL_L = 0x3D,
    CGBL_INSTRUCTION_SUB_A = 0x97, CGBL_INSTRUCTION_SUB_B = 0x90, CGBL_INSTRUCTION_SUB_C = 0x91, CGBL_INSTRUCTION_SUB_D = 0x92,
    CGBL_INSTRUCTION_SUB_E = 0x93, CGBL_INSTRUCTION_SUB_H = 0x94, CGBL_INSTRUCTION_SUB_HLI = 0x96, CGBL_INSTRUCTION_SUB_L = 0x95,
    CGBL_INSTRUCTION_SUB_N = 0xD6,
    CGBL_INSTRUCTION_SWAP_A = 0x37, CGBL_INSTRUCTION_SWAP_B = 0x30, CGBL_INSTRUCTION_SWAP_C = 0x31, CGBL_INSTRUCTION_SWAP_D = 0x32,
    CGBL_INSTRUCTION_SWAP_E = 0x33, CGBL_INSTRUCTION_SWAP_H = 0x34, CGBL_INSTRUCTION_SWAP_HLI = 0x36, CGBL_INSTRUCTION_SWAP_L = 0x35,
    CGBL_INSTRUCTION_XOR_A = 0xAF, CGBL_INSTRUCTION_XOR_B = 0xA8, CGBL_INSTRUCTION_XOR_C = 0xA9, CGBL_INSTRUCTION_XOR_D = 0xAA,
    CGBL_INSTRUCTION_XOR_E = 0xAB, CGBL_INSTRUCTION_XOR_H = 0xAC, CGBL_INSTRUCTION_XOR_HLI = 0xAE, CGBL_INSTRUCTION_XOR_L = 0xAD,
    CGBL_INSTRUCTION_XOR_N = 0xEE,
    CGBL_INSTRUCTION_PREFIX = 0xCB,
    CGBL_INSTRUCTION_MAX = 256,
} cgbl_instruction_e;

typedef union {
    struct {
        uint8_t : 4;
        uint8_t input : 1;
    };
    uint8_t raw;
} cgbl_interrupt_t;

typedef union {
    struct {
        union {
            struct {
                uint8_t : 4;
                uint8_t carry : 1;
                uint8_t half_carry : 1;
                uint8_t negative : 1;
                uint8_t zero : 1;
            };
            uint8_t low;
        };
        uint8_t high;
    };
    uint16_t word;
} cgbl_register_t;

static struct {
    bool halt_bug;
    bool halted;
    bool stopped;
    uint8_t delay;
    cgbl_register_t af;
    cgbl_register_t bc;
    cgbl_register_t de;
    cgbl_register_t hl;
    cgbl_register_t pc;
    cgbl_register_t sp;
    struct {
        uint16_t address;
        cgbl_instruction_e opcode;
    } instruction;
    struct {
        bool enabled;
        uint8_t delay;
        cgbl_interrupt_t enable;
        cgbl_interrupt_t flag;
    } interrupt;
} processor = {};

static cgbl_error_e cgbl_processor_instruction_adc(void)
{
    uint16_t carry = 0, sum = 0;
    cgbl_register_t operand = {};
    processor.delay = 4;
    switch (processor.instruction.opcode)
    {
        case CGBL_INSTRUCTION_ADC_A:
            operand.low = processor.af.high;
            break;
        case CGBL_INSTRUCTION_ADC_B:
            operand.low = processor.bc.high;
            break;
        case CGBL_INSTRUCTION_ADC_C:
            operand.low = processor.bc.low;
            break;
        case CGBL_INSTRUCTION_ADC_D:
            operand.low = processor.de.high;
            break;
        case CGBL_INSTRUCTION_ADC_E:
            operand.low = processor.de.low;
            break;
        case CGBL_INSTRUCTION_ADC_H:
            operand.low = processor.hl.high;
            break;
        case CGBL_INSTRUCTION_ADC_HLI:
            processor.delay += 4;
            operand.low = cgbl_bus_read(processor.hl.word);
            break;
        case CGBL_INSTRUCTION_ADC_L:
            operand.low = processor.hl.low;
            break;
        case CGBL_INSTRUCTION_ADC_N:
            processor.delay += 4;
            operand.low = cgbl_bus_read(processor.pc.word++);
            break;
        default:
            break;
    }
    sum = processor.af.high + operand.low + processor.af.carry;
    carry = processor.af.high ^ operand.low ^ processor.af.carry ^ sum;
    processor.af.carry = ((carry & 0x100) == 0x100);
    processor.af.half_carry = ((carry & 0x10) == 0x10);
    processor.af.negative = false;
    processor.af.zero = !(sum & 0xFF);
    processor.af.high = sum;
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_add(void)
{
    uint16_t carry = 0, sum = 0;
    cgbl_register_t operand = {};
    processor.delay = 4;
    switch (processor.instruction.opcode)
    {
        case CGBL_INSTRUCTION_ADD_A:
            operand.low = processor.af.high;
            break;
        case CGBL_INSTRUCTION_ADD_B:
            operand.low = processor.bc.high;
            break;
        case CGBL_INSTRUCTION_ADD_C:
            operand.low = processor.bc.low;
            break;
        case CGBL_INSTRUCTION_ADD_D:
            operand.low = processor.de.high;
            break;
        case CGBL_INSTRUCTION_ADD_E:
            operand.low = processor.de.low;
            break;
        case CGBL_INSTRUCTION_ADD_H:
            operand.low = processor.hl.high;
            break;
        case CGBL_INSTRUCTION_ADD_HLI:
            processor.delay += 4;
            operand.low = cgbl_bus_read(processor.hl.word);
            break;
        case CGBL_INSTRUCTION_ADD_L:
            operand.low = processor.hl.low;
            break;
        case CGBL_INSTRUCTION_ADD_N:
            processor.delay += 4;
            operand.low = cgbl_bus_read(processor.pc.word++);
            break;
        default:
            break;
    }
    sum = processor.af.high + operand.low;
    carry = processor.af.high ^ operand.low ^ sum;
    processor.af.carry = ((carry & 0x100) == 0x100);
    processor.af.half_carry = ((carry & 0x10) == 0x10);
    processor.af.negative = false;
    processor.af.zero = !(sum & 0xFF);
    processor.af.high = sum;
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_add_hl(void)
{
    uint32_t carry = 0, sum = 0;
    cgbl_register_t operand = {};
    processor.delay = 8;
    switch (processor.instruction.opcode)
    {
        case CGBL_INSTRUCTION_ADD_HL_BC:
            operand.word = processor.bc.word;
            break;
        case CGBL_INSTRUCTION_ADD_HL_DE:
            operand.word = processor.de.word;
            break;
        case CGBL_INSTRUCTION_ADD_HL_HL:
            operand.word = processor.hl.word;
            break;
        case CGBL_INSTRUCTION_ADD_HL_SP:
            operand.word = processor.sp.word;
            break;
        default:
            break;
    }
    sum = processor.hl.word + operand.word;
    carry = processor.hl.word ^ operand.word ^ sum;
    processor.af.carry = ((carry & 0x10000) == 0x10000);
    processor.af.half_carry = ((carry & 0x1000) == 0x1000);
    processor.af.negative = false;
    processor.hl.word = sum;
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_add_sp(void)
{
    uint32_t carry = 0, sum = 0;
    cgbl_register_t operand = { .low = cgbl_bus_read(processor.pc.word++) };
    processor.delay = 16;
    sum = processor.sp.word + (int8_t)operand.low;
    carry = processor.sp.word ^ (int8_t)operand.low ^ sum;
    processor.af.carry = ((carry & 0x100) == 0x100);
    processor.af.half_carry = ((carry & 0x10) == 0x10);
    processor.af.negative = false;
    processor.af.zero = false;
    processor.sp.word = sum;
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_and(void)
{
    processor.delay = 4;
    switch (processor.instruction.opcode)
    {
        case CGBL_INSTRUCTION_AND_A:
            break;
        case CGBL_INSTRUCTION_AND_B:
            processor.af.high &= processor.bc.high;
            break;
        case CGBL_INSTRUCTION_AND_C:
            processor.af.high &= processor.bc.low;
            break;
        case CGBL_INSTRUCTION_AND_D:
            processor.af.high &= processor.de.high;
            break;
        case CGBL_INSTRUCTION_AND_E:
            processor.af.high &= processor.de.low;
            break;
        case CGBL_INSTRUCTION_AND_H:
            processor.af.high &= processor.hl.high;
            break;
        case CGBL_INSTRUCTION_AND_HLI:
            processor.delay += 4;
            processor.af.high &= cgbl_bus_read(processor.hl.word);
            break;
        case CGBL_INSTRUCTION_AND_L:
            processor.af.high &= processor.hl.low;
            break;
        case CGBL_INSTRUCTION_AND_N:
            processor.delay += 4;
            processor.af.high &= cgbl_bus_read(processor.pc.word++);
            break;
        default:
            break;
    }
    processor.af.carry = false;
    processor.af.half_carry = true;
    processor.af.negative = false;
    processor.af.zero = !processor.af.high;
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_bit(void)
{
    processor.delay = 8;
    switch (processor.instruction.opcode)
    {
        case CGBL_INSTRUCTION_BIT_0_A: case CGBL_INSTRUCTION_BIT_1_A: case CGBL_INSTRUCTION_BIT_2_A: case CGBL_INSTRUCTION_BIT_3_A:
        case CGBL_INSTRUCTION_BIT_4_A: case CGBL_INSTRUCTION_BIT_5_A: case CGBL_INSTRUCTION_BIT_6_A: case CGBL_INSTRUCTION_BIT_7_A:
            processor.af.zero = !(processor.af.high & (1 << ((processor.instruction.opcode - CGBL_INSTRUCTION_BIT_0_A) / 8)));
            break;
        case CGBL_INSTRUCTION_BIT_0_B: case CGBL_INSTRUCTION_BIT_1_B: case CGBL_INSTRUCTION_BIT_2_B: case CGBL_INSTRUCTION_BIT_3_B:
        case CGBL_INSTRUCTION_BIT_4_B: case CGBL_INSTRUCTION_BIT_5_B: case CGBL_INSTRUCTION_BIT_6_B: case CGBL_INSTRUCTION_BIT_7_B:
            processor.af.zero = !(processor.bc.high & (1 << ((processor.instruction.opcode - CGBL_INSTRUCTION_BIT_0_B) / 8)));
            break;
        case CGBL_INSTRUCTION_BIT_0_C: case CGBL_INSTRUCTION_BIT_1_C: case CGBL_INSTRUCTION_BIT_2_C: case CGBL_INSTRUCTION_BIT_3_C:
        case CGBL_INSTRUCTION_BIT_4_C: case CGBL_INSTRUCTION_BIT_5_C: case CGBL_INSTRUCTION_BIT_6_C: case CGBL_INSTRUCTION_BIT_7_C:
            processor.af.zero = !(processor.bc.low & (1 << ((processor.instruction.opcode - CGBL_INSTRUCTION_BIT_0_C) / 8)));
            break;
        case CGBL_INSTRUCTION_BIT_0_D: case CGBL_INSTRUCTION_BIT_1_D: case CGBL_INSTRUCTION_BIT_2_D: case CGBL_INSTRUCTION_BIT_3_D:
        case CGBL_INSTRUCTION_BIT_4_D: case CGBL_INSTRUCTION_BIT_5_D: case CGBL_INSTRUCTION_BIT_6_D: case CGBL_INSTRUCTION_BIT_7_D:
            processor.af.zero = !(processor.de.high & (1 << ((processor.instruction.opcode - CGBL_INSTRUCTION_BIT_0_D) / 8)));
            break;
        case CGBL_INSTRUCTION_BIT_0_E: case CGBL_INSTRUCTION_BIT_1_E: case CGBL_INSTRUCTION_BIT_2_E: case CGBL_INSTRUCTION_BIT_3_E:
        case CGBL_INSTRUCTION_BIT_4_E: case CGBL_INSTRUCTION_BIT_5_E: case CGBL_INSTRUCTION_BIT_6_E: case CGBL_INSTRUCTION_BIT_7_E:
            processor.af.zero = !(processor.de.low & (1 << ((processor.instruction.opcode - CGBL_INSTRUCTION_BIT_0_E) / 8)));
            break;
        case CGBL_INSTRUCTION_BIT_0_H: case CGBL_INSTRUCTION_BIT_1_H: case CGBL_INSTRUCTION_BIT_2_H: case CGBL_INSTRUCTION_BIT_3_H:
        case CGBL_INSTRUCTION_BIT_4_H: case CGBL_INSTRUCTION_BIT_5_H: case CGBL_INSTRUCTION_BIT_6_H: case CGBL_INSTRUCTION_BIT_7_H:
            processor.af.zero = !(processor.hl.high & (1 << ((processor.instruction.opcode - CGBL_INSTRUCTION_BIT_0_H) / 8)));
            break;
        case CGBL_INSTRUCTION_BIT_0_HLI: case CGBL_INSTRUCTION_BIT_1_HLI: case CGBL_INSTRUCTION_BIT_2_HLI: case CGBL_INSTRUCTION_BIT_3_HLI:
        case CGBL_INSTRUCTION_BIT_4_HLI: case CGBL_INSTRUCTION_BIT_5_HLI: case CGBL_INSTRUCTION_BIT_6_HLI: case CGBL_INSTRUCTION_BIT_7_HLI:
            processor.delay += 4;
            processor.af.zero = !(cgbl_bus_read(processor.hl.word) & (1 << ((processor.instruction.opcode - CGBL_INSTRUCTION_BIT_0_HLI) / 8)));
            break;
        case CGBL_INSTRUCTION_BIT_0_L: case CGBL_INSTRUCTION_BIT_1_L: case CGBL_INSTRUCTION_BIT_2_L: case CGBL_INSTRUCTION_BIT_3_L:
        case CGBL_INSTRUCTION_BIT_4_L: case CGBL_INSTRUCTION_BIT_5_L: case CGBL_INSTRUCTION_BIT_6_L: case CGBL_INSTRUCTION_BIT_7_L:
            processor.af.zero = !(processor.hl.low & (1 << ((processor.instruction.opcode - CGBL_INSTRUCTION_BIT_0_L) / 8)));
            break;
        default:
            break;
    }
    processor.af.half_carry = true;
    processor.af.negative = false;
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_call(void)
{
    bool taken = false;
    cgbl_register_t operand = {
        .low = cgbl_bus_read(processor.pc.word++),
        .high = cgbl_bus_read(processor.pc.word++)
    };
    processor.delay = 12;
    switch (processor.instruction.opcode)
    {
        case CGBL_INSTRUCTION_CALL:
            taken = true;
            break;
        case CGBL_INSTRUCTION_CALL_C:
            taken = processor.af.carry;
            break;
        case CGBL_INSTRUCTION_CALL_NC:
            taken = !processor.af.carry;
            break;
        case CGBL_INSTRUCTION_CALL_NZ:
            taken = !processor.af.zero;
            break;
        case CGBL_INSTRUCTION_CALL_Z:
            taken = processor.af.zero;
            break;
        default:
            break;
    }
    if (taken)
    {
        processor.delay += 12;
        cgbl_bus_write(--processor.sp.word, processor.pc.high);
        cgbl_bus_write(--processor.sp.word, processor.pc.low);
        processor.pc.word = operand.word;
    }
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_ccf(void)
{
    processor.delay = 4;
    processor.af.carry = !processor.af.carry;
    processor.af.half_carry = false;
    processor.af.negative = false;
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_cp(void)
{
    cgbl_register_t operand = {};
    processor.delay = 4;
    switch (processor.instruction.opcode)
    {
        case CGBL_INSTRUCTION_CP_A:
            operand.low = processor.af.high;
            break;
        case CGBL_INSTRUCTION_CP_B:
            operand.low = processor.bc.high;
            break;
        case CGBL_INSTRUCTION_CP_C:
            operand.low = processor.bc.low;
            break;
        case CGBL_INSTRUCTION_CP_D:
            operand.low = processor.de.high;
            break;
        case CGBL_INSTRUCTION_CP_E:
            operand.low = processor.de.low;
            break;
        case CGBL_INSTRUCTION_CP_H:
            operand.low = processor.hl.high;
            break;
        case CGBL_INSTRUCTION_CP_HLI:
            processor.delay += 4;
            operand.low = cgbl_bus_read(processor.hl.word);
            break;
        case CGBL_INSTRUCTION_CP_L:
            operand.low = processor.hl.low;
            break;
        case CGBL_INSTRUCTION_CP_N:
            processor.delay += 4;
            operand.low = cgbl_bus_read(processor.pc.word++);
            break;
        default:
            break;
    }
    processor.af.carry = (processor.af.high < operand.low);
    processor.af.half_carry = ((processor.af.high & 0x0F) < ((processor.af.high - operand.low) & 0x0F));
    processor.af.negative = true;
    processor.af.zero = (processor.af.high == operand.low);
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_cpl(void)
{
    processor.delay = 4;
    processor.af.high = ~processor.af.high;
    processor.af.half_carry = true;
    processor.af.negative = true;
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_daa(void)
{
    processor.delay = 4;
    if (!processor.af.negative)
    {
        if (processor.af.carry || (processor.af.high > 0x99))
        {
            processor.af.high += 0x60;
            processor.af.carry = true;
        }
        if (processor.af.half_carry || ((processor.af.high & 0xF) > 0x9))
        {
            processor.af.high += 0x6;
        }
    }
    else
    {
        if (processor.af.carry)
        {
            processor.af.high -= 0x60;
        }
        if (processor.af.half_carry)
        {
            processor.af.high -= 0x6;
        }
    }
    processor.af.half_carry = false;
    processor.af.zero = !processor.af.high;
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_dec(void)
{
    cgbl_register_t operand = {};
    processor.delay = 4;
    switch (processor.instruction.opcode)
    {
        case CGBL_INSTRUCTION_DEC_A:
            operand.low = --processor.af.high;
            break;
        case CGBL_INSTRUCTION_DEC_B:
            operand.low = --processor.bc.high;
            break;
        case CGBL_INSTRUCTION_DEC_C:
            operand.low = --processor.bc.low;
            break;
        case CGBL_INSTRUCTION_DEC_D:
            operand.low = --processor.de.high;
            break;
        case CGBL_INSTRUCTION_DEC_E:
            operand.low = --processor.de.low;
            break;
        case CGBL_INSTRUCTION_DEC_H:
            operand.low = --processor.hl.high;
            break;
        case CGBL_INSTRUCTION_DEC_HLI:
            processor.delay += 8;
            operand.low = cgbl_bus_read(processor.hl.word) - 1;
            cgbl_bus_write(processor.hl.word, operand.low);
            break;
        case CGBL_INSTRUCTION_DEC_L:
            operand.low = --processor.hl.low;
            break;
        default:
            break;
    }
    processor.af.half_carry = ((operand.low & 0x0F) == 0x0F);
    processor.af.negative = true;
    processor.af.zero = !operand.low;
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_dec_16(void)
{
    processor.delay = 8;
    switch (processor.instruction.opcode)
    {
        case CGBL_INSTRUCTION_DEC_BC:
            --processor.bc.word;
            break;
        case CGBL_INSTRUCTION_DEC_DE:
            --processor.de.word;
            break;
        case CGBL_INSTRUCTION_DEC_HL:
            --processor.hl.word;
            break;
        case CGBL_INSTRUCTION_DEC_SP:
            --processor.sp.word;
            break;
        default:
            break;
    }
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_di(void)
{
    processor.delay = 4;
    processor.interrupt.delay = 0;
    processor.interrupt.enabled = false;
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_ei(void)
{
    processor.delay = 4;
    if (!processor.interrupt.delay)
    {
        processor.interrupt.delay = 2;
    }
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_halt(void)
{
    processor.delay = 4;
    processor.halt_bug = !processor.interrupt.enabled && (processor.interrupt.enable.raw & processor.interrupt.flag.raw & 0x1F);
    processor.halted = true;
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_inc(void)
{
    cgbl_register_t operand = {};
    processor.delay = 4;
    switch (processor.instruction.opcode)
    {
        case CGBL_INSTRUCTION_INC_A:
            operand.low = ++processor.af.high;
            break;
        case CGBL_INSTRUCTION_INC_B:
            operand.low = ++processor.bc.high;
            break;
        case CGBL_INSTRUCTION_INC_C:
            operand.low = ++processor.bc.low;
            break;
        case CGBL_INSTRUCTION_INC_D:
            operand.low = ++processor.de.high;
            break;
        case CGBL_INSTRUCTION_INC_E:
            operand.low = ++processor.de.low;
            break;
        case CGBL_INSTRUCTION_INC_H:
            operand.low = ++processor.hl.high;
            break;
        case CGBL_INSTRUCTION_INC_HLI:
            processor.delay += 8;
            operand.low = cgbl_bus_read(processor.hl.word) + 1;
            cgbl_bus_write(processor.hl.word, operand.low);
            break;
        case CGBL_INSTRUCTION_INC_L:
            operand.low = ++processor.hl.low;
            break;
        default:
            break;
    }
    processor.af.half_carry = !(operand.low & 0x0F);
    processor.af.negative = false;
    processor.af.zero = !operand.low;
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_inc_16(void)
{
    processor.delay = 8;
    switch (processor.instruction.opcode)
    {
        case CGBL_INSTRUCTION_INC_BC:
            ++processor.bc.word;
            break;
        case CGBL_INSTRUCTION_INC_DE:
            ++processor.de.word;
            break;
        case CGBL_INSTRUCTION_INC_HL:
            ++processor.hl.word;
            break;
        case CGBL_INSTRUCTION_INC_SP:
            ++processor.sp.word;
            break;
        default:
            break;
    }
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_jp(void)
{
    bool taken = false;
    cgbl_register_t operand = {
        .low = cgbl_bus_read(processor.pc.word++),
        .high = cgbl_bus_read(processor.pc.word++)
    };
    processor.delay = 12;
    switch (processor.instruction.opcode)
    {
        case CGBL_INSTRUCTION_JP:
            taken = true;
            break;
        case CGBL_INSTRUCTION_JP_C:
            taken = processor.af.carry;
            break;
        case CGBL_INSTRUCTION_JP_NC:
            taken = !processor.af.carry;
            break;
        case CGBL_INSTRUCTION_JP_NZ:
            taken = !processor.af.zero;
            break;
        case CGBL_INSTRUCTION_JP_Z:
            taken = processor.af.zero;
            break;
        default:
            break;
    }
    if (taken)
    {
        processor.delay += 4;
        processor.pc.word = operand.word;
    }
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_jp_hl(void)
{
    processor.delay = 4;
    processor.pc.word = processor.hl.word;
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_jr(void)
{
    bool taken = false;
    cgbl_register_t operand = { .low = cgbl_bus_read(processor.pc.word++) };
    processor.delay = 8;
    switch (processor.instruction.opcode)
    {
        case CGBL_INSTRUCTION_JR:
            taken = true;
            break;
        case CGBL_INSTRUCTION_JR_C:
            taken = processor.af.carry;
            break;
        case CGBL_INSTRUCTION_JR_NC:
            taken = !processor.af.carry;
            break;
        case CGBL_INSTRUCTION_JR_NZ:
            taken = !processor.af.zero;
            break;
        case CGBL_INSTRUCTION_JR_Z:
            taken = processor.af.zero;
            break;
        default:
            break;
    }
    if (taken)
    {
        processor.delay += 4;
        processor.pc.word += (int8_t)operand.low;
    }
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_ld(void)
{
    cgbl_register_t operand = {};
    processor.delay = 4;
    switch (processor.instruction.opcode)
    {
        case CGBL_INSTRUCTION_LD_A_A:
            break;
        case CGBL_INSTRUCTION_LD_A_B:
            processor.af.high = processor.bc.high;
            break;
        case CGBL_INSTRUCTION_LD_A_BCI:
            processor.delay += 4;
            processor.af.high = cgbl_bus_read(processor.bc.word);
            break;
        case CGBL_INSTRUCTION_LD_A_C:
            processor.af.high = processor.bc.low;
            break;
        case CGBL_INSTRUCTION_LD_A_D:
            processor.af.high = processor.de.high;
            break;
        case CGBL_INSTRUCTION_LD_A_DEI:
            processor.delay += 4;
            processor.af.high = cgbl_bus_read(processor.de.word);
            break;
        case CGBL_INSTRUCTION_LD_A_E:
            processor.af.high = processor.de.low;
            break;
        case CGBL_INSTRUCTION_LD_A_FF00_CI:
            processor.delay += 4;
            processor.af.high = cgbl_bus_read(0xFF00 + processor.bc.low);
            break;
        case CGBL_INSTRUCTION_LD_A_FF00_NI:
            processor.delay += 8;
            processor.af.high = cgbl_bus_read(0xFF00 + cgbl_bus_read(processor.pc.word++));
            break;
        case CGBL_INSTRUCTION_LD_A_H:
            processor.af.high = processor.hl.high;
            break;
        case CGBL_INSTRUCTION_LD_A_HLI:
            processor.delay += 4;
            processor.af.high = cgbl_bus_read(processor.hl.word);
            break;
        case CGBL_INSTRUCTION_LD_A_HLID:
            processor.delay += 4;
            processor.af.high = cgbl_bus_read(processor.hl.word--);
            break;
        case CGBL_INSTRUCTION_LD_A_HLII:
            processor.delay += 4;
            processor.af.high = cgbl_bus_read(processor.hl.word++);
            break;
        case CGBL_INSTRUCTION_LD_A_L:
            processor.af.high = processor.hl.low;
            break;
        case CGBL_INSTRUCTION_LD_A_N:
            processor.delay += 4;
            processor.af.high = cgbl_bus_read(processor.pc.word++);
            break;
        case CGBL_INSTRUCTION_LD_A_NNI:
            processor.delay += 12;
            operand.low = cgbl_bus_read(processor.pc.word++);
            operand.high = cgbl_bus_read(processor.pc.word++);
            processor.af.high = cgbl_bus_read(operand.word);
            break;
        case CGBL_INSTRUCTION_LD_B_A:
            processor.bc.high = processor.af.high;
            break;
        case CGBL_INSTRUCTION_LD_B_B:
            break;
        case CGBL_INSTRUCTION_LD_B_C:
            processor.bc.high = processor.bc.low;
            break;
        case CGBL_INSTRUCTION_LD_B_D:
            processor.bc.high = processor.de.high;
            break;
        case CGBL_INSTRUCTION_LD_B_E:
            processor.bc.high = processor.de.low;
            break;
        case CGBL_INSTRUCTION_LD_B_H:
            processor.bc.high = processor.hl.high;
            break;
        case CGBL_INSTRUCTION_LD_B_HLI:
            processor.delay += 4;
            processor.bc.high = cgbl_bus_read(processor.hl.word);
            break;
        case CGBL_INSTRUCTION_LD_B_L:
            processor.bc.high = processor.hl.low;
            break;
        case CGBL_INSTRUCTION_LD_B_N:
            processor.delay += 4;
            processor.bc.high = cgbl_bus_read(processor.pc.word++);
            break;
        case CGBL_INSTRUCTION_LD_BC_NN:
            processor.delay += 8;
            processor.bc.low = cgbl_bus_read(processor.pc.word++);
            processor.bc.high = cgbl_bus_read(processor.pc.word++);
            break;
        case CGBL_INSTRUCTION_LD_BCI_A:
            processor.delay += 4;
            cgbl_bus_write(processor.bc.word, processor.af.high);
            break;
        case CGBL_INSTRUCTION_LD_C_A:
            processor.bc.low = processor.af.high;
            break;
        case CGBL_INSTRUCTION_LD_C_B:
            processor.bc.low = processor.bc.high;
            break;
        case CGBL_INSTRUCTION_LD_C_C:
            break;
        case CGBL_INSTRUCTION_LD_C_D:
            processor.bc.low = processor.de.high;
            break;
        case CGBL_INSTRUCTION_LD_C_E:
            processor.bc.low = processor.de.low;
            break;
        case CGBL_INSTRUCTION_LD_C_H:
            processor.bc.low = processor.hl.high;
            break;
        case CGBL_INSTRUCTION_LD_C_HLI:
            processor.delay += 4;
            processor.bc.low = cgbl_bus_read(processor.hl.word);
            break;
        case CGBL_INSTRUCTION_LD_C_L:
            processor.bc.low = processor.hl.low;
            break;
        case CGBL_INSTRUCTION_LD_C_N:
            processor.delay += 4;
            processor.bc.low = cgbl_bus_read(processor.pc.word++);
            break;
        case CGBL_INSTRUCTION_LD_DE_NN:
            processor.delay += 8;
            processor.de.low = cgbl_bus_read(processor.pc.word++);
            processor.de.high = cgbl_bus_read(processor.pc.word++);
            break;
        case CGBL_INSTRUCTION_LD_DEI_A:
            processor.delay += 4;
            cgbl_bus_write(processor.de.word, processor.af.high);
            break;
        case CGBL_INSTRUCTION_LD_D_A:
            processor.de.high = processor.af.high;
            break;
        case CGBL_INSTRUCTION_LD_D_B:
            processor.de.high = processor.bc.high;
            break;
        case CGBL_INSTRUCTION_LD_D_C:
            processor.de.high = processor.bc.low;
            break;
        case CGBL_INSTRUCTION_LD_D_D:
            break;
        case CGBL_INSTRUCTION_LD_D_E:
            processor.de.high = processor.de.low;
            break;
        case CGBL_INSTRUCTION_LD_D_H:
            processor.de.high = processor.hl.high;
            break;
        case CGBL_INSTRUCTION_LD_D_HLI:
            processor.delay += 4;
            processor.de.high = cgbl_bus_read(processor.hl.word);
            break;
        case CGBL_INSTRUCTION_LD_D_L:
            processor.de.high = processor.hl.low;
            break;
        case CGBL_INSTRUCTION_LD_D_N:
            processor.delay += 4;
            processor.de.high = cgbl_bus_read(processor.pc.word++);
            break;
        case CGBL_INSTRUCTION_LD_E_A:
            processor.de.low = processor.af.high;
            break;
        case CGBL_INSTRUCTION_LD_E_B:
            processor.de.low = processor.bc.high;
            break;
        case CGBL_INSTRUCTION_LD_E_C:
            processor.de.low = processor.bc.low;
            break;
        case CGBL_INSTRUCTION_LD_E_D:
            processor.de.low = processor.de.high;
            break;
        case CGBL_INSTRUCTION_LD_E_E:
            break;
        case CGBL_INSTRUCTION_LD_E_H:
            processor.de.low = processor.hl.high;
            break;
        case CGBL_INSTRUCTION_LD_E_HLI:
            processor.delay += 4;
            processor.de.low = cgbl_bus_read(processor.hl.word);
            break;
        case CGBL_INSTRUCTION_LD_E_L:
            processor.de.low = processor.hl.low;
            break;
        case CGBL_INSTRUCTION_LD_E_N:
            processor.delay += 4;
            processor.de.low = cgbl_bus_read(processor.pc.word++);
            break;
        case CGBL_INSTRUCTION_LD_FF00_CI_A:
            processor.delay += 4;
            cgbl_bus_write(0xFF00 + processor.bc.low, processor.af.high);
            break;
        case CGBL_INSTRUCTION_LD_FF00_NI_A:
            processor.delay += 8;
            cgbl_bus_write(0xFF00 + cgbl_bus_read(processor.pc.word++), processor.af.high);
            break;
        case CGBL_INSTRUCTION_LD_H_A:
            processor.hl.high = processor.af.high;
            break;
        case CGBL_INSTRUCTION_LD_H_B:
            processor.hl.high = processor.bc.high;
            break;
        case CGBL_INSTRUCTION_LD_H_C:
            processor.hl.high = processor.bc.low;
            break;
        case CGBL_INSTRUCTION_LD_H_D:
            processor.hl.high = processor.de.high;
            break;
        case CGBL_INSTRUCTION_LD_H_E:
            processor.hl.high = processor.de.low;
            break;
        case CGBL_INSTRUCTION_LD_H_H:
            break;
        case CGBL_INSTRUCTION_LD_H_HLI:
            processor.delay += 4;
            processor.hl.high = cgbl_bus_read(processor.hl.word);
            break;
        case CGBL_INSTRUCTION_LD_H_L:
            processor.hl.high = processor.hl.low;
            break;
        case CGBL_INSTRUCTION_LD_H_N:
            processor.delay += 4;
            processor.hl.high = cgbl_bus_read(processor.pc.word++);
            break;
        case CGBL_INSTRUCTION_LD_HL_NN:
            processor.delay += 8;
            processor.hl.low = cgbl_bus_read(processor.pc.word++);
            processor.hl.high = cgbl_bus_read(processor.pc.word++);
            break;
        case CGBL_INSTRUCTION_LD_HLI_A:
            processor.delay += 4;
            cgbl_bus_write(processor.hl.word, processor.af.high);
            break;
        case CGBL_INSTRUCTION_LD_HLI_B:
            processor.delay += 4;
            cgbl_bus_write(processor.hl.word, processor.bc.high);
            break;
        case CGBL_INSTRUCTION_LD_HLI_C:
            processor.delay += 4;
            cgbl_bus_write(processor.hl.word, processor.bc.low);
            break;
        case CGBL_INSTRUCTION_LD_HLI_D:
            processor.delay += 4;
            cgbl_bus_write(processor.hl.word, processor.de.high);
            break;
        case CGBL_INSTRUCTION_LD_HLI_E:
            processor.delay += 4;
            cgbl_bus_write(processor.hl.word, processor.de.low);
            break;
        case CGBL_INSTRUCTION_LD_HLI_H:
            processor.delay += 4;
            cgbl_bus_write(processor.hl.word, processor.hl.high);
            break;
        case CGBL_INSTRUCTION_LD_HLI_L:
            processor.delay += 4;
            cgbl_bus_write(processor.hl.word, processor.hl.low);
            break;
        case CGBL_INSTRUCTION_LD_HLI_N:
            processor.delay += 8;
            cgbl_bus_write(processor.hl.word, cgbl_bus_read(processor.pc.word++));
            break;
        case CGBL_INSTRUCTION_LD_HLID_A:
            processor.delay += 4;
            cgbl_bus_write(processor.hl.word--, processor.af.high);
            break;
        case CGBL_INSTRUCTION_LD_HLII_A:
            processor.delay += 4;
            cgbl_bus_write(processor.hl.word++, processor.af.high);
            break;
        case CGBL_INSTRUCTION_LD_L_A:
            processor.hl.low = processor.af.high;
            break;
        case CGBL_INSTRUCTION_LD_L_B:
            processor.hl.low = processor.bc.high;
            break;
        case CGBL_INSTRUCTION_LD_L_C:
            processor.hl.low = processor.bc.low;
            break;
        case CGBL_INSTRUCTION_LD_L_D:
            processor.hl.low = processor.de.high;
            break;
        case CGBL_INSTRUCTION_LD_L_E:
            processor.hl.low = processor.de.low;
            break;
        case CGBL_INSTRUCTION_LD_L_H:
            processor.hl.low = processor.hl.high;
            break;
        case CGBL_INSTRUCTION_LD_L_HLI:
            processor.delay += 4;
            processor.hl.low = cgbl_bus_read(processor.hl.word);
            break;
        case CGBL_INSTRUCTION_LD_L_L:
            break;
        case CGBL_INSTRUCTION_LD_L_N:
            processor.delay += 4;
            processor.hl.low = cgbl_bus_read(processor.pc.word++);
            break;
        case CGBL_INSTRUCTION_LD_NNI_A:
            processor.delay += 12;
            operand.low = cgbl_bus_read(processor.pc.word++);
            operand.high = cgbl_bus_read(processor.pc.word++);
            cgbl_bus_write(operand.word, processor.af.high);
            break;
        case CGBL_INSTRUCTION_LD_NNI_SP:
            processor.delay += 16;
            operand.low = cgbl_bus_read(processor.pc.word++);
            operand.high = cgbl_bus_read(processor.pc.word++);
            cgbl_bus_write(operand.word, processor.sp.low);
            cgbl_bus_write(operand.word + 1, processor.sp.high);
            break;
        case CGBL_INSTRUCTION_LD_SP_HL:
            processor.delay += 4;
            processor.sp.word = processor.hl.word;
            break;
        case CGBL_INSTRUCTION_LD_SP_NN:
            processor.delay += 8;
            processor.sp.low = cgbl_bus_read(processor.pc.word++);
            processor.sp.high = cgbl_bus_read(processor.pc.word++);
            break;
        default:
            break;
    }
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_ld_hl(void)
{
    uint32_t carry = 0, sum = 0;
    cgbl_register_t operand = { .low = cgbl_bus_read(processor.pc.word++) };
    processor.delay = 12;
    sum = processor.sp.word + (int8_t)operand.low;
    carry = processor.sp.word ^ (int8_t)operand.low ^ sum;
    processor.af.carry = ((carry & 0x100) == 0x100);
    processor.af.half_carry = ((carry & 0x10) == 0x10);
    processor.af.negative = false;
    processor.af.zero = false;
    processor.hl.word = sum;
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_nop(void)
{
    processor.delay = 4;
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_or(void)
{
    processor.delay = 4;
    switch (processor.instruction.opcode)
    {
        case CGBL_INSTRUCTION_OR_A:
            break;
        case CGBL_INSTRUCTION_OR_B:
            processor.af.high |= processor.bc.high;
            break;
        case CGBL_INSTRUCTION_OR_C:
            processor.af.high |= processor.bc.low;
            break;
        case CGBL_INSTRUCTION_OR_D:
            processor.af.high |= processor.de.high;
            break;
        case CGBL_INSTRUCTION_OR_E:
            processor.af.high |= processor.de.low;
            break;
        case CGBL_INSTRUCTION_OR_H:
            processor.af.high |= processor.hl.high;
            break;
        case CGBL_INSTRUCTION_OR_HLI:
            processor.delay += 4;
            processor.af.high |= cgbl_bus_read(processor.hl.word);
            break;
        case CGBL_INSTRUCTION_OR_L:
            processor.af.high |= processor.hl.low;
            break;
        case CGBL_INSTRUCTION_OR_N:
            processor.delay += 4;
            processor.af.high |= cgbl_bus_read(processor.pc.word++);
            break;
        default:
            break;
    }
    processor.af.carry = false;
    processor.af.half_carry = false;
    processor.af.negative = false;
    processor.af.zero = !processor.af.high;
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_pop(void)
{
    processor.delay = 12;
    switch (processor.instruction.opcode)
    {
        case CGBL_INSTRUCTION_POP_AF:
            processor.af.low = cgbl_bus_read(processor.sp.word++) & 0xF0;
            processor.af.high = cgbl_bus_read(processor.sp.word++);
            break;
        case CGBL_INSTRUCTION_POP_BC:
            processor.bc.low = cgbl_bus_read(processor.sp.word++);
            processor.bc.high = cgbl_bus_read(processor.sp.word++);
            break;
        case CGBL_INSTRUCTION_POP_DE:
            processor.de.low = cgbl_bus_read(processor.sp.word++);
            processor.de.high = cgbl_bus_read(processor.sp.word++);
            break;
        case CGBL_INSTRUCTION_POP_HL:
            processor.hl.low = cgbl_bus_read(processor.sp.word++);
            processor.hl.high = cgbl_bus_read(processor.sp.word++);
            break;
        default:
            break;
    }
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_push(void)
{
    processor.delay = 16;
    switch (processor.instruction.opcode)
    {
        case CGBL_INSTRUCTION_PUSH_AF:
            cgbl_bus_write(--processor.sp.word, processor.af.high);
            cgbl_bus_write(--processor.sp.word, processor.af.low);
            break;
        case CGBL_INSTRUCTION_PUSH_BC:
            cgbl_bus_write(--processor.sp.word, processor.bc.high);
            cgbl_bus_write(--processor.sp.word, processor.bc.low);
            break;
        case CGBL_INSTRUCTION_PUSH_DE:
            cgbl_bus_write(--processor.sp.word, processor.de.high);
            cgbl_bus_write(--processor.sp.word, processor.de.low);
            break;
        case CGBL_INSTRUCTION_PUSH_HL:
            cgbl_bus_write(--processor.sp.word, processor.hl.high);
            cgbl_bus_write(--processor.sp.word, processor.hl.low);
            break;
        default:
            break;
    }
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_res(void)
{
    processor.delay = 8;
    switch (processor.instruction.opcode)
    {
        case CGBL_INSTRUCTION_RES_0_A: case CGBL_INSTRUCTION_RES_1_A: case CGBL_INSTRUCTION_RES_2_A: case CGBL_INSTRUCTION_RES_3_A:
        case CGBL_INSTRUCTION_RES_4_A: case CGBL_INSTRUCTION_RES_5_A: case CGBL_INSTRUCTION_RES_6_A: case CGBL_INSTRUCTION_RES_7_A:
            processor.af.high &= ~(1 << ((processor.instruction.opcode - CGBL_INSTRUCTION_RES_0_A) / 8));
            break;
        case CGBL_INSTRUCTION_RES_0_B: case CGBL_INSTRUCTION_RES_1_B: case CGBL_INSTRUCTION_RES_2_B: case CGBL_INSTRUCTION_RES_3_B:
        case CGBL_INSTRUCTION_RES_4_B: case CGBL_INSTRUCTION_RES_5_B: case CGBL_INSTRUCTION_RES_6_B: case CGBL_INSTRUCTION_RES_7_B:
            processor.bc.high &= ~(1 << ((processor.instruction.opcode - CGBL_INSTRUCTION_RES_0_B) / 8));
            break;
        case CGBL_INSTRUCTION_RES_0_C: case CGBL_INSTRUCTION_RES_1_C: case CGBL_INSTRUCTION_RES_2_C: case CGBL_INSTRUCTION_RES_3_C:
        case CGBL_INSTRUCTION_RES_4_C: case CGBL_INSTRUCTION_RES_5_C: case CGBL_INSTRUCTION_RES_6_C: case CGBL_INSTRUCTION_RES_7_C:
            processor.bc.low &= ~(1 << ((processor.instruction.opcode - CGBL_INSTRUCTION_RES_0_C) / 8));
            break;
        case CGBL_INSTRUCTION_RES_0_D: case CGBL_INSTRUCTION_RES_1_D: case CGBL_INSTRUCTION_RES_2_D: case CGBL_INSTRUCTION_RES_3_D:
        case CGBL_INSTRUCTION_RES_4_D: case CGBL_INSTRUCTION_RES_5_D: case CGBL_INSTRUCTION_RES_6_D: case CGBL_INSTRUCTION_RES_7_D:
            processor.de.high &= ~(1 << ((processor.instruction.opcode - CGBL_INSTRUCTION_RES_0_D) / 8));
            break;
        case CGBL_INSTRUCTION_RES_0_E: case CGBL_INSTRUCTION_RES_1_E: case CGBL_INSTRUCTION_RES_2_E: case CGBL_INSTRUCTION_RES_3_E:
        case CGBL_INSTRUCTION_RES_4_E: case CGBL_INSTRUCTION_RES_5_E: case CGBL_INSTRUCTION_RES_6_E: case CGBL_INSTRUCTION_RES_7_E:
            processor.de.low &= ~(1 << ((processor.instruction.opcode - CGBL_INSTRUCTION_RES_0_E) / 8));
            break;
        case CGBL_INSTRUCTION_RES_0_H: case CGBL_INSTRUCTION_RES_1_H: case CGBL_INSTRUCTION_RES_2_H: case CGBL_INSTRUCTION_RES_3_H:
        case CGBL_INSTRUCTION_RES_4_H: case CGBL_INSTRUCTION_RES_5_H: case CGBL_INSTRUCTION_RES_6_H: case CGBL_INSTRUCTION_RES_7_H:
            processor.hl.high &= ~(1 << ((processor.instruction.opcode - CGBL_INSTRUCTION_RES_0_H) / 8));
            break;
        case CGBL_INSTRUCTION_RES_0_HLI: case CGBL_INSTRUCTION_RES_1_HLI: case CGBL_INSTRUCTION_RES_2_HLI: case CGBL_INSTRUCTION_RES_3_HLI:
        case CGBL_INSTRUCTION_RES_4_HLI: case CGBL_INSTRUCTION_RES_5_HLI: case CGBL_INSTRUCTION_RES_6_HLI: case CGBL_INSTRUCTION_RES_7_HLI:
            processor.delay += 8;
            cgbl_bus_write(processor.hl.word, cgbl_bus_read(processor.hl.word) & ~(1 << ((processor.instruction.opcode - CGBL_INSTRUCTION_RES_0_HLI) / 8)));
            break;
        case CGBL_INSTRUCTION_RES_0_L: case CGBL_INSTRUCTION_RES_1_L: case CGBL_INSTRUCTION_RES_2_L: case CGBL_INSTRUCTION_RES_3_L:
        case CGBL_INSTRUCTION_RES_4_L: case CGBL_INSTRUCTION_RES_5_L: case CGBL_INSTRUCTION_RES_6_L: case CGBL_INSTRUCTION_RES_7_L:
            processor.hl.low &= ~(1 << ((processor.instruction.opcode - CGBL_INSTRUCTION_RES_0_L) / 8));
            break;
        default:
            break;
    }
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_ret(void)
{
    bool taken = false;
    processor.delay = 8;
    switch (processor.instruction.opcode)
    {
        case CGBL_INSTRUCTION_RET:
            taken = true;
            break;
        case CGBL_INSTRUCTION_RET_C:
            taken = processor.af.carry;
            break;
        case CGBL_INSTRUCTION_RET_NC:
            taken = !processor.af.carry;
            break;
        case CGBL_INSTRUCTION_RET_NZ:
            taken = !processor.af.zero;
            break;
        case CGBL_INSTRUCTION_RET_Z:
            taken = processor.af.zero;
            break;
        default:
            break;
    }
    if (taken)
    {
        processor.delay += (processor.instruction.opcode == CGBL_INSTRUCTION_RET) ? 8 : 12;
        processor.pc.low = cgbl_bus_read(processor.sp.word++);
        processor.pc.high = cgbl_bus_read(processor.sp.word++);
    }
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_reti(void)
{
    processor.delay = 16;
    processor.interrupt.delay = 0;
    processor.interrupt.enabled = true;
    processor.pc.low = cgbl_bus_read(processor.sp.word++);
    processor.pc.high = cgbl_bus_read(processor.sp.word++);
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_rl(void)
{
    bool carry = processor.af.carry;
    cgbl_register_t operand = {};
    processor.delay = 8;
    switch (processor.instruction.opcode)
    {
        case CGBL_INSTRUCTION_RL_A:
            processor.af.carry = ((processor.af.high & 0x80) == 0x80);
            processor.af.high = (processor.af.high << 1) | (carry ? 1 : 0);
            processor.af.zero = !processor.af.high;
            break;
        case CGBL_INSTRUCTION_RL_B:
            processor.af.carry = ((processor.bc.high & 0x80) == 0x80);
            processor.bc.high = (processor.bc.high << 1) | (carry ? 1 : 0);
            processor.af.zero = !processor.bc.high;
            break;
        case CGBL_INSTRUCTION_RL_C:
            processor.af.carry = ((processor.bc.low & 0x80) == 0x80);
            processor.bc.low = (processor.bc.low << 1) | (carry ? 1 : 0);
            processor.af.zero = !processor.bc.low;
            break;
        case CGBL_INSTRUCTION_RL_D:
            processor.af.carry = ((processor.de.high & 0x80) == 0x80);
            processor.de.high = (processor.de.high << 1) | (carry ? 1 : 0);
            processor.af.zero = !processor.de.high;
            break;
        case CGBL_INSTRUCTION_RL_E:
            processor.af.carry = ((processor.de.low & 0x80) == 0x80);
            processor.de.low = (processor.de.low << 1) | (carry ? 1 : 0);
            processor.af.zero = !processor.de.low;
            break;
        case CGBL_INSTRUCTION_RL_H:
            processor.af.carry = ((processor.hl.high & 0x80) == 0x80);
            processor.hl.high = (processor.hl.high << 1) | (carry ? 1 : 0);
            processor.af.zero = !processor.hl.high;
            break;
        case CGBL_INSTRUCTION_RL_HLI:
            processor.delay += 8;
            operand.low = cgbl_bus_read(processor.hl.word);
            processor.af.carry = ((operand.low & 0x80) == 0x80);
            operand.low = (operand.low << 1) | (carry ? 1 : 0);
            processor.af.zero = !operand.low;
            cgbl_bus_write(processor.hl.word, operand.low);
            break;
        case CGBL_INSTRUCTION_RL_L:
            processor.af.carry = ((processor.hl.low & 0x80) == 0x80);
            processor.hl.low = (processor.hl.low << 1) | (carry ? 1 : 0);
            processor.af.zero = !processor.hl.low;
            break;
        default:
            break;
    }
    processor.af.half_carry = false;
    processor.af.negative = false;
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_rla(void)
{
    bool carry = processor.af.carry;
    processor.delay = 4;
    processor.af.carry = ((processor.af.high & 0x80) == 0x80);
    processor.af.high = (processor.af.high << 1) | (carry ? 1 : 0);
    processor.af.half_carry = false;
    processor.af.negative = false;
    processor.af.zero = false;
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_rlc(void)
{
    cgbl_register_t operand = {};
    processor.delay = 8;
    switch (processor.instruction.opcode)
    {
        case CGBL_INSTRUCTION_RLC_A:
            processor.af.carry = ((processor.af.high & 0x80) == 0x80);
            processor.af.high = (processor.af.high << 1) | (processor.af.carry ? 1 : 0);
            processor.af.zero = !processor.af.high;
            break;
        case CGBL_INSTRUCTION_RLC_B:
            processor.af.carry = ((processor.bc.high & 0x80) == 0x80);
            processor.bc.high = (processor.bc.high << 1) | (processor.af.carry ? 1 : 0);
            processor.af.zero = !processor.bc.high;
            break;
        case CGBL_INSTRUCTION_RLC_C:
            processor.af.carry = ((processor.bc.low & 0x80) == 0x80);
            processor.bc.low = (processor.bc.low << 1) | (processor.af.carry ? 1 : 0);
            processor.af.zero = !processor.bc.low;
            break;
        case CGBL_INSTRUCTION_RLC_D:
            processor.af.carry = ((processor.de.high & 0x80) == 0x80);
            processor.de.high = (processor.de.high << 1) | (processor.af.carry ? 1 : 0);
            processor.af.zero = !processor.de.high;
            break;
        case CGBL_INSTRUCTION_RLC_E:
            processor.af.carry = ((processor.de.low & 0x80) == 0x80);
            processor.de.low = (processor.de.low << 1) | (processor.af.carry ? 1 : 0);
            processor.af.zero = !processor.de.low;
            break;
        case CGBL_INSTRUCTION_RLC_H:
            processor.af.carry = ((processor.hl.high & 0x80) == 0x80);
            processor.hl.high = (processor.hl.high << 1) | (processor.af.carry ? 1 : 0);
            processor.af.zero = !processor.hl.high;
            break;
        case CGBL_INSTRUCTION_RLC_HLI:
            processor.delay += 8;
            operand.low = cgbl_bus_read(processor.hl.word);
            processor.af.carry = ((operand.low & 0x80) == 0x80);
            operand.low = (operand.low << 1) | (processor.af.carry ? 1 : 0);
            processor.af.zero = !operand.low;
            cgbl_bus_write(processor.hl.word, operand.low);
            break;
        case CGBL_INSTRUCTION_RLC_L:
            processor.af.carry = ((processor.hl.low & 0x80) == 0x80);
            processor.hl.low = (processor.hl.low << 1) | (processor.af.carry ? 1 : 0);
            processor.af.zero = !processor.hl.low;
            break;
        default:
            break;
    }
    processor.af.half_carry = false;
    processor.af.negative = false;
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_rlca(void)
{
    processor.delay = 4;
    processor.af.carry = ((processor.af.high & 0x80) == 0x80);
    processor.af.high = (processor.af.high << 1) | (processor.af.carry ? 1 : 0);
    processor.af.half_carry = false;
    processor.af.negative = false;
    processor.af.zero = false;
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_rr(void)
{
    bool carry = processor.af.carry;
    cgbl_register_t operand = {};
    processor.delay = 8;
    switch (processor.instruction.opcode)
    {
        case CGBL_INSTRUCTION_RR_A:
            processor.af.carry = ((processor.af.high & 1) == 1);
            processor.af.high = (processor.af.high >> 1) | (carry ? 0x80 : 0);
            processor.af.zero = !processor.af.high;
            break;
        case CGBL_INSTRUCTION_RR_B:
            processor.af.carry = ((processor.bc.high & 1) == 1);
            processor.bc.high = (processor.bc.high >> 1) | (carry ? 0x80 : 0);
            processor.af.zero = !processor.bc.high;
            break;
        case CGBL_INSTRUCTION_RR_C:
            processor.af.carry = ((processor.bc.low & 1) == 1);
            processor.bc.low = (processor.bc.low >> 1) | (carry ? 0x80 : 0);
            processor.af.zero = !processor.bc.low;
            break;
        case CGBL_INSTRUCTION_RR_D:
            processor.af.carry = ((processor.de.high & 1) == 1);
            processor.de.high = (processor.de.high >>  1) | (carry ? 0x80 : 0);
            processor.af.zero = !processor.de.high;
            break;
        case CGBL_INSTRUCTION_RR_E:
            processor.af.carry = ((processor.de.low & 1) == 1);
            processor.de.low = (processor.de.low >>  1) | (carry ? 0x80 : 0);
            processor.af.zero = !processor.de.low;
            break;
        case CGBL_INSTRUCTION_RR_H:
            processor.af.carry = ((processor.hl.high & 1) == 1);
            processor.hl.high = (processor.hl.high >>  1) | (carry ? 0x80 : 0);
            processor.af.zero = !processor.hl.high;
            break;
        case CGBL_INSTRUCTION_RR_HLI:
            processor.delay += 8;
            operand.low = cgbl_bus_read(processor.hl.word);
            processor.af.carry = ((operand.low & 1) == 1);
            operand.low = (operand.low >>  1) | (carry ? 0x80 : 0);
            processor.af.zero = !operand.low;
            cgbl_bus_write(processor.hl.word, operand.low);
            break;
        case CGBL_INSTRUCTION_RR_L:
            processor.af.carry = ((processor.hl.low & 1) == 1);
            processor.hl.low = (processor.hl.low >> 1) | (carry ? 0x80 : 0);
            processor.af.zero = !processor.hl.low;
            break;
        default:
            break;
    }
    processor.af.half_carry = false;
    processor.af.negative = false;
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_rra(void)
{
    bool carry = processor.af.carry;
    processor.delay = 4;
    processor.af.carry = ((processor.af.high & 1) == 1);
    processor.af.high = (processor.af.high >> 1) | (carry ? 0x80 : 0);
    processor.af.half_carry = false;
    processor.af.negative = false;
    processor.af.zero = false;
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_rrc(void)
{
    cgbl_register_t operand = {};
    processor.delay = 8;
    switch (processor.instruction.opcode)
    {
        case CGBL_INSTRUCTION_RRC_A:
            processor.af.carry = ((processor.af.high & 1) == 1);
            processor.af.high = (processor.af.high >> 1) | (processor.af.carry ? 0x80 : 0);
            processor.af.zero = !processor.af.high;
            break;
        case CGBL_INSTRUCTION_RRC_B:
            processor.af.carry = ((processor.bc.high & 1) == 1);
            processor.bc.high = (processor.bc.high >> 1) | (processor.af.carry ? 0x80 : 0);
            processor.af.zero = !processor.bc.high;
            break;
        case CGBL_INSTRUCTION_RRC_C:
            processor.af.carry = ((processor.bc.low & 1) == 1);
            processor.bc.low = (processor.bc.low >> 1) | (processor.af.carry ? 0x80 : 0);
            processor.af.zero = !processor.bc.low;
            break;
        case CGBL_INSTRUCTION_RRC_D:
            processor.af.carry = ((processor.de.high & 1) == 1);
            processor.de.high = (processor.de.high >> 1) | (processor.af.carry ? 0x80 : 0);
            processor.af.zero = !processor.de.high;
            break;
        case CGBL_INSTRUCTION_RRC_E:
            processor.af.carry = ((processor.de.low & 1) == 1);
            processor.de.low = (processor.de.low >> 1) | (processor.af.carry ? 0x80 : 0);
            processor.af.zero = !processor.de.low;
            break;
        case CGBL_INSTRUCTION_RRC_H:
            processor.af.carry = ((processor.hl.high & 1) == 1);
            processor.hl.high = (processor.hl.high >> 1) | (processor.af.carry ? 0x80 : 0);
            processor.af.zero = !processor.hl.high;
            break;
        case CGBL_INSTRUCTION_RRC_HLI:
            processor.delay += 8;
            operand.low = cgbl_bus_read(processor.hl.word);
            processor.af.carry = ((operand.low & 1) == 1);
            operand.low = (operand.low >> 1) | (processor.af.carry ? 0x80 : 0);
            processor.af.zero = !operand.low;
            cgbl_bus_write(processor.hl.word, operand.low);
            break;
        case CGBL_INSTRUCTION_RRC_L:
            processor.af.carry = ((processor.hl.low & 1) == 1);
            processor.hl.low = (processor.hl.low >> 1) | (processor.af.carry ? 0x80 : 0);
            processor.af.zero = !processor.hl.low;
            break;
        default:
            break;
    }
    processor.af.half_carry = false;
    processor.af.negative = false;
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_rrca(void)
{
    processor.delay = 4;
    processor.af.carry = ((processor.af.high & 1) == 1);
    processor.af.high = (processor.af.high >> 1) | (processor.af.carry ? 0x80 : 0);
    processor.af.half_carry = false;
    processor.af.negative = false;
    processor.af.zero = false;
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_rst(void)
{
    processor.delay = 16;
    cgbl_bus_write(--processor.sp.word, processor.pc.high);
    cgbl_bus_write(--processor.sp.word, processor.pc.low);
    processor.pc.word = processor.instruction.opcode - CGBL_INSTRUCTION_RST_00;
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_sbc(void)
{
    uint16_t carry = 0, sum = 0;
    cgbl_register_t operand = {};
    processor.delay = 4;
    switch (processor.instruction.opcode)
    {
        case CGBL_INSTRUCTION_SBC_A:
            operand.low = processor.af.high;
            break;
        case CGBL_INSTRUCTION_SBC_B:
            operand.low = processor.bc.high;
            break;
        case CGBL_INSTRUCTION_SBC_C:
            operand.low = processor.bc.low;
            break;
        case CGBL_INSTRUCTION_SBC_D:
            operand.low = processor.de.high;
            break;
        case CGBL_INSTRUCTION_SBC_E:
            operand.low = processor.de.low;
            break;
        case CGBL_INSTRUCTION_SBC_H:
            operand.low = processor.hl.high;
            break;
        case CGBL_INSTRUCTION_SBC_HLI:
            processor.delay += 4;
            operand.low = cgbl_bus_read(processor.hl.word);
            break;
        case CGBL_INSTRUCTION_SBC_L:
            operand.low = processor.hl.low;
            break;
        case CGBL_INSTRUCTION_SBC_N:
            processor.delay += 4;
            operand.low = cgbl_bus_read(processor.pc.word++);
            break;
        default:
            break;
    }
    sum = processor.af.high - operand.low - processor.af.carry;
    carry = processor.af.high ^ operand.low ^ processor.af.carry ^ sum;
    processor.af.carry = ((carry & 0x100) == 0x100);
    processor.af.half_carry = ((carry & 0x10) == 0x10);
    processor.af.negative = true;
    processor.af.zero = !(sum & 0xFF);
    processor.af.high = sum;
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_scf(void)
{
    processor.delay = 4;
    processor.af.carry = true;
    processor.af.half_carry = false;
    processor.af.negative = false;
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_set(void)
{
    processor.delay = 8;
    switch (processor.instruction.opcode)
    {
        case CGBL_INSTRUCTION_SET_0_A: case CGBL_INSTRUCTION_SET_1_A: case CGBL_INSTRUCTION_SET_2_A: case CGBL_INSTRUCTION_SET_3_A:
        case CGBL_INSTRUCTION_SET_4_A: case CGBL_INSTRUCTION_SET_5_A: case CGBL_INSTRUCTION_SET_6_A: case CGBL_INSTRUCTION_SET_7_A:
            processor.af.high |= (1 << ((processor.instruction.opcode - CGBL_INSTRUCTION_SET_0_A) / 8));
            break;
        case CGBL_INSTRUCTION_SET_0_B: case CGBL_INSTRUCTION_SET_1_B: case CGBL_INSTRUCTION_SET_2_B: case CGBL_INSTRUCTION_SET_3_B:
        case CGBL_INSTRUCTION_SET_4_B: case CGBL_INSTRUCTION_SET_5_B: case CGBL_INSTRUCTION_SET_6_B: case CGBL_INSTRUCTION_SET_7_B:
            processor.bc.high |= (1 << ((processor.instruction.opcode - CGBL_INSTRUCTION_SET_0_B) / 8));
            break;
        case CGBL_INSTRUCTION_SET_0_C: case CGBL_INSTRUCTION_SET_1_C: case CGBL_INSTRUCTION_SET_2_C: case CGBL_INSTRUCTION_SET_3_C:
        case CGBL_INSTRUCTION_SET_4_C: case CGBL_INSTRUCTION_SET_5_C: case CGBL_INSTRUCTION_SET_6_C: case CGBL_INSTRUCTION_SET_7_C:
            processor.bc.low |= (1 << ((processor.instruction.opcode - CGBL_INSTRUCTION_SET_0_C) / 8));
            break;
        case CGBL_INSTRUCTION_SET_0_D: case CGBL_INSTRUCTION_SET_1_D: case CGBL_INSTRUCTION_SET_2_D: case CGBL_INSTRUCTION_SET_3_D:
        case CGBL_INSTRUCTION_SET_4_D: case CGBL_INSTRUCTION_SET_5_D: case CGBL_INSTRUCTION_SET_6_D: case CGBL_INSTRUCTION_SET_7_D:
            processor.de.high |= (1 << ((processor.instruction.opcode - CGBL_INSTRUCTION_SET_0_D) / 8));
            break;
        case CGBL_INSTRUCTION_SET_0_E: case CGBL_INSTRUCTION_SET_1_E: case CGBL_INSTRUCTION_SET_2_E: case CGBL_INSTRUCTION_SET_3_E:
        case CGBL_INSTRUCTION_SET_4_E: case CGBL_INSTRUCTION_SET_5_E: case CGBL_INSTRUCTION_SET_6_E: case CGBL_INSTRUCTION_SET_7_E:
            processor.de.low |= (1 << ((processor.instruction.opcode - CGBL_INSTRUCTION_SET_0_E) / 8));
            break;
        case CGBL_INSTRUCTION_SET_0_H: case CGBL_INSTRUCTION_SET_1_H: case CGBL_INSTRUCTION_SET_2_H: case CGBL_INSTRUCTION_SET_3_H:
        case CGBL_INSTRUCTION_SET_4_H: case CGBL_INSTRUCTION_SET_5_H: case CGBL_INSTRUCTION_SET_6_H: case CGBL_INSTRUCTION_SET_7_H:
            processor.hl.high |= (1 << ((processor.instruction.opcode - CGBL_INSTRUCTION_SET_0_H) / 8));
            break;
        case CGBL_INSTRUCTION_SET_0_HLI: case CGBL_INSTRUCTION_SET_1_HLI: case CGBL_INSTRUCTION_SET_2_HLI: case CGBL_INSTRUCTION_SET_3_HLI:
        case CGBL_INSTRUCTION_SET_4_HLI: case CGBL_INSTRUCTION_SET_5_HLI: case CGBL_INSTRUCTION_SET_6_HLI: case CGBL_INSTRUCTION_SET_7_HLI:
            processor.delay += 8;
            cgbl_bus_write(processor.hl.word, cgbl_bus_read(processor.hl.word) | (1 << ((processor.instruction.opcode - CGBL_INSTRUCTION_SET_0_HLI) / 8)));
            break;
        case CGBL_INSTRUCTION_SET_0_L: case CGBL_INSTRUCTION_SET_1_L: case CGBL_INSTRUCTION_SET_2_L: case CGBL_INSTRUCTION_SET_3_L:
        case CGBL_INSTRUCTION_SET_4_L: case CGBL_INSTRUCTION_SET_5_L: case CGBL_INSTRUCTION_SET_6_L: case CGBL_INSTRUCTION_SET_7_L:
            processor.hl.low |= (1 << ((processor.instruction.opcode - CGBL_INSTRUCTION_SET_0_L) / 8));
            break;
        default:
            break;
    }
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_sla(void)
{
    cgbl_register_t operand = {};
    processor.delay = 8;
    switch (processor.instruction.opcode)
    {
        case CGBL_INSTRUCTION_SLA_A:
            processor.af.carry = ((processor.af.high & 0x80) == 0x80);
            processor.af.high <<= 1;
            processor.af.zero = !processor.af.high;
            break;
        case CGBL_INSTRUCTION_SLA_B:
            processor.af.carry = ((processor.bc.high & 0x80) == 0x80);
            processor.bc.high <<= 1;
            processor.af.zero = !processor.bc.high;
            break;
        case CGBL_INSTRUCTION_SLA_C:
            processor.af.carry = ((processor.bc.low & 0x80) == 0x80);
            processor.bc.low <<= 1;
            processor.af.zero = !processor.bc.low;
            break;
        case CGBL_INSTRUCTION_SLA_D:
            processor.af.carry = ((processor.de.high & 0x80) == 0x80);
            processor.de.high <<= 1;
            processor.af.zero = !processor.de.high;
            break;
        case CGBL_INSTRUCTION_SLA_E:
            processor.af.carry = ((processor.de.low & 0x80) == 0x80);
            processor.de.low <<= 1;
            processor.af.zero = !processor.de.low;
            break;
        case CGBL_INSTRUCTION_SLA_H:
            processor.af.carry = ((processor.hl.high & 0x80) == 0x80);
            processor.hl.high <<= 1;
            processor.af.zero = !processor.hl.high;
            break;
        case CGBL_INSTRUCTION_SLA_HLI:
            processor.delay += 8;
            operand.low = cgbl_bus_read(processor.hl.word);
            processor.af.carry = ((operand.low & 0x80) == 0x80);
            operand.low <<= 1;
            processor.af.zero = !operand.low;
            cgbl_bus_write(processor.hl.word, operand.low);
            break;
        case CGBL_INSTRUCTION_SLA_L:
            processor.af.carry = ((processor.hl.low & 0x80) == 0x80);
            processor.hl.low <<= 1;
            processor.af.zero = !processor.hl.low;
            break;
        default:
            break;
    }
    processor.af.half_carry = false;
    processor.af.negative = false;
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_sra(void)
{
    cgbl_register_t operand = {};
    processor.delay = 8;
    switch (processor.instruction.opcode)
    {
        case CGBL_INSTRUCTION_SRA_A:
            processor.af.carry = ((processor.af.high & 1) == 1);
            processor.af.high = (processor.af.high >> 1) | (processor.af.high & 0x80);
            processor.af.zero = !processor.af.high;
            break;
        case CGBL_INSTRUCTION_SRA_B:
            processor.af.carry = ((processor.bc.high & 1) == 1);
            processor.bc.high = (processor.bc.high >> 1) | (processor.bc.high & 0x80);
            processor.af.zero = !processor.bc.high;
            break;
        case CGBL_INSTRUCTION_SRA_C:
            processor.af.carry = ((processor.bc.low & 1) == 1);
            processor.bc.low = (processor.bc.low >> 1) | (processor.bc.low & 0x80);
            processor.af.zero = !processor.bc.low;
            break;
        case CGBL_INSTRUCTION_SRA_D:
            processor.af.carry = ((processor.de.high & 1) == 1);
            processor.de.high = (processor.de.high >> 1) | (processor.de.high & 0x80);
            processor.af.zero = !processor.de.high;
            break;
        case CGBL_INSTRUCTION_SRA_E:
            processor.af.carry = ((processor.de.low & 1) == 1);
            processor.de.low = (processor.de.low >> 1) | (processor.de.low & 0x80);
            processor.af.zero = !processor.de.low;
            break;
        case CGBL_INSTRUCTION_SRA_H:
            processor.af.carry = ((processor.hl.high & 1) == 1);
            processor.hl.high = (processor.hl.high >> 1) | (processor.hl.high & 0x80);
            processor.af.zero = !processor.hl.high;
            break;
        case CGBL_INSTRUCTION_SRA_HLI:
            processor.delay += 8;
            operand.low = cgbl_bus_read(processor.hl.word);
            processor.af.carry = ((operand.low & 1) == 1);
            operand.low = (operand.low >> 1) | (operand.low & 0x80);
            processor.af.zero = !operand.low;
            cgbl_bus_write(processor.hl.word, operand.low);
            break;
        case CGBL_INSTRUCTION_SRA_L:
            processor.af.carry = ((processor.hl.low & 1) == 1);
            processor.hl.low = (processor.hl.low >> 1) | (processor.hl.low & 0x80);
            processor.af.zero = !processor.hl.low;
            break;
        default:
            break;
    }
    processor.af.half_carry = false;
    processor.af.negative = false;
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_srl(void)
{
    cgbl_register_t operand = {};
    processor.delay = 8;
    switch (processor.instruction.opcode)
    {
        case CGBL_INSTRUCTION_SRL_A:
            processor.af.carry = ((processor.af.high & 1) == 1);
            processor.af.high >>= 1;
            processor.af.zero = !processor.af.high;
            break;
        case CGBL_INSTRUCTION_SRL_B:
            processor.af.carry = ((processor.bc.high & 1) == 1);
            processor.bc.high >>= 1;
            processor.af.zero = !processor.bc.high;
            break;
        case CGBL_INSTRUCTION_SRL_C:
            processor.af.carry = ((processor.bc.low & 1) == 1);
            processor.bc.low >>= 1;
            processor.af.zero = !processor.bc.low;
            break;
        case CGBL_INSTRUCTION_SRL_D:
            processor.af.carry = ((processor.de.high & 1) == 1);
            processor.de.high >>= 1;
            processor.af.zero = !processor.de.high;
            break;
        case CGBL_INSTRUCTION_SRL_E:
            processor.af.carry = ((processor.de.low & 1) == 1);
            processor.de.low >>= 1;
            processor.af.zero = !processor.de.low;
            break;
        case CGBL_INSTRUCTION_SRL_H:
            processor.af.carry = ((processor.hl.high & 1) == 1);
            processor.hl.high >>= 1;
            processor.af.zero = !processor.hl.high;
            break;
        case CGBL_INSTRUCTION_SRL_HLI:
            processor.delay += 8;
            operand.low = cgbl_bus_read(processor.hl.word);
            processor.af.carry = ((operand.low & 1) == 1);
            operand.low >>= 1;
            processor.af.zero = !operand.low;
            cgbl_bus_write(processor.hl.word, operand.low);
            break;
        case CGBL_INSTRUCTION_SRL_L:
            processor.af.carry = ((processor.hl.low & 1) == 1);
            processor.hl.low >>= 1;
            processor.af.zero = !processor.hl.low;
            break;
        default:
            break;
    }
    processor.af.half_carry = false;
    processor.af.negative = false;
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_stop(void)
{
    processor.delay = 4;
    if (!cgbl_bus_speed_change())
    {
        processor.stopped = true;
        cgbl_bus_write(CGBL_TIMER_DIVIDER, 0);
    }
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_sub(void)
{
    uint16_t carry = 0, sum = 0;
    cgbl_register_t operand = {};
    processor.delay = 4;
    switch (processor.instruction.opcode)
    {
        case CGBL_INSTRUCTION_SUB_A:
            operand.low = processor.af.high;
            break;
        case CGBL_INSTRUCTION_SUB_B:
            operand.low = processor.bc.high;
            break;
        case CGBL_INSTRUCTION_SUB_C:
            operand.low = processor.bc.low;
            break;
        case CGBL_INSTRUCTION_SUB_D:
            operand.low = processor.de.high;
            break;
        case CGBL_INSTRUCTION_SUB_E:
            operand.low = processor.de.low;
            break;
        case CGBL_INSTRUCTION_SUB_H:
            operand.low = processor.hl.high;
            break;
        case CGBL_INSTRUCTION_SUB_HLI:
            processor.delay += 4;
            operand.low = cgbl_bus_read(processor.hl.word);
            break;
        case CGBL_INSTRUCTION_SUB_L:
            operand.low = processor.hl.low;
            break;
        case CGBL_INSTRUCTION_SUB_N:
            processor.delay += 4;
            operand.low = cgbl_bus_read(processor.pc.word++);
            break;
        default:
            break;
    }
    sum = processor.af.high - operand.low;
    carry = processor.af.high ^ operand.low ^ sum;
    processor.af.carry = ((carry & 0x100) == 0x100);
    processor.af.half_carry = ((carry & 0x10) == 0x10);
    processor.af.negative = true;
    processor.af.zero = !(sum & 0xFF);
    processor.af.high = sum;
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_swap(void)
{
    cgbl_register_t operand = {};
    processor.delay = 8;
    switch (processor.instruction.opcode)
    {
        case CGBL_INSTRUCTION_SWAP_A:
            processor.af.high = (processor.af.high << 4) | (processor.af.high >> 4);
            processor.af.zero = !processor.af.high;
            break;
        case CGBL_INSTRUCTION_SWAP_B:
            processor.bc.high = (processor.bc.high << 4) | (processor.bc.high >> 4);
            processor.af.zero = !processor.bc.high;
            break;
        case CGBL_INSTRUCTION_SWAP_C:
            processor.bc.low = (processor.bc.low << 4) | (processor.bc.low >> 4);
            processor.af.zero = !processor.bc.low;
            break;
        case CGBL_INSTRUCTION_SWAP_D:
            processor.de.high = (processor.de.high << 4) | (processor.de.high >> 4);
            processor.af.zero = !processor.de.high;
            break;
        case CGBL_INSTRUCTION_SWAP_E:
            processor.de.low = (processor.de.low << 4) | (processor.de.low >> 4);
            processor.af.zero = !processor.de.low;
            break;
        case CGBL_INSTRUCTION_SWAP_H:
            processor.hl.high = (processor.hl.high << 4) | (processor.hl.high >> 4);
            processor.af.zero = !processor.hl.high;
            break;
        case CGBL_INSTRUCTION_SWAP_HLI:
            processor.delay += 8;
            operand.low = cgbl_bus_read(processor.hl.word);
            operand.low = (operand.low << 4) | (operand.low >> 4);
            cgbl_bus_write(processor.hl.word, operand.low);
            processor.af.zero = !operand.low;
            break;
        case CGBL_INSTRUCTION_SWAP_L:
            processor.hl.low = (processor.hl.low << 4) | (processor.hl.low >> 4);
            processor.af.zero = !processor.hl.low;
            break;
        default:
            break;
    }
    processor.af.carry = false;
    processor.af.half_carry = false;
    processor.af.negative = false;
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_xor(void)
{
    processor.delay = 4;
    switch (processor.instruction.opcode)
    {
        case CGBL_INSTRUCTION_XOR_A:
            processor.af.high = 0;
            break;
        case CGBL_INSTRUCTION_XOR_B:
            processor.af.high ^= processor.bc.high;
            break;
        case CGBL_INSTRUCTION_XOR_C:
            processor.af.high ^= processor.bc.low;
            break;
        case CGBL_INSTRUCTION_XOR_D:
            processor.af.high ^= processor.de.high;
            break;
        case CGBL_INSTRUCTION_XOR_E:
            processor.af.high ^= processor.de.low;
            break;
        case CGBL_INSTRUCTION_XOR_H:
            processor.af.high ^= processor.hl.high;
            break;
        case CGBL_INSTRUCTION_XOR_HLI:
            processor.delay += 4;
            processor.af.high ^= cgbl_bus_read(processor.hl.word);
            break;
        case CGBL_INSTRUCTION_XOR_L:
            processor.af.high ^= processor.hl.low;
            break;
        case CGBL_INSTRUCTION_XOR_N:
            processor.delay += 4;
            processor.af.high ^= cgbl_bus_read(processor.pc.word++);
            break;
        default:
            break;
    }
    processor.af.carry = false;
    processor.af.half_carry = false;
    processor.af.negative = false;
    processor.af.zero = !processor.af.high;
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_processor_instruction_xxx(void)
{
    processor.delay = 4;
    return CGBL_ERROR("Illegal instruction: [%04X] %02X", processor.instruction.address, processor.instruction.opcode);
}

static cgbl_error_e (*const INSTRUCTION[][CGBL_INSTRUCTION_MAX])(void) = {
    {
        cgbl_processor_instruction_nop, cgbl_processor_instruction_ld, cgbl_processor_instruction_ld, cgbl_processor_instruction_inc_16,
        cgbl_processor_instruction_inc, cgbl_processor_instruction_dec, cgbl_processor_instruction_ld, cgbl_processor_instruction_rlca,
        cgbl_processor_instruction_ld, cgbl_processor_instruction_add_hl, cgbl_processor_instruction_ld, cgbl_processor_instruction_dec_16,
        cgbl_processor_instruction_inc, cgbl_processor_instruction_dec, cgbl_processor_instruction_ld, cgbl_processor_instruction_rrca,
        cgbl_processor_instruction_stop, cgbl_processor_instruction_ld, cgbl_processor_instruction_ld, cgbl_processor_instruction_inc_16,
        cgbl_processor_instruction_inc, cgbl_processor_instruction_dec, cgbl_processor_instruction_ld, cgbl_processor_instruction_rla,
        cgbl_processor_instruction_jr, cgbl_processor_instruction_add_hl, cgbl_processor_instruction_ld, cgbl_processor_instruction_dec_16,
        cgbl_processor_instruction_inc, cgbl_processor_instruction_dec, cgbl_processor_instruction_ld, cgbl_processor_instruction_rra,
        cgbl_processor_instruction_jr, cgbl_processor_instruction_ld, cgbl_processor_instruction_ld, cgbl_processor_instruction_inc_16,
        cgbl_processor_instruction_inc, cgbl_processor_instruction_dec, cgbl_processor_instruction_ld, cgbl_processor_instruction_daa,
        cgbl_processor_instruction_jr, cgbl_processor_instruction_add_hl, cgbl_processor_instruction_ld, cgbl_processor_instruction_dec_16,
        cgbl_processor_instruction_inc, cgbl_processor_instruction_dec, cgbl_processor_instruction_ld, cgbl_processor_instruction_cpl,
        cgbl_processor_instruction_jr, cgbl_processor_instruction_ld, cgbl_processor_instruction_ld, cgbl_processor_instruction_inc_16,
        cgbl_processor_instruction_inc, cgbl_processor_instruction_dec, cgbl_processor_instruction_ld, cgbl_processor_instruction_scf,
        cgbl_processor_instruction_jr, cgbl_processor_instruction_add_hl, cgbl_processor_instruction_ld, cgbl_processor_instruction_dec_16,
        cgbl_processor_instruction_inc, cgbl_processor_instruction_dec, cgbl_processor_instruction_ld, cgbl_processor_instruction_ccf,
        cgbl_processor_instruction_ld, cgbl_processor_instruction_ld, cgbl_processor_instruction_ld, cgbl_processor_instruction_ld,
        cgbl_processor_instruction_ld, cgbl_processor_instruction_ld, cgbl_processor_instruction_ld, cgbl_processor_instruction_ld,
        cgbl_processor_instruction_ld, cgbl_processor_instruction_ld, cgbl_processor_instruction_ld, cgbl_processor_instruction_ld,
        cgbl_processor_instruction_ld, cgbl_processor_instruction_ld, cgbl_processor_instruction_ld, cgbl_processor_instruction_ld,
        cgbl_processor_instruction_ld, cgbl_processor_instruction_ld, cgbl_processor_instruction_ld, cgbl_processor_instruction_ld,
        cgbl_processor_instruction_ld, cgbl_processor_instruction_ld, cgbl_processor_instruction_ld, cgbl_processor_instruction_ld,
        cgbl_processor_instruction_ld, cgbl_processor_instruction_ld, cgbl_processor_instruction_ld, cgbl_processor_instruction_ld,
        cgbl_processor_instruction_ld, cgbl_processor_instruction_ld, cgbl_processor_instruction_ld, cgbl_processor_instruction_ld,
        cgbl_processor_instruction_ld, cgbl_processor_instruction_ld, cgbl_processor_instruction_ld, cgbl_processor_instruction_ld,
        cgbl_processor_instruction_ld, cgbl_processor_instruction_ld, cgbl_processor_instruction_ld, cgbl_processor_instruction_ld,
        cgbl_processor_instruction_ld, cgbl_processor_instruction_ld, cgbl_processor_instruction_ld, cgbl_processor_instruction_ld,
        cgbl_processor_instruction_ld, cgbl_processor_instruction_ld, cgbl_processor_instruction_ld, cgbl_processor_instruction_ld,
        cgbl_processor_instruction_ld, cgbl_processor_instruction_ld, cgbl_processor_instruction_ld, cgbl_processor_instruction_ld,
        cgbl_processor_instruction_ld, cgbl_processor_instruction_ld, cgbl_processor_instruction_halt, cgbl_processor_instruction_ld,
        cgbl_processor_instruction_ld, cgbl_processor_instruction_ld, cgbl_processor_instruction_ld, cgbl_processor_instruction_ld,
        cgbl_processor_instruction_ld, cgbl_processor_instruction_ld, cgbl_processor_instruction_ld, cgbl_processor_instruction_ld,
        cgbl_processor_instruction_add, cgbl_processor_instruction_add, cgbl_processor_instruction_add, cgbl_processor_instruction_add,
        cgbl_processor_instruction_add, cgbl_processor_instruction_add, cgbl_processor_instruction_add, cgbl_processor_instruction_add,
        cgbl_processor_instruction_adc, cgbl_processor_instruction_adc, cgbl_processor_instruction_adc, cgbl_processor_instruction_adc,
        cgbl_processor_instruction_adc, cgbl_processor_instruction_adc, cgbl_processor_instruction_adc, cgbl_processor_instruction_adc,
        cgbl_processor_instruction_sub, cgbl_processor_instruction_sub, cgbl_processor_instruction_sub, cgbl_processor_instruction_sub,
        cgbl_processor_instruction_sub, cgbl_processor_instruction_sub, cgbl_processor_instruction_sub, cgbl_processor_instruction_sub,
        cgbl_processor_instruction_sbc, cgbl_processor_instruction_sbc, cgbl_processor_instruction_sbc, cgbl_processor_instruction_sbc,
        cgbl_processor_instruction_sbc, cgbl_processor_instruction_sbc, cgbl_processor_instruction_sbc, cgbl_processor_instruction_sbc,
        cgbl_processor_instruction_and, cgbl_processor_instruction_and, cgbl_processor_instruction_and, cgbl_processor_instruction_and,
        cgbl_processor_instruction_and, cgbl_processor_instruction_and, cgbl_processor_instruction_and, cgbl_processor_instruction_and,
        cgbl_processor_instruction_xor, cgbl_processor_instruction_xor, cgbl_processor_instruction_xor, cgbl_processor_instruction_xor,
        cgbl_processor_instruction_xor, cgbl_processor_instruction_xor, cgbl_processor_instruction_xor, cgbl_processor_instruction_xor,
        cgbl_processor_instruction_or, cgbl_processor_instruction_or, cgbl_processor_instruction_or, cgbl_processor_instruction_or,
        cgbl_processor_instruction_or, cgbl_processor_instruction_or, cgbl_processor_instruction_or, cgbl_processor_instruction_or,
        cgbl_processor_instruction_cp, cgbl_processor_instruction_cp, cgbl_processor_instruction_cp, cgbl_processor_instruction_cp,
        cgbl_processor_instruction_cp, cgbl_processor_instruction_cp, cgbl_processor_instruction_cp, cgbl_processor_instruction_cp,
        cgbl_processor_instruction_ret, cgbl_processor_instruction_pop, cgbl_processor_instruction_jp, cgbl_processor_instruction_jp,
        cgbl_processor_instruction_call, cgbl_processor_instruction_push, cgbl_processor_instruction_add, cgbl_processor_instruction_rst,
        cgbl_processor_instruction_ret, cgbl_processor_instruction_ret, cgbl_processor_instruction_jp, cgbl_processor_instruction_xxx,
        cgbl_processor_instruction_call, cgbl_processor_instruction_call, cgbl_processor_instruction_adc, cgbl_processor_instruction_rst,
        cgbl_processor_instruction_ret, cgbl_processor_instruction_pop, cgbl_processor_instruction_jp, cgbl_processor_instruction_xxx,
        cgbl_processor_instruction_call, cgbl_processor_instruction_push, cgbl_processor_instruction_sub, cgbl_processor_instruction_rst,
        cgbl_processor_instruction_ret, cgbl_processor_instruction_reti, cgbl_processor_instruction_jp, cgbl_processor_instruction_xxx,
        cgbl_processor_instruction_call, cgbl_processor_instruction_xxx, cgbl_processor_instruction_sbc, cgbl_processor_instruction_rst,
        cgbl_processor_instruction_ld, cgbl_processor_instruction_pop, cgbl_processor_instruction_ld, cgbl_processor_instruction_xxx,
        cgbl_processor_instruction_xxx, cgbl_processor_instruction_push, cgbl_processor_instruction_and, cgbl_processor_instruction_rst,
        cgbl_processor_instruction_add_sp, cgbl_processor_instruction_jp_hl, cgbl_processor_instruction_ld, cgbl_processor_instruction_xxx,
        cgbl_processor_instruction_xxx, cgbl_processor_instruction_xxx, cgbl_processor_instruction_xor, cgbl_processor_instruction_rst,
        cgbl_processor_instruction_ld, cgbl_processor_instruction_pop, cgbl_processor_instruction_ld, cgbl_processor_instruction_di,
        cgbl_processor_instruction_xxx, cgbl_processor_instruction_push, cgbl_processor_instruction_or, cgbl_processor_instruction_rst,
        cgbl_processor_instruction_ld_hl, cgbl_processor_instruction_ld, cgbl_processor_instruction_ld, cgbl_processor_instruction_ei,
        cgbl_processor_instruction_xxx, cgbl_processor_instruction_xxx, cgbl_processor_instruction_cp, cgbl_processor_instruction_rst,
    },
    {
        cgbl_processor_instruction_rlc, cgbl_processor_instruction_rlc, cgbl_processor_instruction_rlc, cgbl_processor_instruction_rlc,
        cgbl_processor_instruction_rlc, cgbl_processor_instruction_rlc, cgbl_processor_instruction_rlc, cgbl_processor_instruction_rlc,
        cgbl_processor_instruction_rrc, cgbl_processor_instruction_rrc, cgbl_processor_instruction_rrc, cgbl_processor_instruction_rrc,
        cgbl_processor_instruction_rrc, cgbl_processor_instruction_rrc, cgbl_processor_instruction_rrc, cgbl_processor_instruction_rrc,
        cgbl_processor_instruction_rl, cgbl_processor_instruction_rl, cgbl_processor_instruction_rl, cgbl_processor_instruction_rl,
        cgbl_processor_instruction_rl, cgbl_processor_instruction_rl, cgbl_processor_instruction_rl, cgbl_processor_instruction_rl,
        cgbl_processor_instruction_rr, cgbl_processor_instruction_rr, cgbl_processor_instruction_rr, cgbl_processor_instruction_rr,
        cgbl_processor_instruction_rr, cgbl_processor_instruction_rr, cgbl_processor_instruction_rr, cgbl_processor_instruction_rr,
        cgbl_processor_instruction_sla, cgbl_processor_instruction_sla, cgbl_processor_instruction_sla, cgbl_processor_instruction_sla,
        cgbl_processor_instruction_sla, cgbl_processor_instruction_sla, cgbl_processor_instruction_sla, cgbl_processor_instruction_sla,
        cgbl_processor_instruction_sra, cgbl_processor_instruction_sra, cgbl_processor_instruction_sra, cgbl_processor_instruction_sra,
        cgbl_processor_instruction_sra, cgbl_processor_instruction_sra, cgbl_processor_instruction_sra, cgbl_processor_instruction_sra,
        cgbl_processor_instruction_swap, cgbl_processor_instruction_swap, cgbl_processor_instruction_swap, cgbl_processor_instruction_swap,
        cgbl_processor_instruction_swap, cgbl_processor_instruction_swap, cgbl_processor_instruction_swap, cgbl_processor_instruction_swap,
        cgbl_processor_instruction_srl, cgbl_processor_instruction_srl, cgbl_processor_instruction_srl, cgbl_processor_instruction_srl,
        cgbl_processor_instruction_srl, cgbl_processor_instruction_srl, cgbl_processor_instruction_srl, cgbl_processor_instruction_srl,
        cgbl_processor_instruction_bit, cgbl_processor_instruction_bit, cgbl_processor_instruction_bit, cgbl_processor_instruction_bit,
        cgbl_processor_instruction_bit, cgbl_processor_instruction_bit, cgbl_processor_instruction_bit, cgbl_processor_instruction_bit,
        cgbl_processor_instruction_bit, cgbl_processor_instruction_bit, cgbl_processor_instruction_bit, cgbl_processor_instruction_bit,
        cgbl_processor_instruction_bit, cgbl_processor_instruction_bit, cgbl_processor_instruction_bit, cgbl_processor_instruction_bit,
        cgbl_processor_instruction_bit, cgbl_processor_instruction_bit, cgbl_processor_instruction_bit, cgbl_processor_instruction_bit,
        cgbl_processor_instruction_bit, cgbl_processor_instruction_bit, cgbl_processor_instruction_bit, cgbl_processor_instruction_bit,
        cgbl_processor_instruction_bit, cgbl_processor_instruction_bit, cgbl_processor_instruction_bit, cgbl_processor_instruction_bit,
        cgbl_processor_instruction_bit, cgbl_processor_instruction_bit, cgbl_processor_instruction_bit, cgbl_processor_instruction_bit,
        cgbl_processor_instruction_bit, cgbl_processor_instruction_bit, cgbl_processor_instruction_bit, cgbl_processor_instruction_bit,
        cgbl_processor_instruction_bit, cgbl_processor_instruction_bit, cgbl_processor_instruction_bit, cgbl_processor_instruction_bit,
        cgbl_processor_instruction_bit, cgbl_processor_instruction_bit, cgbl_processor_instruction_bit, cgbl_processor_instruction_bit,
        cgbl_processor_instruction_bit, cgbl_processor_instruction_bit, cgbl_processor_instruction_bit, cgbl_processor_instruction_bit,
        cgbl_processor_instruction_bit, cgbl_processor_instruction_bit, cgbl_processor_instruction_bit, cgbl_processor_instruction_bit,
        cgbl_processor_instruction_bit, cgbl_processor_instruction_bit, cgbl_processor_instruction_bit, cgbl_processor_instruction_bit,
        cgbl_processor_instruction_bit, cgbl_processor_instruction_bit, cgbl_processor_instruction_bit, cgbl_processor_instruction_bit,
        cgbl_processor_instruction_bit, cgbl_processor_instruction_bit, cgbl_processor_instruction_bit, cgbl_processor_instruction_bit,
        cgbl_processor_instruction_res, cgbl_processor_instruction_res, cgbl_processor_instruction_res, cgbl_processor_instruction_res,
        cgbl_processor_instruction_res, cgbl_processor_instruction_res, cgbl_processor_instruction_res, cgbl_processor_instruction_res,
        cgbl_processor_instruction_res, cgbl_processor_instruction_res, cgbl_processor_instruction_res, cgbl_processor_instruction_res,
        cgbl_processor_instruction_res, cgbl_processor_instruction_res, cgbl_processor_instruction_res, cgbl_processor_instruction_res,
        cgbl_processor_instruction_res, cgbl_processor_instruction_res, cgbl_processor_instruction_res, cgbl_processor_instruction_res,
        cgbl_processor_instruction_res, cgbl_processor_instruction_res, cgbl_processor_instruction_res, cgbl_processor_instruction_res,
        cgbl_processor_instruction_res, cgbl_processor_instruction_res, cgbl_processor_instruction_res, cgbl_processor_instruction_res,
        cgbl_processor_instruction_res, cgbl_processor_instruction_res, cgbl_processor_instruction_res, cgbl_processor_instruction_res,
        cgbl_processor_instruction_res, cgbl_processor_instruction_res, cgbl_processor_instruction_res, cgbl_processor_instruction_res,
        cgbl_processor_instruction_res, cgbl_processor_instruction_res, cgbl_processor_instruction_res, cgbl_processor_instruction_res,
        cgbl_processor_instruction_res, cgbl_processor_instruction_res, cgbl_processor_instruction_res, cgbl_processor_instruction_res,
        cgbl_processor_instruction_res, cgbl_processor_instruction_res, cgbl_processor_instruction_res, cgbl_processor_instruction_res,
        cgbl_processor_instruction_res, cgbl_processor_instruction_res, cgbl_processor_instruction_res, cgbl_processor_instruction_res,
        cgbl_processor_instruction_res, cgbl_processor_instruction_res, cgbl_processor_instruction_res, cgbl_processor_instruction_res,
        cgbl_processor_instruction_res, cgbl_processor_instruction_res, cgbl_processor_instruction_res, cgbl_processor_instruction_res,
        cgbl_processor_instruction_res, cgbl_processor_instruction_res, cgbl_processor_instruction_res, cgbl_processor_instruction_res,
        cgbl_processor_instruction_set, cgbl_processor_instruction_set, cgbl_processor_instruction_set, cgbl_processor_instruction_set,
        cgbl_processor_instruction_set, cgbl_processor_instruction_set, cgbl_processor_instruction_set, cgbl_processor_instruction_set,
        cgbl_processor_instruction_set, cgbl_processor_instruction_set, cgbl_processor_instruction_set, cgbl_processor_instruction_set,
        cgbl_processor_instruction_set, cgbl_processor_instruction_set, cgbl_processor_instruction_set, cgbl_processor_instruction_set,
        cgbl_processor_instruction_set, cgbl_processor_instruction_set, cgbl_processor_instruction_set, cgbl_processor_instruction_set,
        cgbl_processor_instruction_set, cgbl_processor_instruction_set, cgbl_processor_instruction_set, cgbl_processor_instruction_set,
        cgbl_processor_instruction_set, cgbl_processor_instruction_set, cgbl_processor_instruction_set, cgbl_processor_instruction_set,
        cgbl_processor_instruction_set, cgbl_processor_instruction_set, cgbl_processor_instruction_set, cgbl_processor_instruction_set,
        cgbl_processor_instruction_set, cgbl_processor_instruction_set, cgbl_processor_instruction_set, cgbl_processor_instruction_set,
        cgbl_processor_instruction_set, cgbl_processor_instruction_set, cgbl_processor_instruction_set, cgbl_processor_instruction_set,
        cgbl_processor_instruction_set, cgbl_processor_instruction_set, cgbl_processor_instruction_set, cgbl_processor_instruction_set,
        cgbl_processor_instruction_set, cgbl_processor_instruction_set, cgbl_processor_instruction_set, cgbl_processor_instruction_set,
        cgbl_processor_instruction_set, cgbl_processor_instruction_set, cgbl_processor_instruction_set, cgbl_processor_instruction_set,
        cgbl_processor_instruction_set, cgbl_processor_instruction_set, cgbl_processor_instruction_set, cgbl_processor_instruction_set,
        cgbl_processor_instruction_set, cgbl_processor_instruction_set, cgbl_processor_instruction_set, cgbl_processor_instruction_set,
        cgbl_processor_instruction_set, cgbl_processor_instruction_set, cgbl_processor_instruction_set, cgbl_processor_instruction_set,
    },
};

static cgbl_error_e cgbl_processor_instruction(void)
{
    bool prefix = false;
    processor.instruction.address = processor.pc.word;
    processor.instruction.opcode = cgbl_bus_read(processor.pc.word++);
    if (processor.halt_bug)
    {
        processor.halt_bug = false;
        --processor.pc.word;
    }
    if ((prefix = (processor.instruction.opcode == CGBL_INSTRUCTION_PREFIX)))
    {
        processor.instruction.opcode = cgbl_bus_read(processor.pc.word++);
    }
    return INSTRUCTION[prefix][processor.instruction.opcode]();
}

static void cgbl_processor_interrupt(void)
{
    for (cgbl_interrupt_e interrupt = 0; interrupt < CGBL_INTERRUPT_MAX; ++interrupt)
    {
        uint8_t mask = 1 << interrupt;
        if (processor.interrupt.enable.raw & processor.interrupt.flag.raw & mask)
        {
            processor.delay = 4;
            processor.interrupt.flag.raw &= ~mask;
            if (processor.halt_bug)
            {
                processor.halt_bug = false;
            }
            else
            {
                processor.delay += 16;
                processor.interrupt.delay = 0;
                processor.interrupt.enabled = false;
                cgbl_bus_write(--processor.sp.word, processor.pc.high);
                cgbl_bus_write(--processor.sp.word, processor.pc.low);
                processor.pc.word = (8 * interrupt) + 0x40;
            }
            break;
        }
    }
}

bool cgbl_processor_halted(void)
{
    return processor.halted;
}

uint8_t cgbl_processor_read(uint16_t address)
{
    uint8_t result = 0xFF;
    switch (address)
    {
        case CGBL_PROCESSOR_INTERRUPT_ENABLE:
            result = processor.interrupt.enable.raw;
            break;
        case CGBL_PROCESSOR_INTERRUPT_FLAG:
            result = processor.interrupt.flag.raw;
            break;
        default:
            break;
    }
    return result;
}

void cgbl_processor_reset(void)
{
    memset(&processor, 0, sizeof (processor));
    processor.interrupt.flag.raw = 0xE0;
}

void cgbl_processor_signal(cgbl_interrupt_e interrupt)
{
    cgbl_processor_write(CGBL_PROCESSOR_INTERRUPT_FLAG,
        cgbl_processor_read(CGBL_PROCESSOR_INTERRUPT_FLAG) | (1 << interrupt));
}

cgbl_error_e cgbl_processor_step(void)
{
    cgbl_error_e result = CGBL_SUCCESS;
    for (uint8_t cycle = 0; cycle < ((cgbl_bus_speed() == CGBL_SPEED_DOUBLE) ? 2 : 1); ++cycle)
    {
        if (!processor.delay)
        {
            if (processor.interrupt.delay && !--processor.interrupt.delay)
            {
                processor.interrupt.enabled = true;
            }
            if (processor.interrupt.enable.raw & processor.interrupt.flag.raw & 0x1F)
            {
                processor.halted = false;
                if (processor.interrupt.enabled)
                {
                    cgbl_processor_interrupt();
                }
                else if (!processor.stopped)
                {
                    if ((result = cgbl_processor_instruction()) != CGBL_SUCCESS)
                    {
                        return result;
                    }
                }
                else
                {
                    processor.delay = 4;
                }
            }
            else if (!processor.halted && !processor.stopped)
            {
                cgbl_processor_instruction();
            }
            else
            {
                processor.delay = 4;
            }
        }
        --processor.delay;
    }
    return result;
}

bool cgbl_processor_stopped(void)
{
    return processor.stopped;
}

void cgbl_processor_write(uint16_t address, uint8_t data)
{
    switch (address)
    {
        case CGBL_PROCESSOR_INTERRUPT_ENABLE:
            processor.interrupt.enable.raw = data;
            break;
        case CGBL_PROCESSOR_INTERRUPT_FLAG:
            processor.interrupt.flag.raw = data | 0xE0;
            if (processor.interrupt.flag.input)
            {
                processor.stopped = false;
            }
            break;
        default:
            break;
    }
}
