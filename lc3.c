#include "lc3.h"

void lc3_init(lc3machine* state) 
{
	state->pc = 0x3000;
	state->cc = LC3_ZERO;
	state->halted = 0;
}

void lc3_load(lc3machine* state, const char* program)
{

	FILE* file = NULL;
	int i = 0;
	short j = 0;
	short k = 0;
	int l = 0;
	int fsize = 0;
	file = fopen(program, "r");
	if (file!=NULL){
		fseek(file, 0, SEEK_END); 		// go to end of file
		fsize = ftell(file); 			// get size of file
		rewind(file); 				// go back to beginning
		short nums[fsize/2]; 			//#of bytes divided by 2 is number of instructions
		for (i = 0; i < fsize; i++){ 
			if (i%2==0){
				j = fgetc(file); 	// read first byte
				j = j<<8;
				
			}
			else{
				k = fgetc(file); 	// read second byte, if i is odd, merge bytes
				k = k|j; 	 	// Or second byte with shifted first byte
				nums[l] = k	 	// put short in array
				l++;
			}
		}
		int origNum = 0;
		int amtNum = 1;
		int count = 0;
		while(count < fsize/2){ 		//read file while there are still values to read
			short orig = nums[origNum]; 	// origin is first value in array
			count++;			// increment count everytime value is stored in nums
			short amount = nums[amtNum]; 	// amount of instructions is second value in array
			count++;
			for(i = amtNum+1; i < amtNum+amount+1; i++) // fill as many values as specified by amount
			{
				state->mem[orig] = nums[i]; 	// put amount (as specified above) of values in memory
				orig++; 			// increment memory address
				count++; 			// increment count
			}
			origNum = amount+amtNum+1;		// new orig = first value after old values
			amtNum  = origNum+1; 			//new amtNum is right after new orig
		}
			
	
		fclose(file);
	}
	else{
		printf("File Open failed.");
	}
}

void lc3_step_one(lc3machine* state)
{
		state->halted = 0; 
		unsigned short instr = lc3_fetch(state); 	// get instruction returned from fetch
		lc3_execute(state, instr);  			// execute that instruction
}

void lc3_run(lc3machine* state, int num_steps)
{
	state->halted = 0;
	if (num_steps == -1){
		while(!state->halted){ 		//while not halted
			lc3_step_one(state);
		}
	}
	else{
		int i;
		for (i = 0; i < num_steps; i++)
		{
			if (!state->halted) 	// if not halted
				lc3_step_one(state);  // do as many steps as specified
			else 
				break;
		}
	}
}

unsigned short lc3_fetch(lc3machine* state)
{
	int memAddress = state->pc; 	//get address currently in pc
	(state->pc)++; 			// increment pc
	return state->mem[memAddress]; // return instruction at spot in memory at address
}

