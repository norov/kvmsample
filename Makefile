CROSS_COMPILE?=/home/yury/work/gcc-linaro-6.3.1-2017.02-i686_aarch64-linux-gnu/bin/aarch64-linux-gnu-
ARCH?=arm64

#CROSS_COMPILE=
#ARCH=i386

CFLAGS := -D _ARCH=$(ARCH) -I./ -I./$(ARCH)/include/ -lpthread

.PHONY: all
all: kvmsample test.bin

.PHONY: clean
clean:
	@rm -f kvmsample;
	@rm -f test.bin;
	@rm -f test.o;
	@rm -f main.o;

kvmsample: main.o $(ARCH)/kvm_init.o
	$(CROSS_COMPILE)gcc main.c $(CFLAGS) $(ARCH)/kvm_init.c -o kvmsample

include $(ARCH)/Makefile.include
