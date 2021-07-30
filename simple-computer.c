#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>
#include "loongarch.h"

#define MAX_MEM_SIZE 64 //The max memory size - 64 words (256 bytes)

typedef struct memory
{
	uint32_t addr[MAX_MEM_SIZE];
} MEMORY;

typedef struct cpu
{
	//Control registers
	uint32_t PC; //Program counter
	uint32_t IR; //Instruction regiser

	//General purpose register
	uint32_t R[32]; // 32 Registers
} CPU;

typedef struct computer
{
	CPU cpu;
	MEMORY memory;
} COMPUTER;

// represente the type of the instruction
enum
{
	TYPE_2R,
	TYPE_3R,
	TYPE_2RI8,
	TYPE_4R,
	TYPE_2RI12,
	TYPE_2RI14,
	TYPE_2RI20,
	TYPE_2RI16,
	TYPE_2RI21,
	TYPE_I26,
	TYPE_SP
};

typedef struct instruction
{
	int ins_type;

	int op;
	int rk;
	int rj;
	int rd;
	// int shamt;
	int imm;
	int offs;

} INSTRUCTION;

// Memory Structure
/*
	0  |-------C-------|
	1  |-------O-------|   CODE Segment
	.  |-------D-------|
	5  |-------E-------|
	6  |-------D-------|
	7  |-------A-------|   DATA Segment
	.  |-------T-------|
	10 |-------A-------|
	11 |---------------|
	12 |---------------|
	.  |---------------|
	59 |-------S-------|
	60 |-------T-------|
	61 |-------A-------|   STACK
	62 |-------C-------|
	63 |-------K-------|
*/

// represent the 32 general registers
enum
{
	$r0,
	$r1,
	$r2,
	$r3,
	$r4,
	$r5,
	$r6,
	$r7,
	$r8,
	$r9,
	$r10,
	$r11,
	$r12,
	$r13,
	$r14,
	$r15,
	$r16,
	$r17,
	$r18,
	$r19,
	$r20,
	$r21,
	$r22,
	$r23,
	$r24,
	$r25,
	$r26,
	$r27,
	$r28,
	$r29,
	$r30,
	$r31
};
char *reg[] = {"$r0 ", "$r1 ", "$r2 ", "$r3 ", "$r4 ", "$r5 ", "$r6 ", "$r7 ", "$r8 ", "$r9 ", "$r10",
			   "$r11", "r12", "$r13", "$r14", "$r15", "$r16", "$r17", "$r18", "$r19", "$r20", "$r21", "$r22", "$r23",
			   "$r24", "$r25", "$r26", "$r27", "$r28", "$r29", "$r30", "$r31"};

int computer_load_init(COMPUTER *, char *);
int cpu_cycle(COMPUTER *);
int fetch(COMPUTER *);

int print_cpu(COMPUTER *);
int print_memory(COMPUTER *);
int print_instruction(int, uint32_t);
int decode(uint32_t, INSTRUCTION *);
int execute(COMPUTER *, INSTRUCTION *);

int isEnd = 0; // halt flag

int main(int argc, char **args)
{

	printf("-------------------------------------------\n");
	printf("|Simple von Neumann Computer for LoongArch|\n");
	printf("|  Zili Shao@CSE,CUHK    Liang Ma@CS,SDU  |\n");
	printf("-------------------------------------------\n");

	if (argc != 3)
	{
		printf("\nUsage: ./cpu program start_addr\n");
		printf("\t program: program file name; start_addr: the start address for initial PC\n \n");
		exit(-1);
	}

	COMPUTER comp;

	//Initialize: Load the program into the memory, and initialize all regisrters;
	if (computer_load_init(&comp, args[1]) < 0)
	{
		printf("Error: computer_poweron_init()\n");
		exit(-1);
	}

	//Set PC and start the cpu execution cycle
	comp.cpu.PC = atoi(args[2]);
	if (comp.cpu.PC >= MAX_MEM_SIZE || comp.cpu.PC < 0)
	{
		printf("Error: start_addr should be in 0-63.\n");
		exit(-1);
	}

	// Execute CPU cyles: fetch, decode, execution, and increment PC; Repeat
	while (1)
	{
		printf("\n\nBefore\n");
		print_cpu(&comp);

		if (cpu_cycle(&comp) < 0)
			break;

		printf("\nAfter\n");
		print_cpu(&comp);
	}
	// print_memory(&comp);
	return 0;
}