void lc3_execute(lc3machine* state, unsigned short instruction)
{
	short opcode = instruction>>12;
	if (opcode == 1) // ADD
	{
		short dr = (instruction&0x0E00)>>9; 	// get dr
		short sr1 = (instruction&0x01C0)>>6; 	// get sr
		if (instruction&0x0020){ 		// check whether adding register or imm5
			short imm5 = (instruction&0x001F)|((instruction&0x0010)? 0xFFE0:0); // sign extend the imm5
			state->regs[dr] = state->regs[sr1] + imm5; // dr = sr + imm5
		}
		else{
			short sr2 = instruction&0x0007; 			// get sr2
			state->regs[dr] = state->regs[sr1] + state->regs[sr2]; // dr = sr + sr2
		}
		if (state->regs[dr] == 0) // check cc
			state->cc = LC3_ZERO;
		else
			state->cc = ((state->regs[dr])>0) ? LC3_POSITIVE : LC3_NEGATIVE;
	}
	else if (opcode == 5) // AND
	{
		short dr = (instruction&0x0E00)>>9; // get dr
		short sr1 = (instruction&0x01C0)>>6; // get sr
		if (instruction&0x0020){ // adding register or imm5?
			short imm5 = (instruction&0x001F)|((instruction&0x0010)? 0xFFE0:0); // sign extend the imm5
			signed short answer = state->regs[sr1]&imm5; // dr = sr&imm5
			state->regs[dr] = answer;
		}
		else{
			short sr2 = instruction&0x0007; // get sr2
			signed short answer = (state->regs[sr1])&(state->regs[sr2]); // dr = sr1&sr2
			state->regs[dr] = answer; 
		}
		if (state->regs[dr]==0) // check cc
			state->cc = LC3_ZERO;
		else
			state->cc = (state->regs[dr]>0) ? LC3_POSITIVE : LC3_NEGATIVE;
	}
	else if (opcode == 0) // BR
	{
		short condition = instruction>>9; //opcode is 0 already, this gets nzp
		short offset = (instruction&0x01FF)|((instruction&0x100)? 0xFE00:0); // this tests PCoffset9 and sign extends it
		if (condition & state->cc) // check condition codes and current condition of lc3
		{
			state->pc = state->pc + offset; // pc = pc + offset, branch to new location
		}
	}
	else if (opcode == 12) // JMP    RET?
	{
		short baseR = (instruction&0x01C0)>>6; // get base register
		state->pc = state->regs[baseR];
	}
	else if (opcode == 4) // JSR
	{
		state->regs[7] = state->pc;
		if (instruction&0x0800){
			short offset11 = (instruction&0x07FF)|((instruction&0x0400)? 0xF000:0); // sign extend PCoffset11
			state->pc = state->pc + offset11; 
		}
		else{
			short baseR = (instruction&0x01C0)>>6; // get base register
			state->pc = state->regs[baseR];
		}
	}
	else if (opcode == 2) // LD
	{
		short dr = (instruction & 0xE00)>>9; // get dr
		short offset = (instruction&0x01FF)|((instruction&0x100)? 0xFE00:0); // this tests PCoffset9 and sign extends it
		int loc = (state->pc + offset);
		int data = state->mem[loc];
		state->regs[dr] = data;
		if (data == 0)
			state->cc = LC3_ZERO;
		else
			state->cc = (data>0)? LC3_POSITIVE : LC3_NEGATIVE;
	}
	else if (opcode == 10) // LDI
	{
		short dr = (instruction & 0xE00)>>9; // get dr
		short offset = (instruction&0x01FF)|((instruction&0x100)? 0xFE00:0); // this tests PCoffset9 and sign extends it
		int address = state->pc + offset; // get address of incremented pc + offset
		int address2 = state->mem[address]; // interrogate memory for next address
		short data = state->mem[address2]; // get data at that address
		state->regs[dr] = data; // put in dest reg
		if (data==0) // set cc
			state->cc = LC3_ZERO;
		else
			state->cc = (data>0) ? LC3_POSITIVE : LC3_NEGATIVE;
	}
	else if (opcode == 6) // LDR
	{
		short dr = (instruction & 0xE00)>>9; // get dr
		short baseR = (instruction&0x01C0)>>6; // get base register
		short offset6 = (instruction&0x003F)|((instruction&0x0020)? 0xFFC0:0); // get offset6
		int address = state->regs[baseR] + offset6; // get addrss of value in register + offset
		short data = state->mem[address]; // get data from that address in memory
		state->regs[dr] = data; // put data in dest reg
		if (data==0) // set cc
			state->cc = LC3_ZERO;
		else
			state->cc = (data>0) ? LC3_POSITIVE : LC3_NEGATIVE;
	}
	else if (opcode == 14) // LEA
	{
		short dr = (instruction & 0xE00)>>9; // get dr
		short offset = (instruction&0x01FF)|((instruction&0x100)? 0xFE00:0); // this tests PCoffset9 and sign extends it
		int address = state->pc + offset;  // compute pc + offset
		state->regs[dr] = address;  // put this address in dest reg
		if (address == 0) // set cc
			state->cc = LC3_ZERO;
		else
			state->cc = LC3_POSITIVE;
	}
	else if (opcode == 9) // NOT
	{
		short dr = (instruction&0x0E00)>>9; // get dr
		short sr1 = (instruction&0x01C0)>>6; // get sr
		int data = ~(state->regs[sr1]); // negate whatever is in source reg
		state->regs[dr] = data; // put it in dest reg
		if (data == 0) // set cc
			state->cc = LC3_ZERO;
		else
			state->cc = (data>0) ? LC3_POSITIVE : LC3_NEGATIVE;
	}
	
	else if (opcode == 3) // ST
	{
		short sr = (instruction & 0xE00)>>9; // get sr
		short offset = (instruction&0x01FF)|((instruction&0x100)? 0xFE00:0); // this tests PCoffset9 and sign extends it
		state->mem[state->pc + offset] = state->regs[sr]; //store value in sr at memory addres from pc+offset
	}
	else if (opcode == 11) // STI
	{
		short sr = (instruction & 0xE00)>>9; // get sr
		short offset = (instruction&0x01FF)|((instruction&0x100)? 0xFE00:0); // this tests PCoffset9 and sign extends it
		int address = state->mem[state->pc + offset]; 	 // address is memory address from pc + offset
		state->mem[address] = state->regs[sr]; // that address equals contents in source reg
	}
	else if (opcode == 7) // STR
	{
		short sr = (instruction & 0xE00)>>9; // get dr
		short baseR = (instruction&0x01C0)>>6; // get base register
		short offset6 = (instruction&0x003F)|((instruction&0x0020)? 0xFFC0:0); // get offset6
		int address = state->regs[baseR] + offset6;  // address is contents of base reg + offset
		state->mem[address] = state->regs[sr]; // store contents in source reg at that address
	}
	else if (opcode == 15) // TRAP
	{
		short vector = (instruction&0x00FF); // get 8 bit unsigned vector
		lc3_trap(state, vector); // call trap with that vector
	}

}

