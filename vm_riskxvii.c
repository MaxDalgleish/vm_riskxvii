#include <stdio.h>
#include <string.h>
#include <stdlib.h>







int main(int argc, char **argv) {
    int registers[32] = {0};
    int pc_val = 0;
    FILE *bin_input = fopen(argv[1], "rb");
    for (int i = 0; i < 4; i++) {
        int a = fgetc(bin_input);
        int b = fgetc(bin_input);
        int c = fgetc(bin_input);
        int d = fgetc(bin_input);
        printf("%x ", a);
        printf("%x ", b);
        printf("%x ", c);
        printf("%x ", d);

        int instruction = ((((((d << 8) | c) << 8) | b) << 8) | a);
        printf("%x ", instruction);


        int opcode = instruction & 0x7F;
        switch (opcode)
        {
            case 0b0010011: {
                int func3 = instruction & (0b111 << 12);
                if (func3 == 0b000) {
                    int rd = (instruction >> 7) & 0b11111;
                    int rs1 = (instruction >> 15) & 0b11111;
                    int imm = (instruction >> 20) & 0xFFF;
                    registers[rd] = registers[rs1] + imm;
                    printf("%d\n", registers[rd]);
                }
                break;
            }
            case 0b1101111: {
                int rd = (instruction >> 7) & 0b11111;
                registers[rd] = pc_val + 4;
                int imm1 = (instruction >> 21) & 0x3FF;
                int imm2 = (instruction >> 20) & 0b1;
                int imm3 = (instruction >> 12) & 0xFF;
                int imm4 = (instruction >> 31) & 0b1;
                int imm = ((((((imm4 << 8) | imm3) << 1) | imm2) << 10) | imm1);
                printf("%d\n ", imm);

                break;
            }
            default:
                break;
        }
        pc_val += 4;
    }
    return 0;
}