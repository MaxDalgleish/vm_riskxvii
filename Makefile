TARGET = vm_riskxvii

CC = gcc

CFLAGS     = -c -Wall -Wvla -Werror -Oz -flto -g0 -std=c11
SRC        = vm_riskxvii.c
OBJ        = $(SRC:.c=.o)

all:$(TARGET)

$(TARGET):$(OBJ)
	$(CC) -o $@ $(OBJ)

.SUFFIXES: .c .o

.c.o:
	 $(CC) $(CFLAGS) $(ASAN_FLAGS) $<

run:
	./$(TARGET)

test:
	gcc --coverage $(TARGET).c -o $(TARGET)

run_tests:
	cat examples/5_sum/5_sum.in | ./$(TARGET) examples/5_sum/5_sum.mi | diff - examples/5_sum/5_sum.out
	cat examples/add_2_numbers/add_2_numbers.in | ./$(TARGET) examples/add_2_numbers/add_2_numbers.mi | diff - examples/add_2_numbers/add_2_numbers.out
	cat examples/add_2_numbers_withfunc/add_2_numbers_withfunc.in | ./$(TARGET) examples/add_2_numbers_withfunc/add_2_numbers_withfunc.mi | diff - examples/add_2_numbers_withfunc/add_2_numbers_withfunc.out
	./$(TARGET) examples/printing_h/printing_h.mi | diff - examples/printing_h/printing_h.out
	./$(TARGET) examples/invalid_instruction/invalid_instruction.mi | diff - examples/invalid_instruction/invalid_instruction.out
	cat examples/subtract/subtract.in | ./$(TARGET) examples/subtract/subtract.mi | diff - examples/subtract/subtract.out
	cat examples/xor/xor.in | ./$(TARGET) examples/xor/xor.mi | diff - examples/xor/xor.out
	cat examples/or/or.in | ./$(TARGET) examples/or/or.mi | diff - examples/or/or.out
	cat examples/and/and.in | ./$(TARGET) examples/and/and.mi | diff - examples/and/and.out
	cat examples/sll/sll.in | ./$(TARGET) examples/sll/sll.mi | diff - examples/sll/sll.out
	cat examples/srl/srl.in | ./$(TARGET) examples/srl/srl.mi | diff - examples/srl/srl.out
	cat examples/sra/sra.in | ./$(TARGET) examples/sra/sra.mi | diff - examples/sra/sra.out
	cat examples/set_if_smaller/set_if_smaller.in | ./$(TARGET) examples/set_if_smaller/set_if_smaller.mi | diff - examples/set_if_smaller/set_if_smaller.out
clean:
	rm -f *.o *.obj $(TARGET)