void lc3_disassemble(unsigned short instruction)
{
	if (instruction == 0)
		printf("NOP\n");
	short opcode = instruction>>12; // get opcode for instruction
	
	if (opcode == 0) // BR instruction
	{
		short condition = instruction>>9; //opcode is 0 already, this gets nzp
		short offset = (instruction&0x01FF)|((instruction&0x100)? 0xFE00:0); // this tests PCoffset9 and sign extends it
		if (condition == 0) 
			printf("BR %d\n", offset);
		else if (condition==1)
			printf("BRP %d\n", offset);
		else if (condition==2)
			printf("BRZ %d\n", offset);
		else if (condition==4)
			printf("BRN %d\n", offset);
		else if (condition==3)
			printf("BRZP %d\n", offset);
		else if (condition==5)
			printf("BRNP %d\n", offset);
		else if (condition==6)
			printf("BRNZ %d\n", offset);
		else if (condition==7)
			printf("BRNZP %d\n", offset);
	}
	else if (opcode == 1) // ADD instruction
	{
		short dr = (instruction&0x0E00)>>9; // get dr
		short sr1 = (instruction&0x01C0)>>6; // get sr
		if (instruction&0x0020){ // adding register or imm5?
			short imm5 = (instruction&0x001F)|((instruction&0x0010)? 0xFFE0:0); // sign extend the imm5
			printf("ADD R%d, R%d, %d\n", dr, sr1, imm5);
		}
		else{
			short sr2 = instruction&0x0007; // get sr2
			printf("ADD R%d, R%d, R%d\n", dr, sr1, sr2);
		}
	}
	else if (opcode==2) // LD instruction
	{
		short dr = (instruction & 0xE00)>>9; // get dr
		short offset = (instruction&0x01FF)|((instruction&0x100)? 0xFE00:0); // this tests PCoffset9 and sign extends it
		printf("LD R%d, %d\n", dr, offset);
	}
	else if (opcode==3) // ST instruction
	{
		short sr = (instruction & 0xE00)>>9; // get sr
		short offset = (instruction&0x01FF)|((instruction&0x100)? 0xFE00:0); // this tests PCoffset9 and sign extends it
		printf("ST R%d, %d\n", sr, offset);
	}
	else if (opcode == 4) //JSR instruction
	{
		if (instruction&0x0800){
			short offset11 = (instruction&0x07FF)|((instruction&0x0400)? 0xF000:0); // sign extend PCoffset11
			printf("JSR %d\n", offset11);
		}
		else{
			short baseR = (instruction&0x01C0)>>6; // get base register
			printf("JSRR R%d\n", baseR);
		}	
	}
	else if (opcode==5) // AND instruction
	{
		short dr = (instruction&0x0E00)>>9; // get dr
		short sr1 = (instruction&0x01C0)>>6; // get sr
		if (instruction&0x0020){ // adding register or imm5?
			short imm5 = (instruction&0x001F)|((instruction&0x0010)? 0xFFE0:0); // sign extend the imm5
			printf("AND R%d, R%d, %d\n", dr, sr1, imm5);
		}
		else{
			short sr2 = instruction&0x0007; // get sr2
			printf("AND R%d, R%d, R%d\n", dr, sr1, sr2);
		}
	}
	else if (opcode==6) // LDR instruction
	{
		short dr = (instruction & 0xE00)>>9; // get dr
		short baseR = (instruction&0x01C0)>>6; // get base register
		short offset6 = (instruction&0x003F)|((instruction&0x0020)? 0xFFC0:0); // get offset6
		printf("LDR R%d, R%d, %d\n", dr, baseR, offset6);	
	}
	else if (opcode==7) // STR instruction
	{
		short sr = (instruction & 0xE00)>>9; // get dr
		short baseR = (instruction&0x01C0)>>6; // get base register
		short offset6 = (instruction&0x003F)|((instruction&0x0020)? 0xFFC0:0); // get offset6
		printf("STR R%d, R%d, %d\n", sr, baseR, offset6);		
	}
	else if (opcode==9) // NOT instruction
	{
		short dr = (instruction&0x0E00)>>9; // get dr
		short sr1 = (instruction&0x01C0)>>6; // get sr
		printf("NOT R%d, R%d\n", dr, sr1);
	}
	else if (opcode == 10) // LDI instruction
	{
		short dr = (instruction & 0xE00)>>9; // get dr
		short offset = (instruction&0x01FF)|((instruction&0x100)? 0xFE00:0); // this tests PCoffset9 and sign extends it
		printf("LDI R%d, %d\n", dr, offset);
	}
	else if (opcode == 11) // STI instruction
	{
		short sr = (instruction & 0xE00)>>9; // get sr
		short offset = (instruction&0x01FF)|((instruction&0x100)? 0xFE00:0); // this tests PCoffset9 and sign extends it
		printf("STI R%d, %d\n", sr, offset);	
	}
	else if (opcode == 12) // JMP instruction
	{
		short baseR = (instruction&0x01C0)>>6; // get base register
		printf("JMP R%d\n", baseR);
	}
	else if (opcode == 13){ // RTI -> error
		printf("ERROR\n");
	}
	else if (opcode==14) // LEA instruction
	{
		short dr = (instruction & 0xE00)>>9; // get dr
		short offset = (instruction&0x01FF)|((instruction&0x100)? 0xFE00:0); // this tests PCoffset9 and sign extends it
		printf("LEA R%d, %d\n", dr, offset);
	}
	else if (opcode ==15) // TRAP
	{
		short vector = (instruction&0x00FF); // get 8 bit unsigned vector
		printf("TRAP %#04x\n", vector);
	}
}

