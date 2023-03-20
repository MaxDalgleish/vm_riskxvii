#include <stdio.h>
#include <string.h>
#include <stdlib.h>



int big_endian(int memory[], int pc_val) {

	int a = memory[pc_val];
	int b = memory[pc_val + 1];
	int c = memory[pc_val + 2];
	int d = memory[pc_val + 3];
	int instruction = ((((((d << 8) | c) << 8) | b) << 8) | a);
	return instruction;

}

// Converts a number of 12 bit to 36 bit
int sign_extending(int instr) {
	int value = (instr & 0xFFF) | ((instr & 0x800) ? 0xFFFFF000 : 0);
	return value;
}

int func3_extract(int instruction) {
	return (instruction >> 12 & 0b111);
}

int func7_extract(int instruction) {
	return (instruction >> 25 & 0x7F);
}

int rd_extract(int instruction) {
	return (instruction >> 7 & 0x1F);
}

int rs1_extract(int instruction) {
	return (instruction >> 15 & 0x1F);
}

int rs2_extract(int instruction) {
	return (instruction >> 20 & 0x1F);
}

int s_imm(int instruction) {
	int imm = instruction >> 25 & 0x7F;
	imm <<= 5;
	imm = (instruction >> 7 & 0x1F) | imm;
	return imm;
}

int virtual_routines(int mem_val, int param) {
	switch (mem_val)
	{
		// print value being stored as ascii
		case 0x800: {
			printf("%c", param);
			return 0;
		}
		// print signed integer to stdout
		case 0x804: {
			printf("%d", param);
			return 0;
		}

		// prints value being stored in hex form
		case 0x808: {
			printf("%x", param);
			return 0;
		}

		// CPU Halt
		case 0x80c: {
			printf("CPU Halt Requested\n");
			exit(1);
		}

		// scan in character
		case 0x812: {
			char c;
			scanf("%c", &c);
			return c;
		}

		// Scan in integer
		case 0x816: {
			int num;
		scanf("%d", &num);
		return num;
		}
	}

	return 0;
}



int main(int argc, char **argv) {
	int reg[32] = {0};
	int memory[2048] = {0};
	int pc_val = 0;
	FILE *bin_input = fopen(argv[1], "rb");
	for (int i = 0; i < 2048; i++) {

		int c = fgetc(bin_input);
		memory[pc_val] = c;
		pc_val += 1;
	}

	pc_val = 0;
	int i = 0; 
	while (i < 100) {
		int instruction = big_endian(memory, pc_val);
		printf("%x ", instruction);
		int opcode = instruction & 0x7F;
		switch (opcode)
		{
			// addi  or format I
			case 0b0010011: {
				if (func3_extract(instruction) == 0b0) {
					int rd = rd_extract(instruction);
					int rs1 = rs1_extract(instruction);
					int imm = sign_extending(instruction >> 20);
					reg[rd] = reg[rs1] + imm;
					printf("%d %d ", rd, reg[rd]);
				}
				break;
			}
			// jal
			case 0b1101111: {
				int rd = rd_extract(instruction);
				reg[rd] = pc_val + 4;
				int imm1 = (instruction >> 21) & 0x3FF;
				int imm2 = (instruction >> 20) & 0b1;
				int imm3 = (instruction >> 12) & 0xFF;
				int imm4 = (instruction >> 31) & 0b1;
				int imm = ((((((imm4 << 8) | imm3) << 1) | imm2) << 10) | imm1);
				pc_val = pc_val + (imm << 1) - 4;
				break;
			}
			// lui
			case 0b0110111: {
				int rd = rd_extract(instruction);
				int imm = ((instruction >> 12) & 0xFFFFF) << 12;
				reg[rd] = imm;
				break;
			}
			
			// Memory access
			case 0b0000011: {
				int func3 = func3_extract(instruction);
				if (func3 == 0b010) {
					int rd = rd_extract(instruction);
					int rs1 = rs1_extract(instruction);
					int imm = instruction >> 20 & 0xFFF;
					if (reg[rs1] + imm >= 0x800) {
						reg[rd] = virtual_routines(reg[rs1] + imm, 0);
					} else {
						reg[rd] = memory[reg[rs1] + imm];
					}
				}
				break;
			}

			// add
			case 0b0110011: {
				int func3 = func3_extract(instruction);
				int func7 = func7_extract(instruction);
				if (func3 == 0 && func7 == 0) {
					int rd = rd_extract(instruction);
					int rs1 = rs1_extract(instruction);
					int rs2 = rs2_extract(instruction);
					reg[rd] = reg[rs1] + reg[rs2];
				}
				break;
			}

			//save memory
			case 0b0100011: {
				int func3 = func3_extract(instruction);
				if (func3 == 0b010) {
					int rs1 = rs1_extract(instruction);
					int rs2 = rs2_extract(instruction);
					int imm = sign_extending(s_imm(instruction));
					if (reg[rs1] + imm >= 0x800) {
						virtual_routines(reg[rs1] + imm, reg[rs2]);
					} else {
						memory[reg[rs1] + imm] = reg[rs2];
					}
				}
				break;
			}

			// jalr
			case 0b1100111: {
				int func3 = func3_extract(instruction);
				if (func3 == 0) {
					int rd = rd_extract(instruction);
					int rs1 = rs1_extract(instruction);
					int imm = sign_extending(instruction >> 20);
					reg[rd] = pc_val + 4;
					printf("h %d %d %d h", reg[rs1], imm, rs1);
					pc_val = reg[rs1] + imm;
				}
			}
			default:
				break;
		}
		pc_val += 4;
		i++;
	}
	return 0;
}