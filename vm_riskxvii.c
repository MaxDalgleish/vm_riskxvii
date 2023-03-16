#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    FILE *bin_input = fopen(argv[1], "rb");
    for (int i = 0; i < 4; i++) {
        int a = fgetc(bin_input);
        int b = fgetc(bin_input);
        int c = fgetc(bin_input);
        int d = fgetc(bin_input);

        int result = ((((((d << 8) | c) << 8) | b) << 8) | a);
        printf("%x ", result);


        int instruction = result & 0x7F
    }
    
    return 0;
}