void lc3_trap(lc3machine* state, unsigned char vector8)
{
	if (vector8==0x20)
	{
		char c;
		if (scanf("%c", &c)!=0) // read in char
			state->regs[0] = c; // put it in r0
	}
	else if (vector8==0x21)
		printf("%c\n", (char)state->regs[0]); // print char from r0
	else if (vector8 == 0x22)
	{
		int i = 0;
		unsigned short address = state->regs[0]; // go to address in r0
		while((state->mem[address+i])!=0) // print chars in there until 0 is reached
		{
			printf("%c", state->mem[address+i]);
			i++;
		}
		printf("\n");
	}
	else if (vector8 == 0x23)
	{
		char c;
		printf("Type a character for input: "); // char input goes in r0 and is echoed
		fflush(stdout);
		if (scanf("%c", &c)!=0){
			state->regs[0] = c;
			printf("\n%c\n", c);
		}
		state->regs[0] = state->regs[0] & 0x00FF; // clear high 8 bits of r0
	}
	else if (vector8 == 0x24)
	{
		int i = 0;
		unsigned short address = state->regs[0]; // r0 has address of char array
		while((state->mem[address+i])!=0)
		{
			short c2 = (state->mem[address+i])>>8; // 2nd char in bits [8:15]
			short c1 = (state->mem[address+i])&0x00FF; // 1st char in bits [0:7]
				printf("%c%c", c1, c2); // print out chars
			if (c2==0) // continue til 0 is reached
				break;
			else
				i++;
		}
		printf("\n");
	}
	else if (vector8 == 0x25)
	{
		state->halted = 1; // turn on halted flag
		(state->pc)--;  // decrement pc
		printf("You have halted the program.\n"); // ouput message
	}
	else
	{	
		state->regs[7] = state->pc; // put pc value in R7
		short vec = vector8&0x00ff; //zero extend vector8
		state->pc = state->mem[vec]; // put mem location at vec goes in pc
	}
}






