/* LC3 simulator file.
 * Complete the functions!
 * This is a front-end to the code you wrote in lc3.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lc3.h"



void cmd_registers(lc3machine* mach);
void cmd_dump(lc3machine* mach, int start, int end);
void cmd_list(lc3machine* mach, int start, int end);
void cmd_setaddr(lc3machine* mach, int address, short value);
void cmd_setreg(lc3machine* mach, int reg, short value);

#define PROMPT "Enter a command: "


// Don't touch I will use this during grading.
#ifndef NO_MAIN_PLZ
int main(int argc, char **argv) 
{

	const char* prog;

	/* We expect only one argument for the program... */
	if (argc != 2) 
	{
		fprintf(stderr, "Usage: %s file.obj\n", argv[0]);
		return 1;
	}
	/* We want to open the file and make sure that it exists. */
	prog = argv[1];
		

		lc3machine* lc3 = malloc(sizeof(lc3machine));
		lc3_init(lc3);
		lc3_load(lc3, prog); 
	

	printf("LC-3 simulator and debugger\n");
	printf("Written by Emily Cahill\n");
	printf("File given %s\n", prog);

	/* Run this loop until we are told to stop going. */
	while (1) 
	{
		printf("%s", PROMPT);
		char buffer[50], cmd[10], num1[5], num2[5];
		if (fgets(buffer, 50, stdin)!=NULL){ // put input in buffer
		int numCmd = sscanf(buffer, "%[^ ] %[^ ] %[^ ]", cmd, num1, num2); // check for number of arguments
		if (numCmd==1)
		{
			if (strncmp(cmd, "step", 4)==0) // if command is step, call one step
				lc3_step_one(lc3);
			else if (strncmp(cmd, "quit", 4) == 0){ // if command is quit, quit simulator
				free(lc3);
				return 0;
			}
			else if (strncmp(cmd, "continue", 8) == 0){ // if command in continue, keep running instructions until halted
				lc3_run(lc3, -1);				
			}
			else if (strncmp(cmd, "registers", 9) == 0) // if command is registers, show contents of registers
				cmd_registers(lc3);
		}
		else if (numCmd == 2)
		{
			if (strncmp(cmd, "step", 4) == 0){ // if command is step, w/ number
				int num = 0;
				num = strtol(num1, NULL, 10); //convert num1 from char to int
				lc3_run(lc3, num); // execute number of steps commanded
			}
			else if (strncmp(cmd, "dump", 4) == 0){
				int start;
				char* p;
				p = strtok(buffer, " x"); // break input command into tokens to get start address
				p = strtok(NULL, " x");
				start = strtol(p, NULL, 16); // convert to hex
				cmd_dump(lc3, start, -1); // dump memory at start address only
			}
			else if (strncmp(cmd, "list", 4) == 0){
				int start;
				char* p;
				p = strtok(buffer, " x"); // break up input command 
				p = strtok(NULL, " x");
				start = strtol(p, NULL, 16); // convert to hex
				cmd_list(lc3, start, -1); // disassemble instructions in memory at start address only
			}
		}
		else if (numCmd == 3)
		{
			if (strncmp(cmd, "dump", 4) == 0){
				int start, end; 
				char* p;
				p = strtok(buffer, " x");
				p = strtok(NULL, " x");
				start = strtol(p, NULL, 16);
				p = strtok(NULL, " x"); // break up string one more time to get last hex value
				end = strtol(p, NULL, 16);
				cmd_dump(lc3, start, end); // dump memory from start to end
			}
			else if (strncmp(cmd, "list", 4) == 0) {
				int start, end;
				char* p;
				p = strtok(buffer, " x");
				p = strtok(NULL, " x");
				start = strtol(p, NULL, 16);
				p = strtok(NULL, " x");
				end = strtol(p, NULL, 16);
				cmd_list(lc3, start, end); // disassemble instructions in memory from start to end
			}
			else if (strncmp(cmd, "setaddr", 7) == 0){
				int address, value;
				char* p;
				p = strtok(buffer, " x");
				p = strtok(NULL, " x");
				address = strtol(p, NULL, 16);
				p = strtok(NULL, " x");
				value = strtol(p, NULL, 10);
				cmd_setaddr(lc3, address, value); // set address in memory to given value
			}
			else if (strncmp(cmd, "setreg", 6) == 0){
				int reg, value;
				char* p ;
				p = strtok(buffer, " R");
				p = strtok(NULL, " R");
				reg = strtol(p, NULL, 10);
				p = strtok(NULL, " ");
				value = strtol(p, NULL, 10);
				cmd_setreg(lc3, reg, value);
			}
				
		}
		}
		
	}

	return 0;
}
#endif // IFNDEF NO_MAIN_PLZ

