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
int print_instruction(uint32_t *);

int main(int argc, char **args)
{
	printf("----------------------------------------------------------------------\n");
	printf("|Program file genration for Simple von Neumann Computer for LoongArch|\n");
	printf("|              Zili Shao@CSE,CUHK      Liang Ma@CS,SDU               |\n");
	printf("----------------------------------------------------------------------\n\n");

	if (argc != 2)
	{
		printf("\nUsage: ./prog-gen file-name\n\n");
		exit(-1);
	}

	int fd;

	if ((fd = open(args[1], O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR)) < 0)
	{
		printf("Error: open().\n");
		exit(-1);
	}

	uint32_t memory[MAX_MEM_SIZE];

	// ADDI.D, op = OP10_ADDI_D, imm = 32, rj = $r3, rd = $r3
	memory[0] = (OP10_ADDI_D << 22) + (32 << 10) + ($r3 << 5) + $r3;	//r3 = r3 + 32
	// ST.D, op = OP10_LT_D, offset = 10, rj = $r2, rd = $r3
	memory[1] = (OP10_ST_D << 22) + (10 << 10) + ($r2 << 5) + $r3;		//mem(r2 + 10) = r3
	// LD.D, op = OP10_LD_D, offset = 10, rj = $r7, rd = $r4
	memory[2] = (OP10_LD_D << 22) + (10 << 10)  + ($r7 << 5) + $r4;		//r4 = mem(r7 + 10)
	// BEQZ, op = OP6_BEQZ
	memory[3] = (OP6_BEQZ << 26) + (2 << 10) + ($r2 << 5);		// if r2 = 0, jump to mem(5)
	// ADDI.D, op = OP10_ADDI_D, imm = 666, rj = $r3, rd = $r3
	memory[4] = (OP10_ADDI_D << 22) + (666 << 10) + ($r5 << 5) + $r5;	//r5 = r5 + 666
	// ADDI.D, op = OP10_ADDI_D, imm = 999, rj = $r3, rd = $r3
	memory[5] = (OP10_ADDI_D << 22) + (999 << 10) + ($r6 << 5) + $r6;	//r6 = r6 + 999
	//	XORI, op = OP10_XORI, imm = 0xfff, rj = $r6, rd = $r7
	memory[6] = (OP10_XORI << 22) + (0xfff << 10) + ($r6 << 5) + $r7;	//r7 = r6 ^ fff
	// the last instruction will be halt, indicates the end of the program
	memory[7] = 0xfc000000;	// instruction: HALT

	/* write the memory contents into the image file*/
	int wcount;
	if ((wcount = write(fd, memory, MAX_MEM_SIZE * 4)) < 0)
	{
		printf("Error: write()\n");
		exit(-1);
	}
	else if (wcount != MAX_MEM_SIZE * 4)
	{
		printf("Error: write bytes not equal 256.\n");
		exit(-1);
	}

	close(fd);

	return 0;
}

int print_instruction(uint32_t *p_i)
{
	uint8_t *p = (uint8_t *)p_i;

	uint8_t low_addr_value = *(p + 3);
	uint8_t sec_addr_value = *(p + 2);
	uint8_t third_addr_value = *(p + 1);
	uint8_t high_addr_value = *(p);

	/* LoongArch is big endian -  the most significant byte first (lowest address) and the least significant byte last (highest address) */
	printf("Instruction- 0x%x; LowAddr- %#x, Second- %#x, Third- %#x, HighAddr- %#x\n",
		   *p_i, low_addr_value, sec_addr_value, third_addr_value, high_addr_value);
	return 0;
}

// https://www.cnblogs.com/blacksunny/p/7192416.html
// https://blog.csdn.net/qq_41848006/article/details/82256626
