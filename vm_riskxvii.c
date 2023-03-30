#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>


// TODO
// add the heap functionality
typedef struct heap_bank {
	char data[64];
	bool used;
	int size;
} heap_bank;

int big_endian(int memory[], int pc_val) {
	int a = memory[pc_val];
	int b = memory[pc_val + 1];
	int c = memory[pc_val + 2];
	int d = memory[pc_val + 3];
	int instruction = ((((((d << 8) | c) << 8) | b) << 8) | a);
	return instruction;

}

void register_dump(int pc, int *reg) {
	printf("PC = 0x%08x;\n", pc);
	for (int i = 0; i < 32; i++) {
		printf("R[%d] = 0x%08x;\n", i, reg[i]);
	}
}

void illegal_operation(int pc, int *reg, int instruction) {
	printf("Illegal Operation: 0x%08x\n", instruction);
	register_dump(pc, reg);
	exit(1);
}

// sign extends an imm
int sign_extending(int instr, int bits) {
	int sign = 1 << (bits - 1);
	int high_val = ((1 << (36 - bits)) - 1) << bits;
	int value = instr | ((instr & sign) ? high_val : 0);
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

int sb_imm(int instruction) {
	int imm = instruction >> 31 & 0x1;
	imm = imm << 1 | (instruction >> 7 & 0x1);
	imm = imm << 6 | (instruction >> 25 & 0x3F);
	imm = imm << 4 | (instruction >> 8 & 0xF);
	return imm;
}

uint16_t heap_malloc(int size, heap_bank *heap_banks) {
	int required_banks = (size + 64 - 1) / 64;
	int multiple_banks = 0;
	int starting_bank_pos = -1;

    for (int i = 0; i < 128; i++) {
        if (!heap_banks[i].used) {
            if (multiple_banks == 0) {
                starting_bank_pos = i;
            }
            multiple_banks++;
        } else {
           	multiple_banks = 0;
        }

        if (multiple_banks == required_banks) {
            for (int j = starting_bank_pos; j < starting_bank_pos + required_banks; j++) {
                heap_banks[j].used = true;
				if (j == starting_bank_pos) {
					heap_banks[j].size = starting_bank_pos + required_banks - 1;
				} else {
					heap_banks[j].size = 0;
				}
            }
			printf("malloc memory location: %x ", 0xb700 + starting_bank_pos * 64);
            return 0xb700 + starting_bank_pos * 64;
        }
    }
    return 0;
}

bool heap_free(int addr, heap_bank *heap_banks) {
    if (addr < 0xb700 || addr >= 0xb700 + 128 * 64) {
        return false;
    }

    int start_bank = (addr - 0xb700) / 64;

    if (!heap_banks[start_bank].used) {
        return false;
    }

    int total_size = heap_banks[start_bank].size;
    heap_banks[start_bank].used = false;
    heap_banks[start_bank].size = 0;

    for (int i = start_bank; i < start_bank + total_size; i++) {
        heap_banks[i].used = false;
        heap_banks[i].size = 0;
    }

    return true;
}

int heap_load(int mem_val, heap_bank *heap_banks, int size, int pc, int *reg, int instruction) {
	if (mem_val < 0xb700 || mem_val >= 0xb700 + 128 * 64) {
        return 0;
    }

	int current_bank = (mem_val - 0xb700) / 64;
	int data_addr = (mem_val - 0xb700) % 64;

	if (!heap_banks[current_bank].used) {
		illegal_operation(pc, reg, instruction);
		exit(1);
	}

	int result = heap_banks[current_bank].data[data_addr] & 0xFF;
	if (size > 8) {
		result = (result << 8) | (heap_banks[current_bank].data[data_addr + 1] & 0xFF);
	}

	if (size > 16) {
		result = (result << 8) | (heap_banks[current_bank].data[data_addr + 2] & 0xFF);
		result = (result << 8) | (heap_banks[current_bank].data[data_addr + 3] & 0xFF);
	}
	return result;
}

void heap_save(int mem_val, int *reg, int param, heap_bank *heap_banks, int size, int pc, int instruction) {
	if (mem_val < 0xb700 || mem_val >= 0xb700 + 128 * 64) {
        return;
    }

	int current_bank = (mem_val - 0xb700) / 64;
	if (!heap_banks[current_bank].used) {
		illegal_operation(pc, reg, instruction);
		exit(1);
	}
	int data_addr = (mem_val - 0xb700) % 64;

	int value = reg[param];
	if (size > 16) {
		for (int i = 31; i > 15; i--) {
			heap_banks[current_bank].data[data_addr] = (value >> i) & 1;
			data_addr += 1;
		}
	}

	if (size > 8) {
		for (int i = 15; i > 7; i--) {
			heap_banks[current_bank].data[data_addr] = (value >> i) & 1;
			data_addr += 1;
		}
	}

	for (int i = 7; i > -1; i--) {
		heap_banks[current_bank].data[data_addr] = (value >> i) & 1;
		data_addr += 1;
	}
}

int virtual_routines(int instruction, int mem_val, int param, int pc, int *reg, int *mem, heap_bank *heap_banks) {
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
			exit(0);
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

		// Dump PC {
		case 0x820: {
			printf("%x", pc);
			return 0;
		}

		// Register Dump
		case 0x824: {
			register_dump(pc, reg);
			return 0;
		}

		// Memory word dump
		case 0x828: {
			printf("%u", mem[param]);
			return 0;
		}

		case 0x830: {
			reg[28] = heap_malloc(param, heap_banks);
			return 0;
		}

		case 0x834: {
			if (!heap_free(param, heap_banks)) {
				illegal_operation(pc, reg, instruction);
			}
			return 0;
		}

		case 0xb700 ... (0xb700 + (128 * 64)): {
			if (!heap_banks[mem_val - 0xb700].used) {
				illegal_operation(pc, reg, instruction);
			}
			return 0;
		}

	}

	return 0;
}



