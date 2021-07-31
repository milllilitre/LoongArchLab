#ifndef _LOONGARCH_H_
#define _LOONGARCH_H_

/* 2R-type instruction (22bits opcode + 5bits rj + 5bits rd)*/

/* 3R-type instruction (17bits opcode + 5bits rk + 5bits rj + 5bits rd)*/
#define OP17_SLTU   0x00025
#define OP17_NOR    0x00028
#define OP17_AND    0x00029
#define OP17_OR     0x00030
#define OP17_XOR    0x00031
//   (17bits opcode + I5 + 5bits rj + 5bits rd)
#define OP17_SLLI_W 0x00081
#define OP17_SRLI_W 0x00089

/* 2RI8-type instruction (14bits opcode + I8 + 5bits rj + 5bits rd)*/

/* 4R-type instruction (12bits opcode + 5bits ra + 5bits rk + 5bits rj + 5bits rd)*/

/* 2RI12-type instruction (10bits opcode + I12 + 5bits rj + 5bits rd)*/
#define OP10_ADDI_D 0x00b
#define OP10_ADDI_W 0x00c
#define OP10_ANDI   0x00d
#define OP10_ORI    0x00e
#define OP10_XORI   0x00f
#define OP10_LD_W   0x0a2
#define OP10_LD_D   0x0a3
#define OP10_LD_B   0x0a4
#define OP10_ST_W   0x0a6
#define OP10_ST_D   0x0a7
#define OP10_LD_BU  0x0a8
//   (10bits opcode + 6bits msbd + 6bits lsbd + 5bits rj + 5bits rd)
#define OP10_BSTRPICK_D 0x003

/* 2RI14-type instruction (8bits opcode + I14 + 5bits rj + 5bits rd)*/
#define OP8_LDPTR_W     0x24
#define OP8_STPTR_W     0x25

/* 2RI20-type instruction (7bits opcode + I20 + 5bits rd)*/
#define OP7_PCADDU12I 0x0E
#define OP7_LU12I_W 0x0a

/* 2RI16-type instruction (6bits opcode + I16 + 5bits rj + 5bits rd)*/
#define OP6_JIRL    0x13
#define OP6_BEQ     0x16
#define OP6_BNE     0x17
#define OP6_BGE     0x19

/* 2RI21-type instruction (6bits opcode + I21[15:0] + 5bits rj + I21[20:16])*/
#define OP6_BEQZ    0x10
#define OP6_BNEZ    0x11


/* I26-type instruction (6bits opcode + I26[15:0] + I26[25:16])*/
#define OP6_B       0x14
#define OP6_BL      0x15

//special instruction added to achieve extra function like halting the simulator
#define OP6_HALT   0x3f
#define OP6_NOP    0x3e
#endif