int cpu_cycle(COMPUTER *cp)
{
	//initialise the instruction struct
	INSTRUCTION ins = {-1, -1, -1, -1, -1, -1, -1};
	if (fetch(cp) < 0)
		return -1;
	if (decode(cp->cpu.IR, &ins) < 0)
		return -1;
	if (execute(cp, &ins) < 0)
		return -1;
	return 0;
}

int fetch(COMPUTER *cp)
{
	uint32_t pc = cp->cpu.PC;

	if (pc < 0 || pc > 63)
	{
		printf("PC is not in 0-63.\n");
		return -1;
	}
	else
	{
		cp->cpu.IR = cp->memory.addr[pc];
		cp->cpu.PC++;
		printf("**********************************************************************");
		printf("\nFETCH: the instruction is : 0x%x\n", cp->cpu.IR);
		return 0;
	}
}

//MIPS Instruction category
//R op(6)<<26 rs(5)<<21  rt(5)<<16  rd(5)<<11  shamt(5)<<6 func(6)
//I op(6)<<26 rs(5)<<21  rt(5)<<16  imm(16)
//J op(6)<<26 address(26)

// classify the category of the instruction, and decompose the instruction into several part including opcode, register and imm etc.
int decode(uint32_t instr, INSTRUCTION *ins)
{
	printf("DECODE: ");
	// get the opcode
	int op6 = (instr&(strtol("111111", NULL, 2) << 26)) >> 26;
	int op7 = (instr&(strtol("1111111", NULL, 2) << 25)) >> 25;
	int op8 = (instr&(strtol("11111111", NULL, 2) << 24)) >> 24;
	int op10 = (instr&(strtol("1111111111", NULL, 2) << 22)) >> 22;
	int op12 = (instr&(strtol("111111111111", NULL, 2) << 20)) >> 20;
	int op14 = (instr&(strtol("11111111111111", NULL, 2) << 18)) >> 18;
	int op17 = (instr&(strtol("11111111111111111", NULL, 2) << 15)) >> 15;
	int op22 = (instr&(strtol("1111111111111111111111", NULL, 2) << 10)) >> 10;

	switch(op6)
	{
		case OP6_HALT:
			// halt	(0x3f)	Effect: Stop CPU and exit
			printf("HALT!\n");
			return -1;
		case OP6_NOP:
			// NOP	(0xff)	Effect: PC <- PC + 1
           ins->op = op6;
           ins->ins_type = TYPE_2RI16;
//			cp->cpu.PC += 1;
			printf("NOP\n");
			return 0;
		case OP6_JIRL:
		case OP6_BEQ:
		case OP6_BNE:
		case OP6_BGE:
			/* 2RI16-type instruction (6bits opcode + I16 + 5bits rj + 5bits rd)*/
			ins->op = op6;
			ins->ins_type = TYPE_2RI16;
			ins->imm = (instr & (strtol("1111111111111111", NULL, 2) << 10)) >> 10;
			ins->rj = ((instr & (strtol("11111", NULL, 2) << 5)) >> 5);
			ins->rd = instr & (strtol("11111", NULL, 2));
			printf("[type: 2RI16]	op = %d, imm = %d, rj = %d, rd = %d\n", ins->op, ins->imm, ins->rj, ins->rd);
			return 0;
		case OP6_BEQZ:
		case OP6_BNEZ:
			/* 2RI21-type instruction (6bits opcode + I21[15:0] + 5bits rj + I21[20:16])*/
			ins->op = op6;
			ins->ins_type = TYPE_2RI21;
			ins->imm = (instr & (strtol("11111", NULL, 2))) << 15;
			ins->imm = ins->imm + ((instr & (strtol("1111111111111111", NULL, 2) << 10)) >> 10);
			ins->rj = ((instr & (strtol("11111", NULL, 2) << 5)) >> 5);
			printf("[type: 2RI21]	op = %d, imm = %d, rj = %d\n", ins->op, ins->imm, ins->rj);
			return 0;
		case OP6_B:
		case OP6_BL:
			/* I26-type instruction (6bits opcode + I26[15:0] + I26[25:16])*/
			//await implementation
			return 0;
		default:
			break;
	}
	switch(op7)
	{
		case OP7_LU12I_W:
		case OP7_PCADDU12I:
			/* 2RI20-type instruction (7bits opcode + I20 + 5bits rd)*/
			//await implementation
			return 0;
		default:
			break;
	}
	switch(op8)
	{
		case OP8_LDPTR_W:
		case OP8_STPTR_W:
			/* 2RI14-type instruction (8bits opcode + I14 + 5bits rj + 5bits rd)*/
			//await implementation
			return 0;
		default:
			break;
	}
	switch(op10)
	{
		case OP10_ADDI_D:
		case OP10_ADDI_W:
		case OP10_ANDI:
		case OP10_ORI:
		case OP10_XORI:
		case OP10_LD_D:
		case OP10_LD_B:
		case OP10_ST_W:
		case OP10_ST_D:
		case OP10_LD_BU:
			/* 2RI12-type instruction (10bits opcode + I12 + 5bits rj + 5bits rd)*/
			ins->op = op10;
			ins->ins_type = TYPE_2RI12;
			ins->imm = ((instr & (strtol("111111111111", NULL, 2) << 10)) >> 10);
			ins->rj = ((instr & (strtol("11111", NULL, 2) << 5)) >> 5);
			ins->rd = instr & (strtol("11111", NULL, 2));
			printf("[type: 2RI12]	op = %d, imm = %d, rj = %d, rd = %d\n", ins->op, ins->imm, ins->rj, ins->rd);
			return 0;
		case OP10_BSTRPICK_D:
			//   (10bits opcode + 6bits msbd + 6bits lsbd + 5bits rj + 5bits rd)
			//await implementation
			return 0;
		default:
			break;
	}
	switch(op17)
	{
		case OP17_SLTU:
		case OP17_NOR:
		case OP17_AND:
		case OP17_OR:
		case OP17_XOR:
			/* 3R-type instruction (17bits opcode + 5bits rk + 5bits rj + 5bits rd)*/
			ins->op = op17;
			ins->ins_type = TYPE_3R;
			ins->rk = ((instr & (strtol("11111", NULL, 2) << 10)) >> 10);
			ins->rj = ((instr & (strtol("11111", NULL, 2) << 5)) >> 5);
			ins->rd = instr & (strtol("11111", NULL, 2));
			printf("[type: 3R]	op = %d, rk = %d, rj = %d, rd = %d\n", ins->op, ins->rk, ins->rj, ins->rd);
			return 0;
		case OP17_SLLI_W:
		case OP17_SRLI_W:
			//(17bits opcode + I5 + 5bits rj + 5bits rd)
			//await implementation
			return 0;
		default:
			printf("ERROR: cannot decode instruction!");
			return -1;
	}
}
	

