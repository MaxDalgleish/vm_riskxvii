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



int main(int argc, char **argv) {
    int registers[32] = {0};
    int memory[2048] = {0};
    int pc_val = 0;
    FILE *bin_input = fopen(argv[1], "rb");
    for (int i = 0; i < 2048; i++) {

        int c = fgetc(bin_input);
        printf("%x ", c);
        memory[pc_val] = c;
        pc_val += 1;
    }

    pc_val = 0;
    int i = 0; 
    while (i < 100) {
        printf("%d\n", pc_val);
        int instruction = big_endian(memory, pc_val);
        printf("%x ", instruction);
        int opcode = instruction & 0x7F;
        switch (opcode)
        {
            // addi
            case 0b0010011: {
                int func3 = (instruction >> 12) & 0b111;
                if (func3 == 0b0) {
                    int rd = (instruction >> 7) & 0x1F;
                    int rs1 = (instruction >> 15) & 0x1F;
                    int imm = (instruction >> 20) & 0xFFF;
                    // printf("%d %d %d\n", rd, rs1, imm);
                    registers[rd] = registers[rs1] + imm;
                }
                pc_val += 4;
                break;
            }
            // jal
            case 0b1101111: {
                int rd = (instruction >> 7) & 0b11111;
                registers[rd] = pc_val + 4;
                int imm1 = (instruction >> 21) & 0x3FF;
                int imm2 = (instruction >> 20) & 0b1;
                int imm3 = (instruction >> 12) & 0xFF;
                int imm4 = (instruction >> 31) & 0b1;
                int imm = ((((((imm4 << 8) | imm3) << 1) | imm2) << 10) | imm1);
                pc_val = pc_val + (imm << 1);
                fseek(bin_input, pc_val, SEEK_SET);
                break;
            }
            // lui
            case 0b0110111: {
                int rd = (instruction >> 7) & 0x1F;
                int imm = ((instruction >> 12) & 0xFFFFF) << 12;
                registers[rd] = imm;
                break;
            }
            
            case 0b0100011: {
                int func3 = (instruction >> 12) & 0b111;
                // Sb
                if (func3 == 0b0) {
                    
                }
            }
            default:
                break;
        }
        i++;
    }
    return 0;
}