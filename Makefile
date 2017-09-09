.PHONY: all
all: kvmsample test.bin

.PHONY: clean
clean:
	@rm -f kvmsample;
	@rm -f test.bin;
	@rm -f test.o;
	@rm -f main.o;

kvmsample: main.o
	$(CROSS_COMPILE)gcc main.c -o kvmsample -lpthread

test.bin: test.o
	$(CROSS_COMPILE)ld -m elf_i386 --oformat binary -N -e _start -Ttext 0x10000 -o test.bin test.o

test.o: test.S
	$(CROSS_COMPILE)as -32 test.S -o test.o
	