int execute(COMPUTER *cp, INSTRUCTION *ins)
{
	printf("EXECUTE: ");
	switch (ins->ins_type)
	{
		case TYPE_2RI16:
			switch(ins->op)
			{
              case OP6_NOP:
                  cp->cpu.PC += 1;
                  break;
				case OP6_JIRL:
					cp->cpu.R[ins->rd] = cp->cpu.PC + 1;
					cp->cpu.PC = cp->cpu.R[ins->rj] + ins->offs;
					printf("JIRL [PC <- rj+(sign-extend)offs; rd <- PC+1]\n");
					break;
				case OP6_BEQ:
					printf("BEQ not implemented\n");
					break;
				case OP6_BNE:
					printf("BNE not implemented\n");
					break;
				case OP6_BGE:
					printf("BGE not implemented\n");
					break;
				default:
					printf("ERROR: cannot execute instruction!");
					return -1;
			}
           break;
		case TYPE_2RI21:
			switch(ins->op)
			{
				case OP6_BEQZ:
					printf("BEQZ not implemented\n");
					break;
				case OP6_BNEZ:
					printf("BNEZ not implemented\n");
					break;

				default:
					printf("ERROR: cannot execute instruction!");
					return -1;
			}
           break;
		case TYPE_I26:
			switch(ins->op)
			{
				case OP6_B:
					printf("B not implemented\n");
					break;
				case OP6_BL:
					printf("BL not implemented\n");
					break;
				default:
					printf("ERROR: cannot execute instruction!");
					return -1;
			}
           break;
		case TYPE_2RI20:
			switch(ins->op)
			{
				case OP7_LU12I_W:
					printf("LU12I.W not implemented\n");
					break;
				case OP7_PCADDU12I:
					printf("PCADDU12I not implemented\n");
					break;
				default:
					printf("ERROR: cannot execute instruction!");
					return -1;
			}
           break;
		case TYPE_2RI14:
			switch(ins->op)
			{
				case OP8_LDPTR_W:
					printf("LDPTR.W not implemented\n");
					break;
				case OP8_STPTR_W:
					printf("STPTR.W not implemented\n");
					break;
				default:
					printf("ERROR: cannot execute instruction!");
					return -1;
			}
           break;
		case TYPE_2RI12:
			switch(ins->op)
			{
				case OP10_ADDI_D:
					printf("ADDI.D not implemented\n");
					break;
				case OP10_ADDI_W:
					printf("ADDI.W not implemented\n");
					break;
                // multiple instructions awaits implementation here
				default:
					printf("ERROR: cannot execute instruction!");
					return -1;
			}
           break;
           default:
               printf("ERROR: instruction type error!");
               return -1;
	}
	printf("**********************************************************************");
	return 0;
}