int main(int argc, char **argv) {
	int reg[32] = {0};
	int memory[2048] = {0};
	int pc_val = 0;
	heap_bank heap_banks[128];
	for (int i = 0; i < 128; i++) {
		heap_banks[i].size = 0;
		heap_banks[i].used = false;
	}
	FILE *bin_input = fopen(argv[1], "rb");
	for (int i = 0; i < 2048; i++) {

		int c = fgetc(bin_input);
		memory[pc_val] = c;
		pc_val += 1;
	}

	pc_val = 0;
	while (pc_val < 1024) {
		int instruction = big_endian(memory, pc_val);
		int opcode = instruction & 0x7F;
		printf("%x, %08x\n", pc_val, instruction);

		switch (opcode)
		{
			// format I
			case 0b0010011: {
				int func3 = func3_extract(instruction);
				int rd = rd_extract(instruction);
				if (rd == 0) {
					break;
				}
				int rs1 = rs1_extract(instruction);
				int imm = sign_extending(instruction >> 20, 12);
				// addi
				if (func3 == 0) {
					reg[rd] = reg[rs1] + imm;
				// xori
				} else if (func3 == 0b100) {
					reg[rd] = reg[rs1] ^ imm;
				// ori
				} else if (func3 == 0b110) {
					reg[rd] = reg[rs1] | imm;
				// andi
				} else if (func3 == 0b111) {
					reg[rd] = reg[rs1] & imm;
				// slti
				} else if (func3 == 0b010) {
					reg[rd] = (reg[rs1] < imm) ? 1 : 0;
				// sltiu
				} else if (func3 == 0b011) {
					reg[rd] = ((unsigned int) reg[rs1] < (unsigned int) imm) ? 1 : 0;
				}
				break;
			}
			// jal
			case 0b1101111: {
				int rd = rd_extract(instruction);
				if (rd != 0) {
					reg[rd] = pc_val + 4;
				}
				int imm1 = (instruction >> 21) & 0x3FF;
				int imm2 = (instruction >> 20) & 0b1;
				int imm3 = (instruction >> 12) & 0xFF;
				int imm4 = (instruction >> 31) & 0b1;
				int imm = ((((((imm4 << 8) | imm3) << 1) | imm2) << 10) | imm1) << 1;
				imm = sign_extending(imm, 20);
				pc_val = pc_val + imm - 4;
				break;
			}
			// lui
			case 0b0110111: {
				int rd = rd_extract(instruction);
				if (rd == 0) {
					break;
				}
				int imm = ((instruction >> 12) & 0xFFFFF) << 12;
				reg[rd] = imm;
				break;
			}
			
			// load functions
			case 0b0000011: {
				int func3 = func3_extract(instruction);
				int rd = rd_extract(instruction);
				if (rd == 0) {
					break;
				}
				int rs1 = rs1_extract(instruction);
				int imm = sign_extending((instruction >> 20) & 0xFFF, 12);
				printf("heap_value: %x\n", reg[rs1] + imm);
				// load 32 bit value
				// lw
				if (func3 == 0b010) {
					if (reg[rs1] + imm >= 0xb700) {
						reg[rd] = heap_load(reg[rs1] + imm, heap_banks, 32, pc_val, reg, instruction);
					} else if (reg[rs1] + imm >= 0x800) {
						reg[rd] = virtual_routines(instruction,reg[rs1] + imm, 0, pc_val, reg, memory, heap_banks);
					} else {
						reg[rd] = (memory[reg[rs1] + imm] << 24) | (memory[reg[rs1] + imm + 1] << 16 | memory[reg[rs1] + imm + 2] << 8 | memory[reg[rs1] + imm + 3]);
					}
				// load a 16 bit value
				// lh
				} else if (func3 == 0b001) {
					if (reg[rs1] + imm >= 0xb700) {
						reg[rd] = heap_load(reg[rs1] + imm, heap_banks, 16, pc_val, reg, instruction);
					} else if (reg[rs1] + imm >= 0x800) {
						reg[rd] = virtual_routines(instruction, reg[rs1] + imm, 0, pc_val, reg, memory, heap_banks);
					} else {
						reg[rd] = sign_extending((memory[reg[rs1] + imm + 2] << 8) | memory[reg[rs1] + imm + 3], 16);
					}
				// load a 8 bit value
				// lb
				} else if (func3 == 0) {
					if (reg[rs1] + imm >= 0xb700) {
						reg[rd] = heap_load(reg[rs1] + imm, heap_banks, 8, pc_val, reg, instruction);
					} else if (reg[rs1] + imm >= 0x800) {
						reg[rd] = virtual_routines(instruction, reg[rs1] + imm, 0, pc_val, reg, memory, heap_banks);
					} else {
						reg[rd] = sign_extending(memory[reg[rs1] + imm + 3], 8);
					}
				// load a unsigned 8 bit value
				// lbu
				} else if (func3 == 0b100) {
					if (reg[rs1] + imm >= 0xb700) {
						reg[rd] = (unsigned int) heap_load(reg[rs1] + imm, heap_banks, 8, pc_val, reg, instruction);
					} else if (reg[rs1] + imm >= 0x800) {
						reg[rd] = (unsigned int) virtual_routines(instruction, reg[rs1] + imm, 0, pc_val, reg, memory, heap_banks);
					} else {
						reg[rd] = (unsigned int) memory[reg[rs1] + imm] & 0xFF;
					}
				// load a unsigned 16 bit value
				// lhu
				} else if (func3 == 0b101) {
					if (reg[rs1] + imm >= 0xb700) {
						reg[rd] = (unsigned int) heap_load(reg[rs1] + imm, heap_banks, 16, pc_val, reg, instruction);
					} else if (reg[rs1] + imm >= 0x800) {
						reg[rd] = (unsigned int) virtual_routines(instruction, reg[rs1] + imm, 0, pc_val, reg, memory, heap_banks);
					} else {
						reg[rd] = (unsigned int) memory[reg[rs1] + imm] & 0xFFFF;
					}
				}
				break;
			}

			// basic bitwise operations
			case 0b0110011: {
				int func3 = func3_extract(instruction);
				int func7 = func7_extract(instruction);
				int rd = rd_extract(instruction);
				if (rd == 0) {
					break;
				}
				int rs1 = rs1_extract(instruction);
				int rs2 = rs2_extract(instruction);
				// Add
				if (func3 == 0 && func7 == 0) {
					reg[rd] = reg[rs1] + reg[rs2];
				// SUB
				} else if (func3 == 0 && func7 == 0x20) {
					reg[rd] = reg[rs1] - reg[rs2];
				// XOR
				} else if (func3 == 0b100 && func7 == 0) {
					reg[rd] = reg[rs1] ^ reg[rs2];
				// OR
				} else if (func3 == 0b110 && func7 == 0) {
					reg[rd] = reg[rs1] | reg[rs2];
				// AND
				} else if (func3 == 0b111 && func7 == 0) {
					reg[rd] = reg[rs1] & reg[rs2];
				// SHIFT LEFT
				} else if (func3 == 0b001 && func7 == 0) {
					reg[rd] = ((unsigned int) reg[rs1] << (unsigned int) reg[rs2]);
				// SHIFT RIGHT
				} else if (func3 == 0b101 && func7 == 0) {
					reg[rd] = ((unsigned int) reg[rs1] >> (unsigned int) reg[rs2]);
				// Rotate Right;
				// SRA
				} else if (func3 == 0b101 && func7 == 0x20) {
					int temp = reg[rs1];
					for (int i = 0; i < reg[rs2]; i++) {
						int num = (temp & 0b1) << 31;
						temp = (temp >> 1) | num;
					}
					reg[rd] = temp;
				// SLT
				} else if (func3 == 0b010 && func7 == 0) {
					reg[rd] = (reg[rs1] < reg[rs2] ? 1 : 0);
				// SLTU
				} else if (func3 == 0b011 && func7 == 0) {
					reg[rd] = ((unsigned int) reg[rs1] <  (unsigned int) reg[rs2] ? 1 : 0);
				}
				break;
			}

			// Save memory
			// Type S
			case 0b0100011: {
				int func3 = func3_extract(instruction);
				int rs1 = rs1_extract(instruction);
				int rs2 = rs2_extract(instruction);
				int imm = sign_extending(s_imm(instruction), 12);
				// 32 bit value
				// sw
				printf("heap_value: %x\n", reg[rs1] + imm);
				if (func3 == 0b010) {
					if (reg[rs1] + imm >= 0xb700) {
						heap_save(reg[rs1] + imm, reg, reg[rs2], heap_banks, 32, pc_val, instruction);
					} else if (reg[rs1] + imm >= 0x800) {
						virtual_routines(instruction, reg[rs1] + imm, reg[rs2], pc_val, reg, memory, heap_banks);
					} else {
						memory[reg[rs1] + imm] = (reg[rs2] >> 24) & 0xFF;
						memory[reg[rs1] + imm + 1] = (reg[rs2] >> 16) & 0xFF;
						memory[reg[rs1] + imm + 2] = (reg[rs2] >> 8) & 0xFF;
						memory[reg[rs1] + imm + 3] = reg[rs2] & 0xFF;
					}
				// 16 bit value
				// sh
				} else if (func3 == 0b001) {
					if (reg[rs1] + imm >= 0xb700) {
						heap_save(reg[rs1] + imm, reg, reg[rs2], heap_banks, 16, pc_val, instruction);
					} else if (reg[rs1] + imm >= 0x800) {
						virtual_routines(instruction, reg[rs1] + imm, reg[rs2], pc_val, reg, memory, heap_banks);
					} else {
						memory[reg[rs1] + imm] = (reg[rs2] >> 8) & 0xFF;
						memory[reg[rs1] + imm + 1] = reg[rs2] & 0xFF;
					}
				// 8 bit value
				// sb
				} else if (func3 == 0) {
					if (reg[rs1] + imm >= 0xb700) {
						heap_save(reg[rs1] + imm, reg, reg[rs2], heap_banks, 8, pc_val, instruction);
					} else if (reg[rs1] + imm >= 0x800) {
						virtual_routines(instruction, reg[rs1] + imm, reg[rs2], pc_val, reg, memory, heap_banks);
					} else {
						memory[reg[rs1] + imm] = reg[rs2] & 0xFF;
					}
				}
				break;
			}
		
			case 0b1100011: {
				int func3 = func3_extract(instruction);
				int imm = sign_extending(sb_imm(instruction) << 1, 12);
				int rs2 = rs2_extract(instruction);
				int rs1 = rs1_extract(instruction);
				// beq
				if (func3 == 0) {
					if (reg[rs1] == reg[rs2]) {
						pc_val += imm - 4;
					}
				// bne
				} else if (func3 == 0b001) {
					if (reg[rs1] != reg[rs2]) {
						pc_val += imm - 4;
					}
				// blt
				} else if (func3 == 0b100) {
					if (reg[rs1] < reg[rs2]) {
						pc_val += imm - 4;
					}
				// bltu
				} else if (func3 == 0b110) {
					if ((unsigned int) reg[rs1] < (unsigned int) reg[rs2]) {
						pc_val += imm - 4;
					}
				// bge
				} else if (func3 == 0b101) {
					if (reg[rs1] >= reg[rs2]) {
						pc_val += imm - 4;
					}
				// bgeu
				} else if (func3 == 0b111) {
					if ((unsigned int) reg[rs1] >= (unsigned int) reg[rs2]) {
						pc_val += imm - 4;
					}
				}
				break;
			}

			// jalr
			case 0b1100111: {
				int func3 = func3_extract(instruction);
				int rd = rd_extract(instruction);
				int rs1 = rs1_extract(instruction);
				int imm = sign_extending(instruction >> 20, 12);
				if (func3 == 0) {
					if (rd != 0) {
						reg[rd] = pc_val + 4;
					}
					pc_val = reg[rs1] + imm - 4;
				}
				break;
			}
			default: {
				printf("Instruction Not Implemented: ");
				printf("0x%x\n", instruction);
				register_dump(pc_val, reg);
				exit(1);
			}
		}
		pc_val += 4;
	}
	return 1;
}