/* cmd_step and cmd_continue 's functionality are provided in lc3_run
Therefore to execute the step and coninute commands you can just call lc3_run with the correct parameters*/

/* cmd_registers
Should print out all of the registers and the PC and CC in both hex and signed decimal. The format should be as follows

R0 dec|hex	R1 dec|hex	R2 dec|hex	R3 dec|hex
R4 dec|hex	R5 dec|hex	R6 dec|hex	R7 dec|hex
CC n/z/p
PC hex

hex is to be preceded with only an x and you should always print out 4 characters representing the hexadecimal representation the hex letters are CAPITALIZED.
n/z/p will only be one of the characters n, z, or p
between each register's information is a single tab.

Example output 
R0 0|x0000	R1 -1|xFFFF	R2 2|x0002	R3 32767|x7FFF
R4 31|x001F	R5 -32768|x8000	R6 6|x0006	R7 11111|x2B67
CC z
PC x3000
*/
void cmd_registers(lc3machine* mach)
{	
	int i, cci;
	char p = 'p'; // print one of these based on CC
	char n = 'n';
	char z = 'z';
	for (i = 0; i < 8; i ++)
	{
		short a = mach->regs[i]; // print each reg and info
		printf("R%d %d|x%04X	", i, a, a);
	}
	cci = (int) mach->cc; // print cc
	if (cci == 1)
		printf("\nCC %c\n", p);
	else if (cci == 2)
		printf("\nCC %c\n", z);
	else
		printf("\nCC %c\n", n);
	printf("PC x%04X\n", mach->pc); // print contents of pc
}

/* cmd_dump
Should print out the contents of memory from start to end
If end is -1 then just print out memory[start]

Output format
addr: dec|hex

Example format
x0000: 37|x0025
x0001: 25|x0019
*/
void cmd_dump(lc3machine* mach, int start, int end)
{
	int i;
	if (end == -1)
	{
		short d = mach->mem[start]; //print that memory spot and info
		printf("%#04X: %d|0x%04X\n", start, d, d);
	}
	else
	{
		for (i = start; i <=end; i++)
		{
			short d = mach->mem[i]; // print memory info requested, inclusive
			printf("%#04X: %d|0x%04X\n", i, d, d);
		}
	}
}

/* cmd_list
Should interpret the contents of memory from start to end as an assembled instruction
and disassemble it. e.g. if the data was x1000 then the output will be x3000
If end is -1 then just disassemble memory[start]

You will be calling lc3_disassemble to do the actual disassembling!
*/
void cmd_list(lc3machine* mach, int start, int end)
{
	int i;
	if (end == -1)
		lc3_disassemble(mach->mem[start]); // call disassemble on one instruction
	else
	{
		for (i = start; i <=end; i++)
		{
			lc3_disassemble(mach->mem[i]); // call disassemble on instructions requested
		}
	}
		
}

/* cmd_setaddr
 Should set a memory address to some value
*/
void cmd_setaddr(lc3machine* mach, int address, short value)
{
	mach->mem[address] = value; // set address in memory to given value
}

/* cmd_setreg
  Should set a register to some value passed in
*/
void cmd_setreg(lc3machine* mach, int reg, short value)
{
	mach->regs[reg] = value; // set register to given value
}