int computer_load_init(COMPUTER *cp, char *file)
{
	//load the image file
	int fd;
	int ret;

	// open the file
	if ((fd = open(file, O_RDONLY)) < 0)
	{
		printf("Error: open().\n");
		exit(-1);
	}

	// read from the program file (the program file <= 256 bytes) into the memory
	if ((ret = read(fd, &cp->memory, MAX_MEM_SIZE * 4)) < 0)
	{
		printf("Error: read().\n");
		exit(-1);
	}
	else if (ret > (MAX_MEM_SIZE * 4))
	{
		printf("Error: read() - Program is too big. \n");
		exit(-1);
	}

	//Initialize all registers
	cp->cpu.PC = 0; //Program counter
	cp->cpu.IR = 0; //Instruction regiser

	//General purpose register
	int reg_index = 0;
	for (reg_index = 0; reg_index < 32; reg_index++)
	{
		cp->cpu.R[reg_index] = 0;	//set general registers to 0
	} 

	return 0;
}

int print_cpu(COMPUTER *cp)
{
	printf("CPU Registers: PC-%d, [PC]-0x%x\n", cp->cpu.PC, cp->cpu.IR);
	for (int i = 0; i < 32; i++)
	{
		if (i != 0 && i % 4 == 0)
		{
			printf("\n");
		}
		printf("$reg%02d:0x%x, ", i, cp->cpu.R[i]);
		//printf("%s: %x ,",reg[i],cp->cpu.R[i]);
	}
	printf("\n");
	return 0;
}

int print_memory(COMPUTER *cp)
{
	//print the memory contents
	int i;
	for (i = 0; i < 64; i++)
	{
		print_instruction(i, cp->memory.addr[i]);
	}
	return 0;
}

int print_instruction(int i, uint32_t inst)
{
	int8_t *p = (int8_t *)&inst;

	int8_t low_addr_value = *p;
	int8_t sec_addr_value = *(p + 1);
	int8_t third_addr_value = *(p + 2);
	int8_t high_addr_value = *(p + 3);
	/* Intel32 is little endian - the least significant byte first (lowest address) and the most significant byte last (highest address) */
	printf("[%d]: Instruction-0x%x;LowAddr-%d,Second-%d,Third-%d,HighAddr-%d\n",
		   i, inst, low_addr_value, sec_addr_value, third_addr_value, high_addr_value);
	return 0;
}
